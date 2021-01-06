#include "common.h"
#include "syscall.h"
#include "proc.h"

int sys_yield();
void sys_exit(uintptr_t arg);
size_t sys_write(int fd,const void *buf,size_t count);
int sys_brk(uintptr_t increment);
int sys_execve(const char *fname, char * const argv[], char *const envp[]);

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  int result = -1;
  switch (a[0]) {
    case SYS_exit: sys_exit(a[1]);break;
    case SYS_yield: result=sys_yield();break;
    case SYS_open: result=fs_open((char*)a[1],a[2],a[3]);break;
    case SYS_read: result = fs_read(a[1],(void*)a[2],a[3]);break;
    case SYS_write: result = fs_write(a[1],(void*)a[2],a[3]);break;
    case SYS_close: result = fs_close(a[1]);break;
    case SYS_lseek: result = fs_lseek(a[1],a[2],a[3]);break;
    case SYS_brk: result=sys_brk(a[1]);break;
    case SYS_execve: result = sys_execve(a[1], a[2], a[3]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  c->GPRx=result;
  return NULL;
}

int sys_yield(){
  _yield();
  return 0;
}

void sys_exit(uintptr_t arg){
  //_halt(arg);
  sys_execve("/bin/init", NULL, NULL);
}

size_t sys_write(int fd,const void *buf,size_t count){
  if(fd==1||fd==2){
    for(int i=0;i<count;i++){
      _putc(((char *)buf)[i]);
    }
  }
  return count;
}

int sys_brk(uintptr_t increment){
  return 0;
}

int sys_execve(const char *fname, char * const argv[], char *const envp[]) {
  naive_uload(NULL, fname);
}