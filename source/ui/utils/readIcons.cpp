#include "boxedwine.h"
#include "../boxedwineui.h"

PACKED(
struct NeHeader {
    char sig[2];                 //"NE"
    uint8_t MajLinkerVersion;    //The major linker version
    uint8_t MinLinkerVersion;    //The minor linker version
    uint16_t EntryTableOffset;   //Offset of entry table, see below
    uint16_t EntryTableLength;   //Length of entry table in bytes
    uint32_t FileLoadCRC;        //32-bit CRC of entire contents of file
    uint8_t ProgFlags;           //Program flags, bitmapped
    uint8_t ApplFlags;           //Application flags, bitmapped
    uint16_t AutoDataSegIndex;   //The automatic data segment index
    uint16_t InitHeapSize;       //The intial local heap size
    uint16_t InitStackSize;      //The inital stack size
    uint32_t EntryPoint;         //CS:IP entry point, CS is index into segment table
    uint32_t InitStack;          //SS:SP inital stack pointer, SS is index into segment table
    uint16_t SegCount;           //Number of segments in segment table
    uint16_t ModRefs;            //Number of module references (DLLs)
    uint16_t NoResNamesTabSiz;   //Size of non-resident names table, in bytes (Please clarify non-resident names table)
    uint16_t SegTableOffset;     //Offset of Segment table
    uint16_t ResTableOffset;     //Offset of resources table
    uint16_t ResidNamTable;      //Offset of resident names table
    uint16_t ModRefTable;        //Offset of module reference table
    uint16_t ImportNameTable;    //Offset of imported names table (array of counted strings, terminated with string of length 00h)
    uint32_t OffStartNonResTab;  //Offset from start of file to non-resident names table
    uint16_t MovEntryCount;      //Count of moveable entry point listed in entry table
    uint16_t FileAlnSzShftCnt;   //File alligbment size shift count (0=9(default 512 byte pages))
    uint16_t nResTabEntries;     //Number of resource table entries
    uint8_t targOS;              //Target OS
    uint8_t OS2EXEFlags;         //Other OS/2 flags
    uint16_t retThunkOffset;     //Offset to return thunks or start of gangload area - what is gangload?
    uint16_t segrefthunksoff;    //Offset to segment reference thunks or size of gangload area
    uint16_t mincodeswap;        //Minimum code swap area size
    uint8_t expctwinver[2];      //Expected windows version (minor first)
}
);

PACKED(
struct PeHeader {
	uint32_t mMagic; // PE\0\0 or 0x00004550
	uint16_t mMachine;
	uint16_t mNumberOfSections;
	uint32_t mTimeDateStamp;
	uint32_t mPointerToSymbolTable;
	uint32_t mNumberOfSymbols;
	uint16_t mSizeOfOptionalHeader;
	uint16_t mCharacteristics;
});

PACKED(
struct ImageFileHeader {
  uint16_t  Machine;
  uint16_t  NumberOfSections;
  uint32_t  TimeDateStamp;
  uint32_t  PointerToSymbolTable;
  uint32_t  NumberOfSymbols;
  uint16_t  SizeOfOptionalHeader;
  uint16_t  Characteristics;
});

PACKED(
struct Pe32OptionalHeader {
	uint16_t Magic; // 0x010b - PE32, 0x020b - PE32+ (64 bit)
	uint8_t  MajorLinkerVersion;
	uint8_t  MinorLinkerVersion;
	uint32_t SizeOfCode;
	uint32_t SizeOfInitializedData;
	uint32_t SizeOfUninitializedData;
	uint32_t AddressOfEntryPoint;
	uint32_t BaseOfCode;
	uint32_t BaseOfData;
	uint32_t ImageBase;
	uint32_t SectionAlignment;
	uint32_t FileAlignment;
	uint16_t MajorOperatingSystemVersion;
	uint16_t MinorOperatingSystemVersion;
	uint16_t MajorImageVersion;
	uint16_t MinorImageVersion;
	uint16_t MajorSubsystemVersion;
	uint16_t MinorSubsystemVersion;
	uint32_t Win32VersionValue;
	uint32_t SizeOfImage;
	uint32_t SizeOfHeaders;
	uint32_t CheckSum;
	uint16_t Subsystem;
	uint16_t DllCharacteristics;
	uint32_t SizeOfStackReserve;
	uint32_t SizeOfStackCommit;
	uint32_t SizeOfHeapReserve;
	uint32_t SizeOfHeapCommit;
	uint32_t LoaderFlags;
	uint32_t NumberOfRvaAndSizes;
});

