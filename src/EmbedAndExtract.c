
#include "slacker.h"

#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
extern char mountedDrive;
extern char driveRootPath[];
extern char openDrivePath[];
extern char fileSystemFormat[];
#else
extern char drvName[];
#endif

extern int VCN;
extern long long *clusterMap;

extern unsigned long sectPerClust, bytesPerSect, freeClusters, totalClusters, bytesPerCluster;
extern unsigned long carrierClusters, carrierSectors, carrierFreeSectors, carrierFreeSpace, carrierFileSize;

extern char *messageData;
extern int messageDataLength;

extern long long rootByteOffset;

void doEmbed()
{
#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
	doWindowsEmbed();
#else
	doLinuxEmbed();
#endif
}

long long getSeekTo()
{
	return clusterMap[VCN - 1] * (long long)bytesPerSect * (long long)sectPerClust + ((long long)sectPerClust - (long long)carrierFreeSectors) * (long long)bytesPerSect + rootByteOffset;
}

#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)

void seekToStart(HANDLE h)
{
	/*                 |-----------------------last cluster in the map-------------------------|  |-----------Used sectors in last cluster---------------|     --------           */
	long long seekTo = getSeekTo();

	/* Now seekTo should be the byte offset of the first open sector in the last cluster. */

	/* Seek to the first unused sector. */
	LARGE_INTEGER there, dest;
	dest.QuadPart = seekTo;
	SetFilePointerEx(h, dest, &there, FILE_BEGIN);
}

int roundUp(int num)
{
	while ((num%bytesPerSect) != 0)
	{
		num++;
	}
	return(num);
}

void doWindowsEmbed()
{
	/* Open the device. */
	HANDLE writeDevice = CreateFile(openDrivePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (writeDevice == INVALID_HANDLE_VALUE)
	{
		char tmp[500];
		getLastErrorAsString(tmp);
		showError(tmp, 1);
	}

	seekToStart(writeDevice);

	DWORD dwBytesReturned = 0;
	/* Here we need to lock the volume or we will get an access denial. */
	if (!DeviceIoControl(writeDevice, FSCTL_LOCK_VOLUME,
		NULL, 0, NULL, 0, &dwBytesReturned, NULL))
	{
		CloseHandle(writeDevice);
		char tmp[500];
		getLastErrorAsString(tmp);
		showError(tmp, 1);
	}

	/* Write the sector data. */
	DWORD bytes;
	/* A little detail is the the data written must be a multiple of the bytesPerSect */
	if (!WriteFile(writeDevice, messageData, roundUp( messageDataLength ), &bytes, NULL))
	{
		CloseHandle(writeDevice);
		char tmp[500];
		getLastErrorAsString(tmp);
		showError(tmp, 1);
	}

	/* Unlock the device. */
	if (!DeviceIoControl(writeDevice, FSCTL_UNLOCK_VOLUME,
		NULL, 0, NULL, 0, &dwBytesReturned, NULL))
	{
		CloseHandle(writeDevice);
		char tmp[500];
		getLastErrorAsString(tmp);
		showError(tmp, 1);
	}

	/* Important to close the handle. */
	CloseHandle(writeDevice);
}
#else
void doLinuxEmbed()
{
	int fd;

	fd = open( drvName, O_RDWR );
	if( fd < 0 )
	{
		showErrorAndQuit();
	}

	long long seekTo = getSeekTo();
	if( lseek( fd, seekTo, SEEK_SET ) < 0 )
	{
		showErrorAndQuit();
	}

	write( fd, messageData, messageDataLength );

	close( fd );
}
#endif

void doExtract()
{
#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
	doWindowsExtract();
#else
	doLinuxExtract();
#endif
}

#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
void doWindowsExtract()
{
	HANDLE readDevice = CreateFile(openDrivePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (readDevice == INVALID_HANDLE_VALUE)
	{
		char tmp[500];
		getLastErrorAsString(tmp);
		showError(tmp, 1);
	}

	seekToStart(readDevice);

	messageData = (char *)calloc(carrierFreeSectors * bytesPerSect, sizeof(char));
	if (messageData == NULL)
	{
		showError("Could not allocate message data buffer.", 1);
	}

	DWORD bytes;
	/* Read the sectors. */
	if (!ReadFile(readDevice, messageData, carrierFreeSectors * bytesPerSect, &bytes, NULL))
	{
		CloseHandle(readDevice);
		char tmp[500];
		getLastErrorAsString(tmp);
		showError(tmp, 1);
	}

	char tmp[10];
	memcpy(tmp, messageData, 5);
	tmp[5] = 0;
	messageDataLength = atoi(tmp);

	memmove(messageData, &messageData[5], carrierFreeSectors * bytesPerSect - 5);

	CloseHandle(readDevice);
}
#else
void doLinuxExtract()
{
	int fd = open(drvName, O_RDONLY);
	if( fd < 0 )
	{
		showErrorAndQuit();
	}

	long long seekTo = getSeekTo();
	if( lseek( fd, seekTo, SEEK_SET ) < 0 )
	{
		showErrorAndQuit();
	}

	messageData = (char *)calloc(carrierFreeSectors * bytesPerSect, sizeof(char));
	if (messageData == NULL)
	{
		showError("Could not allocate message data buffer.", 1);
	}

	if (read(fd, messageData, carrierFreeSectors * bytesPerSect)<0)
	{
		showErrorAndQuit();
	}

	close( fd );

	char tmp[10];
	memcpy(tmp, messageData, 5);
	tmp[5] = 0;
	messageDataLength = atoi(tmp);
	memmove(messageData, &messageData[5], carrierFreeSectors * bytesPerSect - 5);

}
#endif

