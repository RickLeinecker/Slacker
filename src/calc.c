#include "slacker.h"

#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
char mountedDrive;
char driveRootPath[] = "C:\\";
char openDrivePath[] = "\\\\.\\C:";
char fileSystemFormat[MAX_PATH + 1];
#else
char drvName[200];
#endif

unsigned long sectPerClust, bytesPerSect, freeClusters, totalClusters, bytesPerCluster, fileSize;
unsigned long carrierClusters, carrierSectors, carrierFreeSectors, carrierFreeSpace, carrierFileSize;

long long rootByteOffset = 0;

void calcDeviceAndFileParams(char *carrierFilePath)
{
	/*
	  *
	  * The following section is window-specific code to
	  * query the disk parameters.
	*/
#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)

	/* Get drive letter and file format. */
	mountedDrive = toupper(carrierFilePath[0]);

	/* We use this string to tell the Windows API which drive we are interested in */
	driveRootPath[0] = mountedDrive;
	openDrivePath[4] = mountedDrive;

	/* Get drive parameters such as sectors per cluster and bytes per sector. */
	GetDiskFreeSpace(driveRootPath,
		&sectPerClust,
		&bytesPerSect,
		&freeClusters,
		&totalClusters);

	/* Calc bytes per cluster for later use. */
	bytesPerCluster = sectPerClust * bytesPerSect;

	GetVolumeInformation( driveRootPath, NULL, 0, NULL, NULL, NULL, fileSystemFormat, MAX_PATH + 1 );
#else
	struct statvfs vfs;
	struct stat s;
	char *devName;
	int fd;

	/* Here we get statvfs so we will know block size. */
	if( statvfs( carrierFilePath, &vfs ) < 0 )
	{
		showErrorAndQuit();
	}
	bytesPerCluster = vfs.f_bsize;

	/* Here we get stat so we will know device id so we can find
	   device name. */
	if( lstat( carrierFilePath, &s ) < 0 )
	{
		showErrorAndQuit();
	}

	/* Here we get the device name so that we can open and find number
	   of bytes per sector. */
	devName = blkid_devno_to_devname( s.st_dev );

	/* Open the device. We need to be in administrative mode
	   for this to work. */
	fd = open( devName, O_RDONLY );
	if( fd < 0 )
	{
		printf("Could not open device.\n" );
		showErrorAndQuit();
	}
	/* Now we get the number of bytes per sector */
	if( ioctl( fd, BLKSSZGET, &bytesPerSect ) < 0 )
	{
		showErrorAndQuit();
	}
	close( fd );

	strcpy( drvName, devName );

	sectPerClust = bytesPerCluster / bytesPerSect;
#endif

	/*
	  * Here we are going to open the carrier file
	  * in order to obtain its size. We need this
	  * so we can calculate the free slack space.
	*/
	FILE *fp;
	fp = fopen(carrierFilePath, "rb");
	if (fp == NULL)
	{
		/* Something went wrong so we bail out. */
		showError("Could not open the carrier file for calculation.", 1);
	}
	/* Call the platform-independent code that gets the file length. */
	carrierFileSize = getFileLength(fp);
	fclose(fp);

	/*
	  * Here we will calculate number of clusters, sectors, and the free slack space.
	  * We will use these later on.
	*/
	carrierClusters = (carrierFileSize + (bytesPerCluster - 1)) / bytesPerCluster;
	carrierSectors = carrierClusters * sectPerClust;
	unsigned long occupiedSectors = (carrierFileSize + (bytesPerSect - 1)) / bytesPerSect;
	carrierFreeSectors = carrierSectors - occupiedSectors;
	carrierFreeSpace = carrierFreeSectors * bytesPerSect;
}

void displayDeviceAndFileParams()
{
	printf("sectPerClust:%ld,bytesPerSect:%ld,freeClusters:%ld,totalClusters:%ld,bytesPerCluster:%ld\n", sectPerClust, bytesPerSect, freeClusters, totalClusters, bytesPerCluster);
	printf("carrierClusters:%ld,carrierSectors:%ld,carrierFreeSectors:%ld,carrierFreeSpace:%ld,carrierFileSize:%ld\n", carrierClusters, carrierSectors, carrierFreeSectors, carrierFreeSpace, carrierFileSize);
}
