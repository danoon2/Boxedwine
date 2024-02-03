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

#include "boxedwine.h"

#include "kelf.h"
#include "loader.h"

#include <string.h>

#include UNISTD

#define SHF_WRITE 1
#define SHF_ALLOC 2

#define SHT_NOBITS 8

#define PT_LOAD 1
#define PT_INTERP 3 

#ifdef __ARMEB__
#  error "Big-Endian Arch is not supported"
#endif

bool isValidElf(struct k_Elf32_Ehdr* hdr) {
    if (hdr->e_ident[0] != 0x7F || hdr->e_ident[1] != 'E' || hdr->e_ident[2] != 'L' || hdr->e_ident[3] != 'F') {
        return false;
    }
    if (hdr->e_ident[4] != 1) {
        return false;
    }
    if (hdr->e_ident[5] != 1) {
        return false;
    }
    return true;
}

BString ElfLoader::getInterpreter(FsOpenNode* openNode, bool* isElf) {
    U8 buffer[sizeof(struct k_Elf32_Ehdr)] = { 0 };
    struct k_Elf32_Ehdr* hdr = (struct k_Elf32_Ehdr*)buffer;
    U32 len = openNode->readNative(buffer, sizeof(buffer));
    char interp[MAX_FILEPATH_LEN] = { 0 };

    *isElf=true;
    if (len!=sizeof(buffer)) {
        *isElf=true;
    }
    if (*isElf) {		
        *isElf = isValidElf(hdr);
    }
    if (!*isElf) {
        if (buffer[0]=='#') {
            U32 mode = 0;
            U32 pos = 0;
            char shell_interp[MAX_FILEPATH_LEN] = { 0 };

            for (U32 i=1;i<len;i++) {
                if (mode==0) {
                    if (buffer[i]=='!')
                        mode = 1;
                } else if (mode==1 && (buffer[i]==' ' || buffer[i]=='\t')) {
                    continue;
                } else if (buffer[i]=='\n' || buffer[i]=='\r') {
                    break;
                } else {
                    mode = 2;
                    shell_interp[pos++] = buffer[i];
                }
            }
            shell_interp[pos++]=0;
            return BString::copy(shell_interp);
        }
    } else {
        U32 i;

        openNode->seek(hdr->e_phoff);	
        for (i=0;i<hdr->e_phoff;i++) {
            struct k_Elf32_Phdr phdr;		
            openNode->seek(hdr->e_phoff+i*hdr->e_phentsize);	
            openNode->readNative((U8*)&phdr, sizeof(struct k_Elf32_Phdr));
            if (phdr.p_type==PT_INTERP) {
                openNode->seek(phdr.p_offset);	
                openNode->readNative((U8*)interp, phdr.p_filesz);
                interp[phdr.p_filesz] = 0;
                return BString::copy(interp);
            }
        }
    }
    return B("");
}

FsOpenNode* ElfLoader::inspectNode(BString currentDirectory, const std::shared_ptr<FsNode>& node, BString& loader, BString& interpreter, std::vector<BString>& interpreterArgs) {
    bool isElf = 0;
    FsOpenNode* openNode = nullptr;
    std::shared_ptr<FsNode> interpreterNode;
    std::shared_ptr<FsNode> loaderNode;

    if (node) {
        openNode = node->open(K_O_RDONLY);
    }
    if (openNode) {        
        interpreter = getInterpreter(openNode, &isElf);
        if (isElf) {
            loader = interpreter;
            interpreter = B("");
        } else if (interpreter.length()) {
            interpreter.split(' ', interpreterArgs);
            interpreter = interpreterArgs[0];
            interpreterArgs.erase(interpreterArgs.begin());
        }
        openNode->close();
        delete openNode;
    }
    if (!interpreter.length() && !isElf) {
        return nullptr;
    }
    if (interpreter.length()) {
        interpreterNode = Fs::getNodeFromLocalPath(currentDirectory, interpreter, true);	
        if (!interpreterNode) {
            kwarn("Interpreter not found: %s", interpreter.c_str());
            return nullptr;
        }
        openNode = interpreterNode->open(K_O_RDONLY);		
        loader = ElfLoader::getInterpreter(openNode, &isElf);
        openNode->close();
        delete openNode;
    }
    if (loader.length()) {
        loaderNode = Fs::getNodeFromLocalPath(currentDirectory, loader, true);	
        if (!loaderNode) {
            return nullptr;
        }
    }		

    if (loaderNode) {
        return loaderNode->open(K_O_RDONLY);
    } else if (interpreterNode) {
        return interpreterNode->open(K_O_RDONLY);		
    } else {
        return node->open(K_O_RDONLY);
    }
}

