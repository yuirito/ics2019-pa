#include "common.h"
#include "syscall.h"
#include "proc.h"
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
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  c->eax=result;
  return NULL;
}

int sys_yield(){
  _yield();
  return 0;
}

void sys_exit(uintptr_t arg){
  _halt(arg);
}