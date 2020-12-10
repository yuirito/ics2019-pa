#include "proc.h"
#include <elf.h>
#include "fs.h"

#ifdef __ISA_AM_NATIVE__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr elfheader;
  Elf_Phdr programheader;
  int fd = fs_open(filename,0,0);
  assert(fd!=-1);
  fs_read(fd,&elfheader,sizeof(Elf_Ehdr));
  fs_lseek(fd,elfheader.e_phoff,SEEK_SET);
  size_t open_offset;

  for (uint16_t i=0; i<elfheader.e_phnum; i++){

    fs_read(fd,&programheader,sizeof(Elf_Phdr));
    open_offset = fs_open_offset(fd);
    if(programheader.p_type == PT_LOAD){
      fs_lseek(fd,programheader.p_offset,SEEK_SET);
      uint8_t buf[programheader.p_filesz];
      fs_read(fd,&buf,programheader.p_filesz);
      memcpy((void*)programheader.p_vaddr,&buf,programheader.p_filesz);
      memset((void*)(programheader.p_vaddr+programheader.p_filesz),0,(programheader.p_memsz-programheader.p_filesz));
    }
    fs_lseek(fd,open_offset,SEEK_SET);

  }
  fs_close(fd);
  return elfheader.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %x", entry);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void *entry) {
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _kcontext(stack, entry, NULL);
}

void context_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);

  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}
