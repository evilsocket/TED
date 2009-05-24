CC=g++
CFLAGS= -w -O3 -funroll-loops -fno-rtti -fomit-frame-pointer -ffast-math -fno-stack-protector -ffunction-sections `pkg-config --cflags gtk+-2.0` `pkg-config --cflags glib-2.0`
LFLAGS=-lpthread -lnotify
SRC=src
INC=include

all:
	$(CC) -c $(CFLAGS) -I$(INC) $(SRC)/ted_fs.cpp
	$(CC) -c $(CFLAGS) -I$(INC) $(SRC)/ted_common.cpp
	$(CC) -c $(CFLAGS) -I$(INC) $(SRC)/ted_tcpmon.cpp
	$(CC) $(CFLAGS) $(LFLAGS) -I$(INC) $(SRC)/ted_main.cpp *.o -o ted
	
clean:
	rm *.o ted
