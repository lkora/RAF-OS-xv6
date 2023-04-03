// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "kbd.h"

// Echo function toggle
int cons_echo = 1;
int encr_key = 0;


static void consputc(int);

static int panicked = 0;

static struct {
	struct spinlock lock;
	int locking;
} cons;

static void
printint(int xx, int base, int sign)
{
	static char digits[] = "0123456789abcdef";
	char buf[16];
	int i;
	uint x;

	if(sign && (sign = xx < 0))
		x = -xx;
	else
		x = xx;

	i = 0;
	do{
		buf[i++] = digits[x % base];
	}while((x /= base) != 0);

	if(sign)
		buf[i++] = '-';

	while(--i >= 0)
		consputc(buf[i]);
}

// Print to the console. only understands %d, %x, %p, %s.
void
cprintf(char *fmt, ...)
{
	int i, c, locking;
	uint *argp;
	char *s;

	locking = cons.locking;
	if(locking)
		acquire(&cons.lock);

	if (fmt == 0)
		panic("null fmt");

	argp = (uint*)(void*)(&fmt + 1);
	for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
		if(c != '%'){
			consputc(c);
			continue;
		}
		c = fmt[++i] & 0xff;
		if(c == 0)
			break;
		switch(c){
		case 'd':
			printint(*argp++, 10, 1);
			break;
		case 'x':
		case 'p':
			printint(*argp++, 16, 0);
			break;
		case 's':
			if((s = (char*)*argp++) == 0)
				s = "(null)";
			for(; *s; s++)
				consputc(*s);
			break;
		case '%':
			consputc('%');
			break;
		default:
			// Print unknown % sequence to draw attention.
			consputc('%');
			consputc(c);
			break;
		}
	}

	if(locking)
		release(&cons.lock);
}

void
panic(char *s)
{
	int i;
	uint pcs[10];

	cli();
	cons.locking = 0;
	// use lapiccpunum so that we can call panic from mycpu()
	cprintf("lapicid %d: panic: ", lapicid());
	cprintf(s);
	cprintf("\n");
	getcallerpcs(&s, pcs);
	for(i=0; i<10; i++)
		cprintf(" %p", pcs[i]);
	panicked = 1; // freeze other CPU
	for(;;)
		;
}

#define BACKSPACE 0x100
#define CRTPORT 0x3d4
static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory

// Allocate a new buffer of the same size as the original buffer


/*
A color mask is created
- Single space in memory is 2 bytes = 16 Bits
		Higher byte is used for setting the foreground and background color
		Lower byte is an ASCII character
The color is represented as as 4 bits
	First 4 bits represent foreground color
    Last 4 bits represent background color
The mask ((x << 4) | y) << 8
x, y are background and foreground values respectively
1. X's value is taken and shifted by 4 spaces to the left this will represent our background color
   	Shifting to the left populates the last 4 spaces with logical 0 values
2. By doing bitwise | (or) with Y's value we populate the last 4 bits in memory which coorespond to our
   	foreground color
3. Lastly the created byte which represents the color is shifted by 8 to make space for a single byte
   	to be populated with the respective ASCII character representation
*/
static int color = ((0 << 4) | 15) << 8;
/*
Color table: 
0 - black			8 - bright gray
1 - dark blue		9 - bright blue
2 - dark green		10 - bright green
3 - dark cyan		11 - bright cyan
4 - dark red		12 - bright red
5 - dark purple		13 - bright purple
6 - dark yellow		14 - bright yellow
7 - dark gray		15 - white
*/

int selected_row = 0;
const char *selectable_color_names[] = {"WHT BLK", "PUR WHT", "RED AQU", "WHT YEL"};
const int selectable_colors[] = {
					((0 << 4) | 15) << 8, 
					((15 << 4) | 13) << 8,
					((3 << 4) | 12) << 8,
					((6 << 4) | 15) << 8
};

int is_showing_table = 0;
void show_table() {
	const int console_width = 80;
	const int console_height = 25;
	const int base_color = ((7 << 4) | 0) << 8;
	const int selected_color = ((2 << 4) | 0) << 8;


	int rows = 4;
	int cols = 9;
	int pos;

	// Cursor position: col + 80*row.
	outb(CRTPORT, 14);
	pos = inb(CRTPORT+1) << 8;
	outb(CRTPORT, 15);
	pos |= inb(CRTPORT+1);

	int i, j;
	int height = rows * 2 + 1;
	

	// Get the starting position
	pos = pos - height * console_width;
	// Adjust the starting position to the left if the table width exceeds console width
	int pos_in_row = pos % console_width;
	if(pos_in_row + cols > console_width) {
		// Bind the table to the right edge
		// pos -= (pos_in_row + cols) % console_width;
		pos -= cols;
	}


	for(i = 0; i < height; i++) {
		for(j = 0; j < cols; j++) {
			// Corners
			if((i == 0 || i == height - 1) && (j == 0 || j == cols - 1)){
				crt[pos+j] = '+' | base_color;
			} 
			// Vertical sides
			else if (j == 0 || j == cols - 1) {
				crt[pos+j] = '|' | base_color;
			}
			// Horizontal sides
			else if (i == 0 || i == height - 1 || i % 2 == 0) {
				crt[pos+j] = '-' | base_color;
			}
			// Content
			else {
				crt[pos+j] = selectable_color_names[i/2][j-1] | (i == selected_row ? base_color : (i/2 == selected_row ? selected_color : base_color));
			}
			
		}
		// Go to next row starting from the same column
		pos += 80;
	}

}

