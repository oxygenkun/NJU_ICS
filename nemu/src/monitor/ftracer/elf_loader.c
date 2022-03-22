#include <stdio.h>

#include "elf_loader.h"
#include <elf.h>

static void read_from_file(FILE *elf, size_t offset, size_t size, void *dest) {
  fseek(elf, offset, SEEK_SET);
  int flag = fread(dest, size, 1, elf);
  assert(flag == 1);
}

static void get_str_from_file(FILE *elf, size_t offset, size_t n, void *dest) {
  fseek(elf, offset, SEEK_SET);
  char *flag = fgets(dest, n, elf);
  assert(flag != NULL);
}

static FUNC_INFO func_info_list[FUNC_LIST_MAX];
int func_info_num = 0;

void init_elf(const char *elf_file) {
  FILE *elf = fopen(elf_file, "rb");
  assert(elf != NULL);
  // read elf header
  Elf32_Ehdr elf_header;
  read_from_file(elf, 0, sizeof(Elf32_Ehdr), &elf_header);
  assert(sizeof(Elf32_Shdr) == elf_header.e_shentsize);

  // read section headers.
  // find symbol section and string section.
  Elf32_Shdr sym_shdr = {};
  Elf32_Shdr str_shdr = {};
  Log("Section headers num: %d", elf_header.e_shnum);
  for (int sh_i = 0; sh_i < elf_header.e_shnum; ++sh_i) {
    Elf32_Shdr section_header;
    size_t section_header_offset =
        elf_header.e_shoff + elf_header.e_shentsize * sh_i;
    read_from_file(elf, section_header_offset, sizeof(Elf32_Shdr),
                   &section_header);

    if (section_header.sh_type == SHT_SYMTAB) {
      sym_shdr = section_header;
    } else if (section_header.sh_type == SHT_STRTAB &&
               sh_i != elf_header.e_shstrndx) {
      str_shdr = section_header;
    }
  }
  assert(sym_shdr.sh_type && str_shdr.sh_type); // all not 0

  // read each symbol entry and there name string.
  //   Log("read each symbol entry and there name string.");
  for (int i = 0; i < sym_shdr.sh_size / sym_shdr.sh_entsize; ++i) {
    Elf32_Sym symbol_entry = {};
    Elf32_Off symbol_entry_offset =
        sym_shdr.sh_offset + sym_shdr.sh_entsize * i;
    read_from_file(elf, symbol_entry_offset, sym_shdr.sh_entsize,
                   &symbol_entry);

    // Log("read symbol_entry %d", ELF32_ST_TYPE[symbol_entry.st_info]);
    if (ELF32_ST_TYPE(symbol_entry.st_info) == STT_FUNC) {
      char *fname = func_info_list[func_info_num].fname;
      Elf32_Off string_offset = str_shdr.sh_offset + symbol_entry.st_name;
      get_str_from_file(elf, string_offset, 64, fname);

      func_info_list[func_info_num].faddr = symbol_entry.st_value;
      func_info_list[func_info_num].fsize = symbol_entry.st_size;
      ++func_info_num;
    }
  }
  Log("[frace] number of functions %d:", func_info_num);
}

FUNC_INFO* get_func_info(paddr_t addr){
    for(int i=0; i<func_info_num; ++i){
        if(addr >= func_info_list[i].faddr && 
            addr < func_info_list[i].faddr + func_info_list[i].fsize){
            return &func_info_list[i];
        }
    }
    return NULL;
}