PACKED(
struct DosHeader
{
     uint16_t e_magic;
     uint16_t e_cblp;
     uint16_t e_cp;
     uint16_t e_crlc;
     uint16_t e_cparhdr;
     uint16_t e_minalloc;
     uint16_t e_maxalloc;
     uint16_t e_ss;
     uint16_t e_sp;
     uint16_t e_csum;
     uint16_t e_ip;
     uint16_t e_cs;
     uint16_t e_lfarlc;
     uint16_t e_ovno;
     uint16_t e_res[4];
     uint16_t e_oemid;
     uint16_t e_oeminfo;
     uint16_t e_res2[10];
     uint32_t e_lfanew;
});

PACKED(
struct ImageResourceDirectory {
    uint32_t   Characteristics;
    uint32_t   TimeDateStamp;
    uint16_t  MajorVersion;
    uint16_t  MinorVersion;
    uint16_t  NumberOfNamedEntries;
    uint16_t  NumberOfIdEntries;
//  IMAGE_RESOURCE_DIRECTORY_ENTRY DirectoryEntries[];
});

PACKED(
struct ImageResourceDirectoryEntry {
    union {
        struct {
            uint32_t NameOffset:31;
            uint32_t NameIsString:1;
        } NameInfo;
        uint32_t   Name;
        uint16_t  Id;
    };
    union {
        uint32_t   OffsetToData;
        struct {
            uint32_t   OffsetToDirectory:31;
            uint32_t   DataIsDirectory:1;
        } DirectoryInfo;
    };
});

PACKED(
struct ImageResourceDataEntry {
    uint32_t   OffsetToData;
    uint32_t   Size;
    uint32_t   CodePage;
    uint32_t   Reserved;
});

PACKED(
struct BitmapInfoHeader {
  uint32_t biSize;
  int32_t  biWidth;
  int32_t  biHeight;
  uint16_t  biPlanes;
  uint16_t  biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  int32_t  biXPelsPerMeter;
  int32_t  biYPelsPerMeter;
  uint32_t biClrUsed;
  uint32_t biClrImportant;
});

PACKED(
struct ImageOs2Header {      // OS/2 .EXE header
    uint16_t ne_magic;                    // Magic number
    int8_t   ne_ver;                      // Version number
    int8_t   ne_rev;                      // Revision number
    uint16_t ne_enttab;                   // Offset of Entry Table
    uint16_t ne_cbenttab;                 // Number of bytes in Entry Table
    int32_t   ne_crc;                      // Checksum of whole file
    uint16_t ne_flags;                    // Flag word
    uint16_t ne_autodata;                 // Automatic data segment number
    uint16_t ne_heap;                     // Initial heap allocation
    uint16_t ne_stack;                    // Initial stack allocation
    int32_t   ne_csip;                     // Initial CS:IP setting
    int32_t   ne_sssp;                     // Initial SS:SP setting
    uint16_t ne_cseg;                     // Count of file segments
    uint16_t ne_cmod;                     // Entries in Module Reference Table
    uint16_t ne_cbnrestab;                // Size of non-resident name table
    uint16_t ne_segtab;                   // Offset of Segment Table
    uint16_t ne_rsrctab;                  // Offset of Resource Table
    uint16_t ne_restab;                   // Offset of resident name table
    uint16_t ne_modtab;                   // Offset of Module Reference Table
    uint16_t ne_imptab;                   // Offset of Imported Names Table
    int32_t   ne_nrestab;                  // Offset of Non-resident Names Table
    uint16_t ne_cmovent;                  // Count of movable entries
    uint16_t ne_align;                    // Segment alignment shift count
    uint16_t ne_cres;                     // Count of resource segments
    uint8_t  ne_exetyp;                   // Target Operating system
    uint8_t  ne_flagsothers;              // Other .EXE flags
    uint16_t ne_pretthunks;               // offset to return thunks
    uint16_t ne_psegrefbytes;             // offset to segment ref. bytes
    uint16_t ne_swaparea;                 // Minimum code swap area size
    uint16_t ne_expver;                   // Expected Windows version number
  });

