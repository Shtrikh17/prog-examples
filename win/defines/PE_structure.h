struct IMAGE_DOS_HEADER{
	WORD e_magic;								// MZ
	WORD e_cblp;								// ?
	WORD e_cp;									// ?
	WORD e_crlc;								// ?
	WORD e_cparhdr;								// ?
	WORD e_minalloc;							// ?
	WORD e_maxalloc;							// ?
	WORD e_ss;									// ?
	WORD e_sp;									// ?
	WORD e_lfarlc;								// ?
	WORD e_ovno;								// ?
	WORD e_res[4];								// RESERVED
	WORD e_oemid;								// ?
	WORD e_oeminfo;								// ?
	WORD e_res2[10];							// RESERVER 2
	DWORD e_lfanew;								// PE Header offset
}

struct IMAGE_NT_HEADERS{
	DWORD Signature;							// PE
	IMAGE_FILE_HEADER FileHeader;				// file header
	IMAGE_OPTIONAL_HEADER32 OptionalHeader;		// Optional Header
}

struct IMAGE_FILE_HEADER{
	WORD Machine;								
	WORD NumberOfSections;						
	DWORD TimeDateStamp;						
	DWORD PointerToSymbolTable;					
	DWORD NumberOfSymbols;						
	WORD SizeOfOptionalHeader;					
	WORD Characteristics;						
}

struct IMAGE_OPTIONAL_HEADER32{					// 224 bytes
	WORD magic;									// Magic number
	BYTE MajorLinkerVersion;					// Major Linker Version
	BYTE MinorLinkerVersion;					// Minor Linker Version
	DWORD SizeOfCode;							
	DWORD SizeOfInitializedData;
	DWORD SizeOfUninitializedData;
	DWORD AddressOfEntryPoint;					// Entry Point RVA
	DWORD BaseOfCode;
	DWORD BaseOfData;
	DWORD ImageBase;							// Base address for VA calculation
	DWORD SectionAlignment;						// In-memory alignment of sections (usually 1000h)
	DWORD FileAlignment;						// On-disk alignment (usually 200h)
	WORD MajorOperatingSystemVersion;
	WORD MinorOperatingSystemVersion;
	WORD MajorImageVersion;
	WORD MinorImageVersion;
	WORD MajorSubsystemVersion;
	WORD MinorSubsystemVersion;
	DWORD Win32VersionValue;
	DWORD SizeOfImage;
	DWORD SizeOfHeaders;
	DWORD CheckSum;
	WORD Subsystem;
	WORD DllCharacteristics;
	DWORD SizeOfStackReserve;
	DWORD SizeOfStackCommit;
	DWORD SizeOfHeapReserve;
	DWORD SizeOfHeapCommit;
	DWORD LoaderFlags;
	DWORD NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY DataDirectory[16];		// 128 bytes; 16 structures, each 8 bytes
}

struct IMAGE_DATA_DIRECTORY{
	DWORD VirtualAddress;						// Relative Virtual Address of data structure
	DWORD isize;								// size of data structure in bytes
}

/* DIRECTORIES
 * 0	=>	IMAGE_DIRECTORY_ENTRY_EXPORT			=> Export Directory
 * 1	=>	IMAGE_DIRECTORY_ENTRY_IMPORT			=> Import Directory
 * 2	=>	IMAGE_DIRECTORY_ENTRY_RESOURCE			=> Resource Directory
 * 3	=>	IMAGE_DIRECTORY_ENTRY_EXCEPTION			=> Exception Directory
 * 4	=>	IMAGE_DIRECTORY_ENTRY_SECURITY			=> Security Directory
 * 5	=>	IMAGE_DIRECTORY_ENTRY_BASERELOC			=> Relocation Directory
 * 6	=>	IMAGE_DIRECTORY_ENTRY_DEBUG				=> Debug Directory
 * 7	=>	IMAGE_DIRECTORY_ENTRY_COPYRIGHT			=> Architecture Directory
 * 8	=>	IMAGE_DIRECTORY_ENTRY_GLOBALPTR			=> Reserved Directory
 * 9	=>	IMAGE_DIRECTORY_ENTRY_TLS				=> TLS Directory
 * 10	=>	IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG		=> Configuration Directory
 * 11	=>	IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT		=> Bound Import Directory
 * 12	=>	IMAGE_DIRECTORY_ENTRY_IAT				=> IAT
 * 13	=>	IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT		=> Delay IAT
 * 14	=>	IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR	=> .NET MetaData Directory
 * 15	=>	IMAGE_NUMBEROF_DIRECTORY_ENTRIES
*/
 
struct IMAGE_SECTION_HEADER{
	BYTE Name1[8];								// 8 bytes - name; may be empty
	union Misc{
		DWORD PhysicalAddress;
		DWORD VirtualSize;						// Actual in-memory section size
	}
	DWORD VirtualAddress;						// RVA for section
	DWORD SizeOfRawData;						// On-disk section size
	DWORD PointerToRawData;						// Offset from file beginning to the section's data
	DWORD PointerToRelocations;
	DWORD PointerToLinenumbers;
	WORD NumberOfRelocations;
	WORD NumberOfLinenumbers;
	DWORD Characteristics;						// Various parameters, ex. access rights
}
 
struct IMAGE_EXPORT_DIRECTORY{
	DWORD Characteristics;
	DWORD TimeDateStamp;
	WORD MajorVersion;
	WORD MinorVersion;
	DWORD nName;								// Internal Module Name
	DWORD nBase;								// Starting ordinal number
	DWORD NumberOfFunctions;					// Number of functions
	DWORD NumberOfNames;						// Number of exported symbols
	DWORD AddressOfFunctions;					// RVA of Export Address Table
	DWORD AddressOfNames;						// RVA of Export Name Table
	DWORD AddressOfNameOrdinals;				// RVA of Export Ordinal Table
}
 
struct IMAGE_IMPORT_DESCRIPTOR{
	union{
		DWORD Characteristics;
		DWORD OriginalFirstThunk;				// RVA for array of IMAGE_THUNK_DATA
	}
	DWORD TimeDateStamp;
	DWORD ForwarderChain;						// Not used now (outdated binding)
	DWORD Name1;								// Pointer to ASCII DLL name
	DWORD FirstThunk;							// RVA of IMAGE_THUNK_DATA array - duplicate of first member
}
 
struct IMAGE_THUNK_DATA32{						// either ordinal or pointer to IMAGE_IMPORT_BY_NAME; in FirstThunk array - virtual address of function
	union{
		DWORD ForwarderString;
		DWORD Function;
		DWORD Ordinal;
		DWORD AddressOfData;
	}
}

struct IMAGE_IMPORT_BY_NAME{
	WORD Hint;									// Index in EAT
	BYTE Name1;									// Name of imported function
}
 
struct IMAGE_BOUND_IMPORT_DESCRIPTOR{
	DWORD TimeDateStamp;						// must match TimeDateStamp of exporting DLL
	WORD OffsetModuleName;						// offset from the first IMAGE_BOUND_IMPORT_DESCRIPTOR to the name of DLL in NULL-terminated ASCII
	WORD NumberOfModuleForwarderRefs;			// # of IMAGE_BOUND_FORWARDER_REF, which follow this structure immediately; used for referenced import
}

struct IMAGE_BOUND_FORWARDER_REF{
	DWORD TimeDateStamp;
	WORD OffsetModuleName;
	WORD Reserved;
}

