#include "slacker.h"

extern unsigned long sectPerClust, bytesPerSect, freeClusters, totalClusters, bytesPerCluster;
extern unsigned long carrierClusters, carrierSectors, carrierFreeSectors, carrierFreeSpace, carrierFileSize;

char *messageData = NULL;
int messageDataLength;

void freeMessageData()
{
	if (messageData != NULL)
	{
		free(messageData);
	}
	messageData = NULL;
	messageDataLength = 0;
}

/*
** This function loads the message data into an allocated buffer.
** It also sets the first characters to indicate the size of the message data.
*/
void loadMessageDataFromFile(char *filePath)
{
	FILE *fp;

	freeMessageData();

	/* Attempt to open the message data file. */
	fp = fopen(filePath, "rb");
	if (fp == NULL)
	{
		/* Bail out since the file did not open. */
		showError("Could not open the message file.", 1);
	}

	/* Get the file size. */
	messageDataLength = getFileLength(fp);

	/* Attempt to allocate the message data buffer.
	We need to also allocate additional space for the message size. */
	messageData = (char *)calloc(messageDataLength+5+(bytesPerSect), sizeof(char));
	if (messageData == NULL)
	{
		/* Buffer did not allocate so we close
		the file and bail out. */
		fclose(fp);

		showError("Could not allocate message data buffer.", 1);
	}

	sprintf(messageData, "%05d", messageDataLength);

	/* Read the message data into the buffer. We skip the first
	several bytes of the return buffer where the size data
	will reside. */
	fread(&messageData[5], sizeof(char), messageDataLength, fp);
	fclose(fp);

	messageDataLength += 5;
}

/*
** This function returns the file length for an open file.
** Could have used _filelength(_fileno(fp)) but that is not portable.
*/
int getFileLength(FILE *fp)
{
	int fileLength, curr;
	/* Seek to the end. */
	curr = fseek(fp, 0, SEEK_END);
	/* Record where we are. */
	fileLength = ftell(fp);
	/* Back to where we started. */
	fseek(fp, curr, SEEK_SET);

	/* Return the value. */
	return(fileLength);
}

/*
Show the program usage.
*/
void usage()
{
	printf("Slacker by Rick Leinecker\nUsage:\n");
	printf("    Slacker embed fully-qualified-carrierFilePath messageFile\n");
	printf("    Slacker extract fully-qualified-carrierFilePath [messageFile] [display]\n");
}

void showError(char *errorMessage, int quitFlag)
{
	printf("%s\n", errorMessage);
	if (quitFlag != 0)
	{
		exit(0);
	}
}

void showErrorAndQuit()
{
	char error[200];
	getLastErrorAsString( error );
	showError( error, 1 );
}

void getLastErrorAsString( char *errorMessageString )
{
	errorMessageString[0] = 0;

#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
	//Get the error message, if any.
	DWORD errorMessageID = GetLastError();
	if (errorMessageID == 0)
	{
		return;
	}

	LPSTR messageBuffer = NULL;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	strcpy(errorMessageString, messageBuffer);

	//Free the buffer.
	LocalFree(messageBuffer);
#else
	strcpy( errorMessageString, strerror(errno) );
#endif
}

void saveMessage(char *saveFileName)
{
	FILE *fp;

	fp = fopen(saveFileName, "wb");
	if (fp == NULL)
	{
		showError("Could not create the saved file.", 0);
	}
	else
	{
		fwrite(messageData, messageDataLength, sizeof(char), fp);
		fclose(fp);
	}

}