#define IMAGE_SIZEOF_SHORT_NAME 8
#define IMAGE_SCN_CNT_CODE          0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA      0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA    0x00000080

PACKED(
struct ImageSectionHeader {
    uint8_t name[IMAGE_SIZEOF_SHORT_NAME];
    union {
        uint32_t physical_address;
        uint32_t virtual_size;
    } misc;
    uint32_t virtual_address;
    uint32_t size_of_raw_data;
    uint32_t pointer_to_raw_data;
    uint32_t pointer_to_relocations;
    uint32_t pointer_to_linenumbers;
    uint16_t number_of_relocations;
    uint16_t number_of_linenumbers;
    uint32_t characteristics;
});

#define IMAGE_NT_SIGNATURE 4
#define SIZE_OF_NT_SIGNATURE sizeof(IMAGE_NT_SIGNATURE)
#define OPTHDROFFSET(a) ((Pe32OptionalHeader*)((unsigned char *)a + ((DosHeader*)a)->e_lfanew + SIZE_OF_NT_SIGNATURE + sizeof(ImageFileHeader)))
#define PEFHDROFFSET(a) ((ImageFileHeader*)((unsigned char *)a + ((DosHeader*)a)->e_lfanew + SIZE_OF_NT_SIGNATURE))
#define PE_SECTIONS(module) ((Win32ImageSectionHeader *)((uint8_t *) &PE_HEADER(module)->optional_header + PE_HEADER(module)->file_header.size_of_optional_header))

#define READD(b, offset) ((b[offset]) | (b[offset+1] << 8) | (b[offset+2] << 16) | (b[offset+3] << 24))

class IconInfo {
public:
    BitmapInfoHeader bih;
    U32 fileOffset;
};

