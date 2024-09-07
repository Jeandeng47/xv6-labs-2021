#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

// functions for sysinfo
uint64 freememsize(void); // Function to calculate the free memory size
int procnum(void);        // Function to count the number of active processes

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
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

uint64
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

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_trace(void) 
{
  int mask;
  // retrieve the bit mask arg passed from user space
  // trace(MASK): get 0th arg as int and store it in mask
  if (argint(0, &mask) < 0) {
    // if retrieval fails
    return -1;
  }
  myproc()->trace_mask = mask;
  return 0; // return success
}

uint64 
sys_sysinfo(void)
{
  // retrieve the address of sysinfo struct from user space
  uint64 p;
  if (argaddr(0, &p) < 0)
  {
    return -1;
  }
  struct sysinfo sinfo;
  sinfo.freemem = freememsize();
  sinfo.nproc = procnum();
  // copy out the sysinfo struct from kernel space to user space
  if (copyout(myproc()->pagetable, p, (char *)&sinfo, sizeof(sinfo)))
  {
    return -1;
  }
  return 0;
}
