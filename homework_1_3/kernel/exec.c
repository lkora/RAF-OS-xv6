#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"

int
exec(char *path, char **argv)
{
	char *s, *last;
	int i, off;
	uint argc, sz, sp, ustack[3+MAXARG+1];
	struct elfhdr elf;
	struct inode *ip;
	struct proghdr ph;
	pde_t *pgdir, *oldpgdir;
	struct proc *curproc = myproc();

	begin_op();

	if((ip = namei(path)) == 0){
		end_op();
		cprintf("exec: fail\n");
		return -1;
	}
	ilock(ip);
	pgdir = 0;

	// Check ELF header
	if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
		goto bad;
	if(elf.magic != ELF_MAGIC)
		goto bad;

	if((pgdir = setupkvm()) == 0)
		goto bad;

	// Load program into memory.
	sz = 0;
	for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
		if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
			goto bad;
		if(ph.type != ELF_PROG_LOAD)
			continue;
		if(ph.memsz < ph.filesz)
			goto bad;
		if(ph.vaddr + ph.memsz < ph.vaddr)
			goto bad;
		if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
			goto bad;
		if(ph.vaddr % PGSIZE != 0)
			goto bad;
		if(loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
			goto bad;
	}
	iunlockput(ip);
	end_op();
	ip = 0;

	// Allocate two pages at the next page boundary.
	// Make the first inaccessible.  Use the second as the user stack.
	sz = PGROUNDUP(sz);
	if((sz = allocuvm(pgdir, sz, sz + 2*PGSIZE)) == 0)
		goto bad;
	clearpteu(pgdir, (char*)(sz - 2*PGSIZE));
	sp = sz;

	// Push argument strings, prepare rest of stack in ustack.
	for(argc = 0; argv[argc]; argc++) {
		if(argc >= MAXARG)
			goto bad;
		sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
		if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
			goto bad;
		ustack[3+argc] = sp;
	}
	ustack[3+argc] = 0;

	ustack[0] = 0xffffffff;  // fake return PC
	ustack[1] = argc;
	ustack[2] = sp - (argc+1)*4;  // argv pointer

	sp -= (3+argc+1) * 4;
	if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
		goto bad;

	// Save program name for debugging.
	for(last=s=path; *s; s++)
		if(*s == '/')
			last = s+1;
	safestrcpy(curproc->name, last, sizeof(curproc->name));

	// Map the shared objects
    // pde_t *parent_pgdir = curproc->pgdir;
    // for (int i = 0; i < MAX_SHARED_STRUCTS; i++) {
    //     if (curproc->shared_structs[i].size > 0) {
    //         char *mem = kalloc();
    //         memmove(mem, curproc->shared_structs[i].addr, PGSIZE);
    //         smappages(parent_pgdir, (char *)(1 << 30) + i * PGSIZE, PGSIZE, V2P(mem), PTE_W | PTE_U);
    //         curproc->shared_structs[i].addr = (void *)((1 << 30) + i * PGSIZE);
    //     }
    // }
	for(i = 0; i < MAX_SHARED_STRUCTS; i++){
		int size;
		if((size=curproc->shared_structs[i].size) != 0){
			pte_t* pte;
			// Only valid shared memory regions are to be mapped
 			if((pte = swalkpgdir(curproc->parent_pgdir, curproc->shared_structs[i].addr, 0)) == 0)
				panic("copyuvm: pte should exist");
     		
			// Retreive the corresponding page table entry for the shared memory block
			if(!(*pte & PTE_P))
				panic("copyuvm: page not present");
  			uint pa = PTE_ADDR(*pte);

   			uint flags = PTE_FLAGS(*pte);
			uint start = i * PGSIZE;
			// Calculate the valid starting memory
			if (size > PGSIZE)
				start = 12 * i * curproc->pid * PGSIZE;

 			int* ptr = (int*)(SHAREDMEM + start);
      		if(smappages(pgdir,(void*)ptr, size, pa, flags) < 0)
				panic("smtn");
		}
	}

	// Commit to the user image.
	oldpgdir = curproc->pgdir;
	curproc->pgdir = pgdir;
	curproc->sz = sz;
	curproc->tf->eip = elf.entry;  // main
	curproc->tf->esp = sp;
	switchuvm(curproc);
	freevm(oldpgdir);
	return 0;

	bad:
	if(pgdir)
		freevm(pgdir);
	if(ip){
		iunlockput(ip);
		end_op();
	}
	return -1;
}