void readResourceDirectory(BReadFile& f, uint32_t resourceVirtualAddress, uint32_t baseResourceFileOffset, unsigned char* resourceBuffer, uint32_t offsetFromBaseResource, uint32_t resourceBufferSize, ImageResourceDirectory* dir, uint16_t type, bool isIcon, std::vector<IconInfo>& icons) {
    ImageResourceDirectoryEntry* entry = (ImageResourceDirectoryEntry*)(((unsigned char*)dir)+sizeof(ImageResourceDirectory));
    for (int i=0;i<dir->NumberOfNamedEntries+dir->NumberOfIdEntries;i++, entry++) {
        if (entry->DirectoryInfo.DataIsDirectory) {
            if (entry->DirectoryInfo.OffsetToDirectory<offsetFromBaseResource || entry->DirectoryInfo.OffsetToDirectory+sizeof(ImageResourceDirectory)>=offsetFromBaseResource+resourceBufferSize) {
                unsigned char buffer[4096] = {};
                int newOffsetFromBaseResource = entry->DirectoryInfo.OffsetToDirectory & 0xFFFFF000;
                f.setPos(offsetFromBaseResource+baseResourceFileOffset);
                f.read(buffer, 4096);
                readResourceDirectory(f, resourceVirtualAddress, baseResourceFileOffset, buffer, newOffsetFromBaseResource, 4096, (ImageResourceDirectory*)(entry->DirectoryInfo.OffsetToDirectory+resourceBuffer), entry->Id, isIcon || type==3, icons);
            } else {
                readResourceDirectory(f, resourceVirtualAddress, baseResourceFileOffset, resourceBuffer, offsetFromBaseResource, resourceBufferSize, (ImageResourceDirectory*)(entry->DirectoryInfo.OffsetToDirectory+resourceBuffer), entry->Id, isIcon || type==3, icons);
            }
        } else {
            if (isIcon) {
                ImageResourceDataEntry* data = nullptr;
                if (entry->DirectoryInfo.OffsetToDirectory>=offsetFromBaseResource && entry->DirectoryInfo.OffsetToDirectory+sizeof(ImageResourceDirectory)<offsetFromBaseResource+resourceBufferSize) {
                    data = (ImageResourceDataEntry*)(entry->OffsetToData+resourceBuffer);
                } else {
                    static unsigned char buffer[sizeof(ImageResourceDirectory)];
                    f.setPos(entry->DirectoryInfo.OffsetToDirectory+baseResourceFileOffset);
                    f.read(buffer, sizeof(ImageResourceDirectory));
                    data = (ImageResourceDataEntry*)buffer;
                }
                if (data->Size>0 && data->Size<1024*1024) {
                    f.setPos((data->OffsetToData-resourceVirtualAddress)+baseResourceFileOffset);
                    IconInfo info;
                    f.read((char*)&info.bih, sizeof(BitmapInfoHeader));
                    info.fileOffset = (data->OffsetToData-resourceVirtualAddress)+baseResourceFileOffset;
                    icons.push_back(info);
                }
            }
        }
    }
}

void readPalette(BitmapInfoHeader& bih, BReadFile& f, U32* palette) {
	int depth = bih.biBitCount;
	if (depth <= 8) {
		int numColors = bih.biClrUsed;
		if (numColors == 0) {
			numColors = 1 << depth;
		} else {
			if (numColors > 256)
				numColors = 256;
		}
        f.read((char*)palette, numColors*4);
	}
}

std::shared_ptr<U8> loadData(BitmapInfoHeader& bih, BReadFile& f, int stride) {
    if (bih.biCompression != 0) { // BMP_NO_COMPRESSION
        kwarn("Compressed icon was not handled");
        return nullptr;
    }
	int dataSize = bih.biHeight * stride;
    std::shared_ptr<U8> data = std::make_shared<U8>(dataSize);
    f.read(data.get(), dataSize);
	return data;
}

static void flipBitmap(unsigned char* data, int stride, int height) {
	int i1 = 0;
	int i2 = (height - 1) * stride;
	for (int i = 0; i < height / 2; i++) {
		for (int index = 0; index < stride; index++) {
			unsigned char b = data[index + i1];
			data[index + i1] = data[index + i2];
			data[index + i2] = b;
		}
		i1 += stride;
		i2 -= stride;
	}
}

static void swapRGB(U8* data, int height, int width) {
    for (int y=0;y<height;y++) {
        for (int x=0;x<width;x++) {
            int index = y*width*4+x*4;
            U8 tmp = data[index];
            data[index] = data[index+2];
            data[index+2] = tmp;
        }
    }
}

std::shared_ptr<U8[]> loadData(BitmapInfoHeader& bih, BReadFile& f) {
	int stride = (bih.biWidth * bih.biBitCount + 7) / 8;
	stride = (stride + 3) / 4 * 4;
    std::shared_ptr<U8[]> result;

    if (bih.biCompression != 0) { // BMP_NO_COMPRESSION
        kwarn("Compressed icon was not handled");
        return nullptr;
    } else {
	    int dataSize = bih.biHeight * stride;
        result = std::make_shared<U8[]>(dataSize);
        f.read(result.get(), dataSize);
    }

    if (result) {
	    flipBitmap(result.get(), stride, bih.biHeight);
    }
	return result;
}

