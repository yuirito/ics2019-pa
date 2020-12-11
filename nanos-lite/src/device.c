#include "common.h"
#include <amdev.h>

#ifndef KEYDOWN_MASK
#define KEYDOWN_MASK 0x8000
#endif

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for(int i=0;i<len;i++){
    _putc(((char *)buf)[i]);
  }
  return len;
}

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t offset, size_t len) {
  int key = read_key();
  bool down =false;
  if(key & KEYDOWN_MASK){
    key ^=KEYDOWN_MASK;
    down = true;
  }
  if(key != _KEY_NONE){
    sprintf(buf,"%s %s\n",down ? "kd":"ku",keyname[key]);

  }
  else {
    sprintf(buf, "t %u\n", uptime());
  }
  return strlen(buf);
}

static char dispinfo[128] __attribute__((used)) = {};

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  strncpy(buf,dispinfo + offset, len);
  return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  int x,y;
  offset = offset>>2;
  y = offset / screen_width();
  x = offset % screen_width();
  //printf("%d %d\n",x,y);
  int lenth = len>>2;
  int len1,len2=0,len3=0;

  len1 = lenth<= (screen_width()-x) ? lenth : screen_width()-x;
  draw_rect((uint32_t*)buf,x,y,len1,1);

  if((lenth>len1)&&((lenth-len1)>screen_width())){
    len2 = lenth-len1;
    draw_rect((uint32_t*)buf+len1,0,y+1,screen_width(),len2/screen_width());
  }

  if(lenth-len1-len2>0){
    len3 = lenth-len1-len2;
    draw_rect((uint32_t*)buf+len1+len2,0,y+len2/screen_width()+1,len3,1);
  }
	//draw_rect((uint32_t *)buf,x,y,lenth,1);
  return len;
}

size_t fbsync_write(const void *buf, size_t offset, size_t len) {
  draw_sync();
  return len;
}

void init_device() {
  Log("Initializing devices...");
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", screen_width(), screen_height());
}
