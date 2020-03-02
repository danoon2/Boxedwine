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

void readResourceDirectory(FILE* f, uint32_t resourceVirtualAddress, uint32_t baseResourceFileOffset, unsigned char* resourceBuffer, uint32_t offsetFromBaseResource, uint32_t resourceBufferSize, ImageResourceDirectory* dir, uint16_t type, bool isIcon, std::vector<IconInfo>& icons) {
    ImageResourceDirectoryEntry* entry = (ImageResourceDirectoryEntry*)(((unsigned char*)dir)+sizeof(ImageResourceDirectory));
    for (int i=0;i<dir->NumberOfNamedEntries+dir->NumberOfIdEntries;i++, entry++) {
        if (entry->DirectoryInfo.DataIsDirectory) {
            if (entry->DirectoryInfo.OffsetToDirectory<offsetFromBaseResource || entry->DirectoryInfo.OffsetToDirectory+sizeof(ImageResourceDirectory)>=offsetFromBaseResource+resourceBufferSize) {
                unsigned char buffer[4096];
                int newOffsetFromBaseResource = entry->DirectoryInfo.OffsetToDirectory & 0xFFFFF000;
                fseek(f, offsetFromBaseResource+baseResourceFileOffset, SEEK_SET);
                fread(buffer, 1, 4096, f);
                readResourceDirectory(f, resourceVirtualAddress, baseResourceFileOffset, buffer, newOffsetFromBaseResource, 4096, (ImageResourceDirectory*)(entry->DirectoryInfo.OffsetToDirectory+resourceBuffer), entry->Id, isIcon || type==3, icons);
            } else {
                readResourceDirectory(f, resourceVirtualAddress, baseResourceFileOffset, resourceBuffer, offsetFromBaseResource, resourceBufferSize, (ImageResourceDirectory*)(entry->DirectoryInfo.OffsetToDirectory+resourceBuffer), entry->Id, isIcon || type==3, icons);
            }
        } else {
            if (isIcon) {
                ImageResourceDataEntry* data;
                if (entry->DirectoryInfo.OffsetToDirectory>=offsetFromBaseResource && entry->DirectoryInfo.OffsetToDirectory+sizeof(ImageResourceDirectory)<offsetFromBaseResource+resourceBufferSize) {
                    data = (ImageResourceDataEntry*)(entry->OffsetToData+resourceBuffer);
                } else {
                    static unsigned char buffer[sizeof(ImageResourceDirectory)];
                    fseek(f, entry->DirectoryInfo.OffsetToDirectory+baseResourceFileOffset, SEEK_SET);
                    fread(buffer, 1, sizeof(ImageResourceDirectory), f);
                    data = (ImageResourceDataEntry*)buffer;
                }
                if (data->Size>0 && data->Size<1024*1024) {
                    int width=0;
                    int height=0;
                    int comp=4;
                    fseek(f, (data->OffsetToData-resourceVirtualAddress)+baseResourceFileOffset, SEEK_SET);
                    IconInfo info;
                    fread(&info.bih, 1, sizeof(BitmapInfoHeader), f);
                    info.fileOffset = (data->OffsetToData-resourceVirtualAddress)+baseResourceFileOffset;
                    icons.push_back(info);
                }
            }
        }
    }
}

class IconPalette {
public:
    U32 palette[256];
    U32 paletteSize;
};

void loadIconPalette(BitmapInfoHeader& bih, FILE* f, IconPalette& palette) {
	int depth = bih.biBitCount;
	if (depth <= 8) {
		int numColors = bih.biClrUsed;
		if (numColors == 0) {
			numColors = 1 << depth;
		} else {
			if (numColors > 256)
				numColors = 256;
		}
        fread(palette.palette, 1, numColors*4, f);
        palette.paletteSize = numColors;
	} else if (depth == 16) {
        palette.palette[0] = 0x7C00;
        palette.palette[1] = 0x03E0;
        palette.palette[2] = 0x001F;
        palette.paletteSize = 3;
    } else if (depth == 24) {
        palette.palette[0] = 0xFF;
        palette.palette[1] = 0xFF00;
        palette.palette[2] = 0xFF0000;
        palette.paletteSize = 3;
    } else {
        palette.palette[0] = 0xFF00;
        palette.palette[1] = 0xFF0000;
        palette.palette[2] = 0xFF000000;
        palette.paletteSize = 3;
    }
}

unsigned char* loadData(BitmapInfoHeader& bih, FILE* f, int stride) {
    if (bih.biCompression != 0) { // BMP_NO_COMPRESSION
        kwarn("Compressed icon was not handled");
        return NULL;
    }
	int dataSize = bih.biHeight * stride;
	unsigned char* data = new unsigned char[dataSize];
    fread(data, 1, dataSize, f);
	return data;
}