std::shared_ptr<U8[]> parseIcon(BReadFile& f, IconInfo& info, int* width, int* height) {
    U32 palette[256];
    f.setPos(info.fileOffset+sizeof(BitmapInfoHeader));
    readPalette(info.bih, f, palette);
    info.bih.biHeight/=2;
    std::shared_ptr<U8[]> color = loadData(info.bih, f);
    U8* pColor = color.get();
    int bpp = info.bih.biBitCount;
	info.bih.biBitCount = 1;
    std::shared_ptr<U8[]> maskData = loadData(info.bih, f);
    U8* pMaskData = maskData.get();

    if (bpp==32) {
        swapRGB(color.get(), info.bih.biWidth, info.bih.biHeight);
        *width = info.bih.biWidth;
        *height = info.bih.biHeight;
        return color;
    } else if (bpp==4) {
        int maskStride = (info.bih.biWidth + 31) / 32 * 4;
        *width = info.bih.biWidth;
        *height = info.bih.biHeight;
        std::shared_ptr<U8[]> result = std::make_shared<U8[]>(info.bih.biWidth * info.bih.biHeight * 4);
        U8* pResult = result.get();
        for (int y=0;y<info.bih.biHeight;y++) {
            for (int x=0;x<info.bih.biWidth;x++) {
                int maskIndex = y * maskStride + x / 8;
                int index = y*info.bih.biWidth*4+x*4;
                U32 byteIndex = y*info.bih.biWidth+x;
                U8 paletteIndex = pColor[byteIndex/2];
                if (byteIndex & 1) {
                    paletteIndex = paletteIndex & 0xF;
                } else {
                    paletteIndex = (paletteIndex >> 4) & 0xF;                    
                }
                U32 c = palette[paletteIndex];
                pResult[index+2] = c & 0xFF;
                pResult[index+1] = (c >> 8) & 0xFF;
                pResult[index] = (c >> 16) & 0xFF;
                if (pMaskData[maskIndex] & (1 << (7-(x % 8)))) {
                    pResult[index+3] = 0;
                } else {
                    pResult[index+3] = 0xFF;
                }
            }
        }
        return result;
    } else if (bpp==8) {
        int maskStride = (info.bih.biWidth + 31) / 32 * 4;
        *width = info.bih.biWidth;
        *height = info.bih.biHeight;
        std::shared_ptr<U8[]> result = std::make_shared<U8[]>(info.bih.biWidth * info.bih.biHeight * 4);
        U8* pResult = result.get();
        for (int y=0;y<info.bih.biHeight;y++) {
            for (int x=0;x<info.bih.biWidth;x++) {
                int maskIndex = y * maskStride + x / 8;
                int index = y*info.bih.biWidth*4+x*4; // index into 32-bit result
                int byteIndex = y*info.bih.biWidth+x; // index into icon
                U32 c = palette[pColor[byteIndex]];
                pResult[index+2] = c & 0xFF;
                pResult[index+1] = (c >> 8) & 0xFF;
                pResult[index] = (c >> 16) & 0xFF;
                if (pMaskData[maskIndex] & (1 << (7-(x % 8)))) {
                    pResult[index+3] = 0;
                } else {
                    pResult[index+3] = 0xFF;
                }
            }
        }
        return result;
    } else if (bpp == 24) {
        int stride = (info.bih.biWidth*3 + 3) / 4 * 4;
        int maskStride = (info.bih.biWidth + 31) / 32 * 4;
        *width = info.bih.biWidth;
        *height = info.bih.biHeight;
        std::shared_ptr<U8[]> result = std::make_shared<U8[]>(info.bih.biWidth * info.bih.biHeight * 4);
        U8* pResult = result.get();
        for (int y = 0; y < info.bih.biHeight; y++) {
            for (int x = 0; x < info.bih.biWidth; x++) {
                int maskIndex = y * maskStride + x / 8;
                int index = y * info.bih.biWidth * 4 + x * 4; // index into 32-bit result
                int colorIndex = y * stride + x*3; // index into icon
                U32 c = *(U32*)(&pColor[colorIndex]);
                pResult[index + 2] = c & 0xFF;
                pResult[index + 1] = (c >> 8) & 0xFF;
                pResult[index] = (c >> 16) & 0xFF;
                if (pMaskData[maskIndex] & (1 << (7 - (x % 8)))) {
                    pResult[index + 3] = 0;
                } else {
                    pResult[index + 3] = 0xFF;
                }
            }
        }
        return result;
    }
    return nullptr;
}