int ElfLoader::getMemSizeOfElf(FsOpenNode* openNode) {
    U8 buffer[sizeof(struct k_Elf32_Ehdr)] = { 0 };
    struct k_Elf32_Ehdr* hdr = (struct k_Elf32_Ehdr*)buffer;
    U64 pos = openNode->getFilePointer();
    U32 address = 0xFFFFFFFF;
    int sections = 0;

    openNode->seek(0);
    U32 len = openNode->readNative(buffer, sizeof(buffer));
    if (len != sizeof(buffer)) {
        return 0;
    }
    if (!isValidElf(hdr)) {
        openNode->seek(pos);
        return 0;
    }

    len = 0;
    openNode->seek(hdr->e_phoff);
    for (int i = 0; i<hdr->e_phnum; i++) {
        struct k_Elf32_Phdr phdr;
        openNode->readNative((U8*)&phdr, sizeof(struct k_Elf32_Phdr));
        if (phdr.p_type == PT_LOAD) {
            if (phdr.p_paddr<address)
                address = phdr.p_paddr;
            if (len<phdr.p_paddr + phdr.p_memsz)
                len = phdr.p_paddr + phdr.p_memsz;
            sections++;
        }
    }
    openNode->seek(pos);
    return len - address + 4096*sections; // 4096 for alignment
}
#if 0
U32 getPELoadAddress(struct FsOpenNode* openNode, U32* section, U32* numberOfSections, U32* sizeOfSection) {
    static U8 buffer[1024];	
    U64 pos = openNode->func->getFilePointer(openNode);
    U32 len;
    int offset;
    U32 sizeOfOptionalHeader;

    openNode->func->seek(openNode, 0);
    len = openNode->func->readNative(openNode, buffer, sizeof(buffer));
    openNode->func->seek(openNode, pos);
    if (len != sizeof(buffer)) {		
        return FALSE;
    }
    // DOS Magic MZ
    if (buffer[0] != 0x4D || buffer[1] != 0x5A) {
        return 0;
    }

    // offset is pointer to IMAGE_NT_HEADERS
    offset = buffer[0x3C] | ((int)buffer[0x3D] << 8);

    // check IMAGE_NT_HEADERS.Signature
    if (buffer[offset] != 0x50 || buffer[offset + 1] != 0x45 || buffer[offset + 2] != 0 || buffer[offset + 3] != 0) {
        return 0;
    }
    // IMAGE_NT_HEADERS.FileHeader.NumberOfSections
    *numberOfSections = buffer[offset + 0x6] | ((U32)buffer[offset + 0x7] << 8);

    *sizeOfSection = 0x28;

    // IMAGE_NT_HEADERS.FileHeader.SizeOfOptionalHeader
    sizeOfOptionalHeader = buffer[offset + 0x14] | ((U32)buffer[offset + 0x15] << 8);

    // section should not reference buffer, but be mapped into memory
    *section = (U32)(buffer + offset + 0x14 /*sizeof(IMAGE_FILE_HEADER)*/ + sizeOfOptionalHeader + 4 /*sizeof(IMAGE_NT_HEADERS.Signature)*/);

    // IMAGE_NT_HEADERS.OptionalHeader.ImageBase
    return buffer[offset + 0x34] | ((U32)buffer[offset + 0x35] << 8) | ((U32)buffer[offset + 0x36] << 16) | ((U32)buffer[offset + 0x37] << 24);
}
#endif
bool ElfLoader::loadProgram(KThread* thread, FsOpenNode* openNode, U32* eip) {
    U8 buffer[sizeof(struct k_Elf32_Ehdr)] = { 0 };
    struct k_Elf32_Ehdr* hdr = (struct k_Elf32_Ehdr*)buffer;
    U32 len = openNode->readNative(buffer, sizeof(buffer));
    U32 address=0xFFFFFFFF;
    U32 reloc = 0;

    if (len!=sizeof(buffer)) {
        return false;
    }
    if (!isValidElf(hdr))
        return false;    
    len=0;
    openNode->seek(hdr->e_phoff);	
    for (U32 i=0;i<hdr->e_phnum;i++) {
        struct k_Elf32_Phdr phdr;		
        openNode->readNative((U8*)&phdr, sizeof(struct k_Elf32_Phdr));
        if (phdr.p_type==PT_LOAD) {
            if (phdr.p_paddr<address) {
                address=phdr.p_paddr;
            }
            if (len<phdr.p_paddr+phdr.p_memsz)
                len=phdr.p_paddr+phdr.p_memsz;
        }
    }

    if (address>0x10000) {
        reloc = 0;
        len-=address;
    } else {
        reloc = ADDRESS_PROCESS_LOADER<<K_PAGE_SHIFT;
        address = reloc;
    }

    if (reloc)
        address = thread->memory->mmap(thread, address, len, K_PROT_READ | K_PROT_WRITE | K_PROT_EXEC, K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED, -1, 0);
    thread->process->loaderBaseAddress = address;
    thread->process->brkEnd = address+len;
    thread->process->phdr = 0;

    for (U32 i=0;i<hdr->e_phnum;i++) {
        struct k_Elf32_Phdr phdr;		
        openNode->seek(hdr->e_phoff+hdr->e_phentsize*i);
        openNode->readNative((U8*)&phdr, sizeof(struct k_Elf32_Phdr));
        if (phdr.p_type==PT_LOAD) {
            if (!reloc) {
                U32 addr = phdr.p_paddr;
                U32 sectionLen = phdr.p_memsz;

                if (phdr.p_paddr & 0xFFF) {
                    addr &= 0xFFFFF000;
                    sectionLen+=(phdr.p_memsz+(phdr.p_paddr-addr));
                }
                thread->memory->mmap(thread, addr, sectionLen, K_PROT_READ | K_PROT_WRITE | K_PROT_EXEC, K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED, -1, 0);
            }
            if (phdr.p_filesz>0) {
                if (phdr.p_offset<=hdr->e_phoff && hdr->e_phoff<phdr.p_offset+phdr.p_filesz) {
                    thread->process->phdr = reloc+phdr.p_paddr+hdr->e_phoff-phdr.p_offset;
                }
                openNode->seek(phdr.p_offset);                
                openNode->read(thread, reloc+phdr.p_paddr, phdr.p_filesz);		
            }
        }
    }
    thread->process->phentsize=hdr->e_phentsize;
    thread->process->phnum=hdr->e_phnum;

    *eip = hdr->e_entry+reloc;
    thread->process->entry = *eip;
    return true;
}
