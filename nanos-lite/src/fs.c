#include "fs.h"

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t fbsync_write(const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin", 0, 0, 0, invalid_read, invalid_write},
  {"stdout", 0, 0, 0, invalid_read, serial_write},
  {"stderr", 0, 0, 0, invalid_read, serial_write},
  {"/dev/fb", 0, 0, 0, invalid_read, fb_write},
  {"/dev/events", 0, 0, 0, events_read, invalid_write},
  {"/dev/fbsync", 0, 0, 0, invalid_read,fbsync_write},
  {"/proc/dispinfo", 128, 0, 0, dispinfo_read,invalid_write},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_table[FD_FB].size = (screen_width()*screen_height())<<2;
}

int fs_open(const char *pathname, int flags, int mode){
  for(int i=0;i<NR_FILES;i++){
    if(strcmp(file_table[i].name,pathname)==0){
      file_table[i].open_offset=0;
      return i;
    }
  }
  printf("file %s does not exist!",pathname);
  assert(0);
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len){
	assert(fd >= 0 && fd < NR_FILES);
	
	int r_len = len;
	if(file_table[fd].size > 0 && file_table[fd].open_offset + len > file_table[fd].size) {
		r_len = file_table[fd].size - file_table[fd].open_offset;
	}
	assert(r_len >= 0);

	size_t length = 0;
	if(file_table[fd].read == NULL) {
		length = ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, r_len);
	}else{
		length = file_table[fd].read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, r_len);
	}
  file_table[fd].open_offset += length;
	return length;
}

int fs_close(int fd){
  return 0;
}

size_t fs_write(int fd, const void *buf, size_t len){
	assert(fd >= 0 && fd < NR_FILES);

	int w_len = len;
	if(file_table[fd].size > 0&& file_table[fd].open_offset + len > file_table[fd].size) {
		w_len = file_table[fd].size - file_table[fd].open_offset;
	}
	
	assert(w_len >= 0);
	
	size_t length = 0;
  if(file_table[fd].write == NULL) {
		length = ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, w_len);
	}else{
		length = file_table[fd].write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, w_len);
	}
  
  file_table[fd].open_offset += length;
	return length;
}

size_t fs_lseek(int fd, size_t offset, int whence){
	assert(fd >= 0 && fd < NR_FILES);
	size_t open_offset = file_table[fd].open_offset;
	switch (whence) {
		case SEEK_SET: 
			open_offset = offset;
			break;
		case SEEK_CUR:
			open_offset += offset;
			break;
		case SEEK_END:
			open_offset = file_table[fd].size + offset;
			break;
		default: panic("There is no such whence");
	}
	file_table[fd].open_offset = open_offset;
	return open_offset;
}

size_t fs_open_offset(int fd){
  return file_table[fd].open_offset;
}
