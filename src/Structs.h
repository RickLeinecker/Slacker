#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#pragma pack(1)

typedef struct
{
	unsigned short charsPerSector;			//13
	unsigned char sectorsPerCluster;			//14
	unsigned short reservedSectorCount;		//16
	unsigned char fatCount;					//17
	unsigned short rootDirEntryCount;			//19
	unsigned short logicalSectors;			//21
	unsigned char mediaType;					//22
	unsigned short sectorsPerFat;				//24
	unsigned short sectorsPerTrack;			//26
	unsigned short numHeads;					//28
	unsigned long hiddenSectors;			//32
	unsigned long logicalSectors2;			//36
}BPB;

typedef struct
{
	unsigned long logicalSectorsPerFat;		// 4 (40)
	unsigned short driveDescription;			// 6 (42)
	unsigned short version;					// 8 (44)
	unsigned long clusterNumberOfRootDirectory;// 12 (48)
	unsigned short logicalSectorNumberOfFSInformationSector;//14 (50)
	unsigned short firstLogicalSectorNumberOfFatCopy;//16 (52)
	char reserved[12];				//28 (64)
	unsigned char physicalDriveNumber;		//29 (65)
	unsigned char filler;					//30 (66)
	unsigned char extendedBootSignature;		//31 (67)
	unsigned long volumeID;					//35 (71)
	char volumeLabel[11];			//46 (82)
	char fileSytemType[8];			//54 (90)
}EXTENDED_BPB;

typedef struct
{
	unsigned char jmpToBootCode[3];
	char oemName[8];
	BPB bpb;
	EXTENDED_BPB ebpb;

	unsigned char bootCode[414];

	unsigned long serialNumber;
	unsigned short reserved;

	unsigned char bootSectorSig0;	// Should be 0x55	// 1
	unsigned char bootSectorSig1;	// Should be 0xaa	// 1
} BOOTSECTOR;

#pragma pack()

#endif
