#ifndef __SLACKER_H__
#define __SLACKER_H__

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <ctype.h>
#include <memory.h>

#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
#include <Windows.h>
#include <WinIoCtl.h>
#else
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <linux/fs.h>
#include <blkid/blkid.h>
#define _stricmp strcmp
#endif

/* In Utils.c */
void usage();
int getFileLength(FILE *fp);
void loadMessageDataFromFile(char *filePath);
void showError(char *errorMessage, int quitFlag);
void getLastErrorAsString(char *errorMessageString);
void saveMessage(char *saveFileName);
void showErrorAndQuit();

/* In Calc.c */
void calcDeviceAndFileParams(char *carrierFile);
void displayDeviceAndFileParams();

/* In main.c */
void checkSlackSpaceAvailability();
void dealWithExtractedMessage(int argc, char *argv[]);

/* In map.c */
#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
void getWindowsClusterMap(char *carrierFilePath);
void adjustForFAT();
#else
void getLinuxClusterMap(char *carrierFilePath);
#endif
void getClusterMap(char *carrierFilePath);
void showClusterMap();

/* In EmbedAndExtract.c */
void doEmbed();
#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
void doWindowsEmbed();
void seekToStart(HANDLE h);
#else
void doLinuxEmbed();
#endif
void doExtract();
#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
void doWindowsExtract();
#else
void doLinuxExtract();
#endif

#endif