/*
from http://bytepointer.com/resources/win16_ne_exe_format_win3.0.htm
======================================================================
                            RESOURCE TABLE
======================================================================

The resource table follows the segment table and contains entries for
each resource in the executable file. The resource table consists of
an alignment shift count, followed by a table of resource records. The
resource records define the type ID for a set of resources. Each
resource record contains a table of resource entries of the defined
type. The resource entry defines the resource ID or name ID for the
resource. It also defines the location and size of the resource. The
following describes the contents of each of these structures:

   Size Description
   ---- -----------

   DW   Alignment shift count for resource data.

   A table of resource type information blocks follows. The following
   is the format of each type information block:

        DW  Type ID. This is an integer type if the high-order bit is
            set (8000h); otherwise, it is an offset to the type string,
            the offset is relative to the beginning of the resource
            table. A zero type ID marks the end of the resource type
            information blocks.

        DW  Number of resources for this type.

        DD  Reserved.

        A table of resources for this type follows. The following is
        the format of each resource (8 bytes each):

            DW  File offset to the contents of the resource data,
                relative to beginning of file. The offset is in terms
                of the alignment shift count value specified at
                beginning of the resource table.

            DW  Length of the resource in the file (in bytes).

            DW  Flag word.
                0010h = MOVEABLE  Resource is not fixed.
                0020h = PURE      Resource can be shared.
                0040h = PRELOAD   Resource is preloaded.

            DW  Resource ID. This is an integer type if the high-order
                bit is set (8000h), otherwise it is the offset to the
                resource string, the offset is relative to the
                beginning of the resource table.

            DD  Reserved.

   Resource type and name strings are stored at the end of the
   resource table. Note that these strings are NOT null terminated and
   are case sensitive.

   DB   Length of the type or name string that follows. A zero value
        indicates the end of the resource type and name string, also
        the end of the resource table.

   DB   ASCII text of the type or name string.
*/

U16 getWord(BReadFile& f) {
    U16 result = 0;
    f.read(result);
    return result;
}

U8 getBye(BReadFile& f) {
    U8 result = 0;
    f.read(result);
    return result;
}

U32 getDoubleWord(BReadFile& f) {
    U32 result = 0;
    f.read(result);
    return result;
}

