#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  /* PA1.3*/
  char expr[128];
  uint32_t value;
  int hit;

} WP;

#endif
