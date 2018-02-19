# Modify this line to change the compiler (eg, for ic instead of gcc)
CC = gcc
WINCC = mingw32-gcc
#compiler flags 
CFLAGS = -g -Wall -lblkid

SRCS = src/*.c

EXEC = slacker

slacker_make = src/*.c

ifeq ($(OS), Windows_NT)
windows:
	mkdir bin
	$(WINCC) $(slacker_make) $(CFLAGS) -o bin/$(EXEC).exe
clean: 
	del bin 

else
linux:
	mkdir -p bin
	$(CC) $(slacker_make) $(CFLAGS) -o bin/$(EXEC).sh 
clean:
	rm -rf bin
endif