std::shared_ptr<U8[]> extractIconFromExe(BString nativeExePath, int size, int* width, int* height) {
    BReadFile f(nativeExePath);
    if (!f.isOpen()) {
        return nullptr;
    }
    unsigned char* buffer = new unsigned char[32*1024];
    U32 read = (U32)f.read(buffer, 32 * 1024);
    if (buffer[0]!='M' || buffer[1]!='Z') {
        return nullptr;
    }
    U32 nextHeader = READD(buffer, 60);    // e_lfanew File address of new exe header
    if (nextHeader>read-1) {
        return nullptr;
    }
    std::vector<IconInfo> icons;
    if (buffer[nextHeader]=='N' && buffer[nextHeader+1]=='E') {
        ImageOs2Header* header = (ImageOs2Header*)(buffer+nextHeader);
        if (header->ne_rsrctab >= header->ne_restab) {
            return nullptr;
        }
        f.setPos(nextHeader + header->ne_rsrctab);
        U16 alignShiftCount = getWord(f);
        //U8* resBuffer = (buffer + nextHeader + header->ne_rsrctab+2);
        f.setPos(nextHeader + header->ne_rsrctab+2);

        while (true) {
            U16 typeId = getWord(f);
            U16 count = getWord(f);
            getDoubleWord(f); // reserved
            if (typeId == 0) {
                break;
            }
            if (typeId==0x800e) {
                for (int i=0;i<(int)count;i++) {
                    //U32 fileOffset = ((U32)getWord(f)) << alignShiftCount;
                    //U16 resourceLen = getWord(f) << alignShiftCount;
                    //U16 flags = getWord(f);
                    //U16 id = getWord(f);
                    f.advance(8);
                    f.advance(4); // internal use
                    // RT_GROUP_ICON structure
                    /*
                    U32 pos = (U32)ftell(f);                    
                    fseek(f, fileOffset, SEEK_SET);
                    U16 idReserved = getWord(f);
                    U16 idType = getWord(f);
                    U16 idCount = getWord(f);
                    fseek(f, pos, SEEK_SET);
                    */
                }
            } else if (typeId==0x8003) {
                for (int i=0;i<(int)count;i++) {
                    IconInfo info;
                    info.fileOffset = ((U32)getWord(f)) << alignShiftCount;
                    //U16 resourceLen = getWord(f) << alignShiftCount;
                    //U16 flags = getWord(f);
                    //U16 id = getWord(f);
                    f.advance(6);
                    f.advance(4); // internal use
                    U64 pos = f.getPos();                    
                    f.setPos(info.fileOffset);
                    f.read((char*)&info.bih, sizeof(BitmapInfoHeader));
                    f.setPos(pos);
                    icons.push_back(info);
                }
            } else {
                f.advance(12*count);
            }
        }
    }
    if (buffer[nextHeader]=='P' && buffer[nextHeader+1]=='E') {
        ImageFileHeader* header = PEFHDROFFSET(buffer);
        Pe32OptionalHeader* optionalHeader = (Pe32OptionalHeader*)(buffer+nextHeader+sizeof(PeHeader));
        ImageSectionHeader* sectionHeader = (ImageSectionHeader*)(((unsigned char*)optionalHeader)+header->SizeOfOptionalHeader);

        if (optionalHeader->Magic == 0x010b) {
            U32 resourceRVA = *(U32*)(buffer+nextHeader+sizeof(PeHeader)+sizeof(Pe32OptionalHeader)+16); // 8 bytes per entry, resources is at 3 rd entry
            U32 resourceSize = *(U32*)(buffer+nextHeader+sizeof(PeHeader)+sizeof(Pe32OptionalHeader)+20);
            U32 rawOffset = resourceRVA;
            for (int i=0;i<header->NumberOfSections;i++) {
                ImageSectionHeader* section = sectionHeader+i;

                if (section->virtual_address == resourceRVA) {
                    rawOffset = section->pointer_to_raw_data;
                }
            }

            if (resourceSize) {
                f.setPos(rawOffset);
                read = f.read(buffer, 32*1024);
                readResourceDirectory(f, resourceRVA, rawOffset, buffer, 0, 32*1024, (ImageResourceDirectory*)buffer, 0, false, icons);                                
            }
        } else if (optionalHeader->Magic == 0x020b) {
            kwarn("Icon 20b not implemented");
        } else {
            return nullptr;
        }                
    }
    if (icons.size()>0) {
        IconInfo bestIcon = icons[0];
        for (size_t i=1;i<icons.size();i++) {
            if (bestIcon.bih.biCompression) {
                continue;
            }
            int bestDiff = abs(bestIcon.bih.biWidth - size);
            int diff = abs(icons[i].bih.biWidth - size);
            if (diff<bestDiff) {
                bestIcon = icons[i];
            } else if (bestIcon.bih.biBitCount && icons[i].bih.biBitCount && bestIcon.bih.biBitCount<icons[i].bih.biBitCount && icons[i].bih.biBitCount<=32) {
                bestIcon = icons[i];
            }
        }
        return parseIcon(f, bestIcon, width, height);
    }
    return nullptr;
}