static void flipScanLines(unsigned char* data, int stride, int height) {
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

unsigned char* loadData(BitmapInfoHeader& bih, FILE* f) {
	int stride = (bih.biWidth * bih.biBitCount + 7) / 8;
	stride = (stride + 3) / 4 * 4;
	unsigned char* result = loadData(bih, f, stride);
    if (result) {
	    flipScanLines(result, stride, bih.biHeight);
    }
	return result;
}

const unsigned char* parseIcon(FILE* f, IconInfo& info, int* width, int* height) {
    IconPalette palette;
    fseek(f, info.fileOffset+sizeof(BitmapInfoHeader), SEEK_SET);
    loadIconPalette(info.bih, f, palette);
    info.bih.biHeight/=2;
    unsigned char* color = loadData(info.bih, f);
    int bpp = info.bih.biBitCount;
	info.bih.biBitCount = 1;
	unsigned char* maskData = loadData(info.bih, f);

    if (bpp==32) {
        swapRGB(color, info.bih.biWidth, info.bih.biHeight);
        *width = info.bih.biWidth;
        *height = info.bih.biHeight;
        return color;
    } else if (bpp==4) {
        *width = info.bih.biWidth;
        *height = info.bih.biHeight;
        unsigned char* result = new unsigned char[info.bih.biWidth * info.bih.biHeight * 4];
        for (int y=0;y<info.bih.biHeight;y++) {
            for (int x=0;x<info.bih.biWidth;x++) {
                int index = y*info.bih.biWidth*4+x*4;
                U32 byteIndex = y*info.bih.biWidth+x;
                U8 paletteIndex = color[byteIndex/2];
                if (byteIndex & 1) {
                    paletteIndex = (paletteIndex >> 4) & 0xF;
                } else {
                    paletteIndex = paletteIndex & 0xF;
                }
                U32 c = palette.palette[paletteIndex];
                result[index+2] = c & 0xFF;
                result[index+1] = (c >> 8) & 0xFF;
                result[index] = (c >> 16) & 0xFF;
                if (maskData[byteIndex/8] & (1 << (7-(byteIndex % 8)))) {
                    result[index+3] = 0;
                } else {
                    result[index+3] = 0xFF;
                }
            }
        }
        return result;
    } else if (bpp==8) {
        *width = info.bih.biWidth;
        *height = info.bih.biHeight;
        unsigned char* result = new unsigned char[info.bih.biWidth * info.bih.biHeight * 4];
        for (int y=0;y<info.bih.biHeight;y++) {
            for (int x=0;x<info.bih.biWidth;x++) {
                int index = y*info.bih.biWidth*4+x*4;
                int byteIndex = y*info.bih.biWidth+x;
                U32 c = palette.palette[color[byteIndex]];
                result[index+2] = c & 0xFF;
                result[index+1] = (c >> 8) & 0xFF;
                result[index] = (c >> 16) & 0xFF;
                if (maskData[byteIndex/8] & (1 << (7-(byteIndex % 8)))) {
                    result[index+3] = 0;
                } else {
                    result[index+3] = 0xFF;
                }
            }
        }
        return result;
    }
    return NULL;
}

const unsigned char* extractIconFromExe(BoxedContainer* container, const std::string& exeLocalPath, int size, int* width, int* height) {
    std::string exeNativePath = GlobalSettings::getRootFolder(container)+Fs::nativeFromLocal(exeLocalPath);    
    
    FILE* f = fopen(exeNativePath.c_str(), "rb");
    if (!f) {
        return false;
    }
    unsigned char* buffer = new unsigned char[32*1024];
    U32 read = (U32)fread(buffer, 1, 1024, f);
    if (buffer[0]!='M' || buffer[1]!='Z') {
        fclose(f);
        return NULL;
    }
    U32 nextHeader = READD(buffer, 60);    // e_lfanew File address of new exe header
    if (nextHeader>read-1) {
        fclose(f);
        return NULL;
    }
    if (buffer[nextHeader]=='N' && buffer[nextHeader+1]=='E') {
    }
    if (buffer[nextHeader]=='P' && buffer[nextHeader+1]=='E') {
        ImageFileHeader* header = PEFHDROFFSET(buffer);
        Pe32OptionalHeader* optionalHeader = (Pe32OptionalHeader*)(buffer+nextHeader+sizeof(PeHeader));
        ImageSectionHeader* sectionHeader = (ImageSectionHeader*)(((unsigned char*)optionalHeader)+header->SizeOfOptionalHeader);

        U32 resourceRVA;
        U32 resourceSize;
        if (optionalHeader->Magic == 0x010b) {
            resourceRVA = *(U32*)(buffer+nextHeader+sizeof(PeHeader)+sizeof(Pe32OptionalHeader)+16); // 8 bytes per entry, resources is at 3 rd entry
            resourceSize = *(U32*)(buffer+nextHeader+sizeof(PeHeader)+sizeof(Pe32OptionalHeader)+20);
            U32 rawOffset = resourceRVA;
            for (int i=0;i<header->NumberOfSections;i++) {
                ImageSectionHeader* section = sectionHeader+i;

                if (section->virtual_address == resourceRVA) {
                    rawOffset = section->pointer_to_raw_data;
                }
            }

            if (resourceSize) {
                fseek(f, rawOffset, SEEK_SET);
                read = (U32)fread(buffer, 1, 32*1024, f);
                std::vector<IconInfo> icons;
                readResourceDirectory(f, resourceRVA, rawOffset, buffer, 0, 32*1024, (ImageResourceDirectory*)buffer, 0, false, icons);
                if (icons.size()>0) {
                    IconInfo bestIcon = icons[0];
                    for (int i=1;i<(int)icons.size();i++) {
                        if (bestIcon.bih.biCompression) {
                            continue;
                        }
                        if (bestIcon.bih.biWidth!=size && icons[i].bih.biWidth==size) {
                            bestIcon = icons[i];
                        } else if (bestIcon.bih.biBitCount && icons[i].bih.biBitCount && bestIcon.bih.biBitCount<icons[i].bih.biBitCount && icons[i].bih.biBitCount<=32) {
                            bestIcon = icons[i];
                        }
                    }
                    const unsigned char* result = parseIcon(f, bestIcon, width, height);
                    fclose(f);
                    return result;
                }                
            }
        } else if (optionalHeader->Magic == 0x020b) {

        } else {
            fclose(f);
            return NULL;
        }        
    }
    fclose(f);
    return NULL;
}