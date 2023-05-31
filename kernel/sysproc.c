#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
	return fork();
}

int
sys_exit(void)
{
	exit();
	return 0;  // not reached
}

int
sys_wait(void)
{
	return wait();
}

int
sys_kill(void)
{
	int pid;

	if(argint(0, &pid) < 0)
		return -1;
	return kill(pid);
}

int
sys_getpid(void)
{
	return myproc()->pid;
}

int
sys_sbrk(void)
{
	int addr;
	int n;

	if(argint(0, &n) < 0)
		return -1;
	addr = myproc()->sz;
	if(growproc(n) < 0)
		return -1;
	return addr;
}

int
sys_sleep(void)
{
	int n;
	uint ticks0;

	if(argint(0, &n) < 0)
		return -1;
	acquire(&tickslock);
	ticks0 = ticks;
	while(ticks - ticks0 < n){
		if(myproc()->killed){
			release(&tickslock);
			return -1;
		}
		sleep(&ticks, &tickslock);
	}
	release(&tickslock);
	return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
	uint xticks;

	acquire(&tickslock);
	xticks = ticks;
	release(&tickslock);
	return xticks;
}

int sys_share_data(void) {
    char *name;
    void *addr;
    int size;

    // Retrieve the parameters from the user stack
    if (argstr(0, &name) < 0 || argptr(1, (char **)&addr, sizeof(void *)) < 0 || argint(2, &size) < 0)
        return -1;

    // Check if the name is longer than 10 characters
    if (strlen(name) > MAX_SHARED_STRUCT_NAME)
        return -1;

    // Check if there is already a shared structure with the same name
    struct proc *curproc = myproc();
    for (int i = 0; i < MAX_SHARED_STRUCTS; i++) {
        if (curproc->shared_structs[i].size > 0 && strncmp(curproc->shared_structs[i].name, name, MAX_SHARED_STRUCT_NAME) == 0)
            return -2;
    }

    // Find an unused shared structure
    int index = -1;
    for (int i = 0; i < MAX_SHARED_STRUCTS; i++) {
        if (curproc->shared_structs[i].size == 0) {
            index = i;
            break;
        }
    }

    // Check if there are already 10 shared structures in the current process
    if (index == -1)
        return -3;

    // Register the new shared structure
    strncpy(curproc->shared_structs[index].name, name, MAX_SHARED_STRUCT_NAME);
    curproc->shared_structs[index].addr = addr;
    curproc->shared_structs[index].size = size;
    cprintf("Saved at address: %p - name %s - size %d\n", addr, name, size);

    return index;
}

int sys_get_data(void) {
    char *name;
    void **addr;

    // Retrieve the parameters from the user stack
    if (argstr(0, &name) < 0 || argptr(1, (char **)&addr, sizeof(void *)) < 0)
        return -1;

    // Check if there is a shared structure with the specified name
    struct proc *curproc = myproc();
    int index = -1;
    for (int i = 0; i < MAX_SHARED_STRUCTS; i++) {
        if (curproc->shared_structs[i].size > 0 && strncmp(curproc->shared_structs[i].name, name, MAX_SHARED_STRUCT_NAME) == 0) {
            index = i;
            cprintf("Found addres for name: %s at index: %d\n", name, index);
            break;
        }
    }

    // Check if there is no shared structure with the specified name
    if (index == -1)
        return -2;

    // Update the addr pointer
    *addr = (void*)(curproc->shared_structs[index].addr);
    cprintf("Shared index: %d - at address: %p\n", index, addr);

    return 0;

}
