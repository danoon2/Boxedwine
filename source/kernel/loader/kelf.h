/*
 *  Copyright (C) 2016  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __kelf_H__
#define __kelf_H__

#include "platform.h"

#define k_EI_NIDENT 16

#define k_Elf32_Addr U32
#define k_Elf32_Half U16
#define k_Elf32_Off U32
#define k_Elf32_Sword S32
#define k_Elf32_Word U32

PACKED(
struct k_Elf32_Ehdr{
    unsigned char   e_ident[k_EI_NIDENT];
    k_Elf32_Half      e_type;
    k_Elf32_Half      e_machine;
    k_Elf32_Word      e_version;
    k_Elf32_Addr      e_entry;
    k_Elf32_Off       e_phoff;
    k_Elf32_Off       e_shoff;
    k_Elf32_Word      e_flags;
    k_Elf32_Half      e_ehsize;
    k_Elf32_Half      e_phentsize;
    k_Elf32_Half      e_phnum;
    k_Elf32_Half      e_shentsize;
    k_Elf32_Half      e_shnum;
    k_Elf32_Half      e_shstrndx;
}
);

PACKED(
struct k_Elf32_Shdr
{
    k_Elf32_Word    sh_name;
    k_Elf32_Word    sh_type;
    k_Elf32_Word    sh_flags;
    k_Elf32_Addr    sh_addr;
    k_Elf32_Off     sh_offset;
    k_Elf32_Word    sh_size;
    k_Elf32_Word    sh_link;
    k_Elf32_Word    sh_info;
    k_Elf32_Word    sh_addralign;
    k_Elf32_Word    sh_entsize;
}
);

PACKED(
struct k_Elf32_Phdr{
    k_Elf32_Word      p_type;
    k_Elf32_Off       p_offset;
    k_Elf32_Addr      p_vaddr;
    k_Elf32_Addr      p_paddr;
    k_Elf32_Word      p_filesz;
    k_Elf32_Word      p_memsz;
    k_Elf32_Word      p_flags;
    k_Elf32_Word      p_align;
}
);

#endif
