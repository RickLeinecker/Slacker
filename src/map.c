
#include "slacker.h"
#include "Structs.h"

int VCN;
long long *clusterMap = NULL;
extern char fileSystemFormat[];
extern char openDrivePath[];

extern unsigned long sectPerClust, bytesPerSect, freeClusters, totalClusters, bytesPerCluster;
extern unsigned long carrierClusters, carrierSectors, carrierFreeSectors, carrierFreeSpace, carrierFileSize;

extern long long rootByteOffset;

#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
#else
extern char drvName[];
#endif

void freeClusterMap()
{
	if (clusterMap != NULL)
	{
		free(clusterMap);
	}
	clusterMap = NULL;
}

void getClusterMap(char *carrierFilePath)
{
	rootByteOffset = 0;

	freeClusterMap();

#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
	getWindowsClusterMap(carrierFilePath);
#else
	getLinuxClusterMap(carrierFilePath);
#endif
}

#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
void getWindowsClusterMap(char *carrierFilePath)
{
	/* Open the file. */
	HANDLE file = CreateFile(carrierFilePath, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		char tmp[500];
		getLastErrorAsString(tmp);
		showError(tmp, 1);
	}

	/* Set up our parameters for retrieving the extents. */
	const DWORD EXTENT_SIZE = 1024 * 1024 * 64;
	STARTING_VCN_INPUT_BUFFER inbuffer;
	memset(&inbuffer, 0, sizeof(STARTING_VCN_INPUT_BUFFER));
	inbuffer.StartingVcn.QuadPart = 0;
	RETRIEVAL_POINTERS_BUFFER *outbuffer = (RETRIEVAL_POINTERS_BUFFER *)malloc(EXTENT_SIZE);
	memset(outbuffer, 0, EXTENT_SIZE);

	/* Call the Windows API function to get the extents. */
	DWORD bytesRead;
	BOOL ret = DeviceIoControl(file, FSCTL_GET_RETRIEVAL_POINTERS,
		&inbuffer, sizeof(STARTING_VCN_INPUT_BUFFER),
		outbuffer, EXTENT_SIZE,
		&bytesRead, NULL);

	/* Close the file handle. This is essential. */
	CloseHandle(file);

	if (!ret)
	{
		free(outbuffer);
		showError("Could not get extents via the Windows API", 1);
	}

	/* This buffer will contain the cluster map. */
	int bufferLen = 100000 / sizeof(long long);
	clusterMap = (long long *)malloc(100000);
	memset(clusterMap, 0, 100000);

	// We map virtual cluster number (reference is start of file) to logical cluster number (reference is start of volume)
	VCN = 0;
	for (int extent = 0; extent < (int)outbuffer->ExtentCount; extent++)
	{
		/* Need the number of clusters in this extent. */
		int numClustersInExtent = (int)( outbuffer->Extents[extent].NextVcn.QuadPart - outbuffer->StartingVcn.QuadPart );

		/* Loop through the clusters in this extent. */
		for (int cluster = 0; cluster < numClustersInExtent; cluster++)
		{
			/* Make sure we have enough buffer size. */
			if (VCN < bufferLen)
			{
				/* Store the value. */
				
				long long tmp = (long long)outbuffer->Extents[extent].Lcn.QuadPart;
				
				clusterMap[VCN++] = tmp + (long long)cluster;//(long long)outbuffer->Extents[extent].Lcn.QuadPart + (long long)cluster;
			}
			else
			{
				showError("Insufficient buffer.", 1);
			}
		}
	}

	free( outbuffer );

	if( _strnicmp(fileSystemFormat, "NTFS", 4 )!=0)
	{
		adjustForFAT();
	}
}

void adjustForFAT()
{
	// Open the drive.
	HANDLE device = CreateFile(openDrivePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (device == INVALID_HANDLE_VALUE)
	{
		char tmp[500];
		getLastErrorAsString(tmp);
		showError(tmp, 1);
	}

	/* Read the boot sector. */
	BOOTSECTOR bs;
	DWORD bytesRead;
	if (!ReadFile(device, &bs, bytesPerSect, &bytesRead, NULL))
	{
		CloseHandle(device);
		char tmp[500];
		getLastErrorAsString(tmp);
		showError(tmp, 1);
	}

	CloseHandle(device);

	/* Do the calculations. */
	char *tmp = (char *)&bs;
	memcpy( &bs.bpb.reservedSectorCount, &tmp[11+3], sizeof(short));
	memcpy( &bs.ebpb.logicalSectorsPerFat, &tmp[11+25], sizeof(long));

	rootByteOffset = (bs.bpb.reservedSectorCount + bs.ebpb.logicalSectorsPerFat * 2) * bytesPerSect;
}
#else
void getLinuxClusterMap(char *carrierFilePath)
{
	int i, fd, block;
	BOOTSECTOR bs;

	fd = open( carrierFilePath, O_RDONLY );
	VCN = ( carrierFileSize + bytesPerCluster - 1 ) / bytesPerCluster;
	clusterMap = calloc((sizeof(long long))*VCN,1);
	for( i=0; i<VCN; i++ )
	{
		block = i;
		ioctl( fd, FIBMAP, &block );
		clusterMap[i] = block;
	}
	close( fd );

	/* Adjust for FAT32 */
	fd = open( drvName, O_RDWR );
	if( fd < 0 )
	{
		showErrorAndQuit();
	}
	read( fd, &bs, sizeof(BOOTSECTOR));
	close( fd );

	char *name = (char *)&bs;
	name += (29+29+24);
	if( name[0] != 'F' || name[1] != 'A' || name[2] != 'T' )
	{
		return;
	}

	char *tmp = (char *)&bs;
	memcpy( &bs.bpb.reservedSectorCount, &tmp[11+3], sizeof(short));
	memcpy( &bs.ebpb.logicalSectorsPerFat, &tmp[11+25], sizeof(long));

	rootByteOffset = (bs.bpb.reservedSectorCount + bs.ebpb.logicalSectorsPerFat * 2) * bytesPerSect;
}
#endif

void showClusterMap()
{
	int i;
	printf("%d clusters in map.\n", VCN);
	for (i = 0; i < VCN; i++)
	{
#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
		printf("%d:%I64d\n", i, clusterMap[i]);
#else
		printf("%d:%lld\n", i, clusterMap[i]);
#endif
	}
}
