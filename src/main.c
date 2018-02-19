/*
Slacker by Rick Leinecker
This build: February 2018

This program hides data into the slack of a disk file.

Usage:
Slacker embed carrierFile messageFile
Slacker extract carrierFile [messageFile] [display]
*/

#include "slacker.h"

int main(int argc, char *argv[])
{

	if (argc >= 4 && _stricmp(argv[1], "embed") == 0)
	{
		calcDeviceAndFileParams(argv[2]);
		//displayDeviceAndFileParams();
		loadMessageDataFromFile(argv[3]);
		checkSlackSpaceAvailability();
		getClusterMap(argv[2]);
		//showClusterMap();
		doEmbed();
	}
	else if (argc >= 3 && _stricmp(argv[1], "extract") == 0)
	{
		calcDeviceAndFileParams(argv[2]);
		//displayDeviceAndFileParams();
		getClusterMap(argv[2]);
		//showClusterMap();
		doExtract();
		dealWithExtractedMessage(argc, argv);
	}
	else
	{
		usage();
	}

	return 0;
}

extern int messageDataLength;
extern char *messageData;
extern unsigned long carrierFreeSpace;

void checkSlackSpaceAvailability()
{
	if (messageDataLength > (int)carrierFreeSpace)
	{
		char tmp[500];
		sprintf(tmp, "You need %d bytes of slack space for the message but only have %d bytes\n",
			messageDataLength, (int)carrierFreeSpace);
		showError(tmp, 1);
	}
}

void dealWithExtractedMessage(int argc, char *argv[])
{

	if ((argc >= 4 && _stricmp(argv[3], "DISPLAY") == 0) ||
		(argc >= 5 && _stricmp(argv[4], "DISPLAY") == 0))
	{
		printf("%s\n", messageData);
	}

	if ((argc >= 4 && _stricmp(argv[3], "DISPLAY") != 0))
	{
		saveMessage(argv[3]);
	}
	else if ((argc >= 5 && _stricmp(argv[4], "DISPLAY") != 0))
	{
		saveMessage(argv[4]);
	}

}