ushort *crt_copy;
void copy_crt() {
    // Allocate memory for the copy
    crt_copy = (ushort*)kalloc();
    if (!crt_copy) {
        cprintf("Error: Failed to allocate memory for CRT copy\n");
        return;
    }

    // Copy the CRT memory to the new location
    memcpy(crt_copy, crt, 2 * 80 * 25);
}

void restore_crt() {
    // Copy the contents of the copy back to CRT memory
    memcpy(crt, crt_copy, 2 * 80 * 25);

    // Free the memory used by the copy
    kfree((char*)crt_copy);
    crt_copy = 0;
}


static void
cgaputc(int c)
{
	int pos;

	// Cursor position: col + 80*row.
	outb(CRTPORT, 14);
	pos = inb(CRTPORT+1) << 8;
	outb(CRTPORT, 15);
	pos |= inb(CRTPORT+1);

	if(c == '\n')
		pos += 80 - pos%80;
	else if(c == BACKSPACE){
		if(pos > 0) --pos;
	} else
		crt[pos++] = (c&0xff) | color;  // Or it with the custom color mask

	if(pos < 0 || pos > 25*80)
		panic("pos under/overflow");

	if((pos/80) >= 24){  // Scroll up.
		memmove(crt, crt+80, sizeof(crt[0])*23*80);
		pos -= 80;
		memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
	}

	outb(CRTPORT, 14);
	outb(CRTPORT+1, pos>>8);
	outb(CRTPORT, 15);
	outb(CRTPORT+1, pos);
	crt[pos] = ' ' | 0x0700;
}

void
consputc(int c)
{
	if(panicked){
		cli();
		for(;;)
			;
	}

	if(c == BACKSPACE){
		uartputc('\b'); uartputc(' '); uartputc('\b');
	} else
		uartputc(c);
	cgaputc(c);
}

#define INPUT_BUF 128
struct {
	char buf[INPUT_BUF];
	uint r;  // Read index
	uint w;  // Write index
	uint e;  // Edit index
} input;

void
consoleintr(int (*getc)(void))
{
	int c, doprocdump = 0;

	acquire(&cons.lock);
	while((c = getc()) >= 0){
		switch(c){
		case C('P'):  // Process listing.
			// procdump() locks cons.lock indirectly; invoke later
			doprocdump = 1;
			break;
		case C('U'):  // Kill line.
			while(input.e != input.w &&
			      input.buf[(input.e-1) % INPUT_BUF] != '\n'){
				input.e--;
				consputc(BACKSPACE);
			}
			break;
		case C('H'): case '\x7f':  // Backspace
			if(input.e != input.w){
				input.e--;
				consputc(BACKSPACE);
			}
			break;
		case A('C'): 
			is_showing_table = !is_showing_table;
			if (is_showing_table) {
				// Save previous buffer and show selection
				copy_crt();
				show_table();
			} else {
				// Restore buffer
				restore_crt();
				color = selectable_colors[selected_row];
			}

			break;
		default:
			// Move up/down and redraw table
			if(is_showing_table) {
				if (c == 'w') {
					selected_row -= 1;
					selected_row = selected_row < 0 ? 0 : selected_row;
				} else if (c == 's') {
					selected_row += 1;
					selected_row = selected_row >= 4 ? selected_row - 1 : selected_row;
				}
				show_table();
			} else if(c != 0 && input.e-input.r < INPUT_BUF){
				c = (c == '\r') ? '\n' : c;
				input.buf[input.e++ % INPUT_BUF] = c;
				
				// Hide characters that are to be hidden
				if (cons_echo == 0 && c != '\n'){
					consputc('*');
				} else {
					consputc(c);
				}				
				
				if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
					input.w = input.e;
					wakeup(&input.r);
				}
			}
			
			break;
		}
	}
	release(&cons.lock);
	if(doprocdump) {
		procdump();  // now call procdump() wo. cons.lock held
	}
}

int
consoleread(struct inode *ip, char *dst, int n)
{
	uint target;
	int c;

	iunlock(ip);
	target = n;
	acquire(&cons.lock);
	while(n > 0){
		while(input.r == input.w){
			if(myproc()->killed){
				release(&cons.lock);
				ilock(ip);
				return -1;
			}
			sleep(&input.r, &cons.lock);
		}
		c = input.buf[input.r++ % INPUT_BUF];
		if(c == C('D')){  // EOF
			if(n < target){
				// Save ^D for next time, to make sure
				// caller gets a 0-byte result.
				input.r--;
			}
			break;
		}
		*dst++ = c;
		--n;
		if(c == '\n')
			break;
	}
	release(&cons.lock);
	ilock(ip);

	return target - n;
}

int
consolewrite(struct inode *ip, char *buf, int n)
{
	int i;

	iunlock(ip);
	acquire(&cons.lock);
	for(i = 0; i < n; i++)
		consputc(buf[i] & 0xff);
	release(&cons.lock);
	ilock(ip);

	return n;
}

void
consoleinit(void)
{
	initlock(&cons.lock, "console");

	devsw[CONSOLE].write = consolewrite;
	devsw[CONSOLE].read = consoleread;
	cons.locking = 1;

	ioapicenable(IRQ_KBD, 0);
}

