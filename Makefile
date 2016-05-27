PREFIX = /usr/local

WORKDIR = `pwd`

CC = gcc
AR = ar
LD = g++

INC = 
CFLAGS = 
RESINC = 
LIBDIR = 
LIB = -lwiringPi
LDFLAGS = 

INC_DEBUG = $(INC)
CFLAGS_DEBUG = $(CFLAGS) -Wall -g
RESINC_DEBUG = $(RESINC)
RCFLAGS_DEBUG = $(RCFLAGS)
LIBDIR_DEBUG = $(LIBDIR)
LIB_DEBUG = $(LIB)
LDFLAGS_DEBUG = $(LDFLAGS)
OBJDIR_DEBUG = obj/Debug
DEP_DEBUG = 
OUT_DEBUG = bin/Debug/libPCD8544.a

INC_RELEASE = $(INC)
CFLAGS_RELEASE = $(CFLAGS) -Wall -O2
RESINC_RELEASE = $(RESINC)
RCFLAGS_RELEASE = $(RCFLAGS)
LIBDIR_RELEASE = $(LIBDIR)
LIB_RELEASE = $(LIB)
LDFLAGS_RELEASE = $(LDFLAGS) -s
OBJDIR_RELEASE = obj/Release
DEP_RELEASE = 
OUT_RELEASE = bin/Release/libPCD8544.a

LIB_FNAME = libPCD8544.a

OBJ_DEBUG = $(OBJDIR_DEBUG)/src/PCD8544.o

OBJ_RELEASE = $(OBJDIR_RELEASE)/src/PCD8544.o

all: debug release

clean: clean_debug clean_release

before_debug: 
	test -d bin/Debug || mkdir -p bin/Debug
	test -d $(OBJDIR_DEBUG)/src || mkdir -p $(OBJDIR_DEBUG)/src

after_debug: 

debug: before_debug out_debug after_debug

out_debug: before_debug $(OBJ_DEBUG) $(DEP_DEBUG)
	$(AR) rcs $(OUT_DEBUG) $(OBJ_DEBUG)

$(OBJDIR_DEBUG)/src/PCD8544.o: src/PCD8544.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c src/PCD8544.c $(LIB_DEBUG) -o $(OBJDIR_DEBUG)/src/PCD8544.o

clean_debug: 
	rm -f $(OBJ_DEBUG) $(OUT_DEBUG)
	rm -rf bin/Debug
	rm -rf $(OBJDIR_DEBUG)/src

before_release: 
	test -d bin/Release || mkdir -p bin/Release
	test -d $(OBJDIR_RELEASE)/src || mkdir -p $(OBJDIR_RELEASE)/src

after_release: 

release: before_release out_release after_release

out_release: before_release $(OBJ_RELEASE) $(DEP_RELEASE)
	$(AR) rcs $(OUT_RELEASE) $(OBJ_RELEASE)

$(OBJDIR_RELEASE)/src/PCD8544.o: src/PCD8544.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c src/PCD8544.c $(LIB_DEBUG) -o $(OBJDIR_RELEASE)/src/PCD8544.o

clean_release: 
	rm -f $(OBJ_RELEASE) $(OUT_RELEASE)
	rm -rf bin/Release
	rm -rf $(OBJDIR_RELEASE)/src

install:
	mkdir -p $(PREFIX)/lib
	mkdir -p $(PREFIX)/include
	cp $(OUT_RELEASE) $(PREFIX)/lib/$(LIB_FNAME)
	cp include/PCD8544.h $(PREFIX)/include/

uninstall:
	rm -f $(PREFIX)/lib/$(LIB_FNAME)
	rm -f $(PREFIX)/include/PCD8544.h

.PHONY: before_debug after_debug clean_debug before_release after_release clean_release install

