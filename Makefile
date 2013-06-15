# This program is distributed in the hope that it will be useful,
CC = gcc
CXX = g++

OUTPUT = .

BIN_PATH 	= $(OUTPUT)/bin/
LIB_PATH 	= $(OUTPUT)/lib/
PREFIX=./
OBJDIR=./obj/
SRC=./

TEST_BARRIER=$(BIN_PATH)barrier
TEST_TASKDIST=$(BIN_PATH)taskdist
TEST_TASKHANDLE=$(BIN_PATH)taskhandle
TEST_CONFIGMGR=$(BIN_PATH)configmgr
ALL_TEST = $(TEST_BARRIER) $(TEST_TASKDIST) $(TEST_TASKHANDLE) $(TEST_CONFIGMGR)
#PROGRAM =  $(BIN_PATH)$(TEST01) $(BIN_PATH)$(TEST02) $(BIN_PATH)$(TEST03)
PROGRAM = $(BIN_PATH)test

MYREVISION		= 0
MYMAJORVERSION  = 0
RELEASE			= 0

###################################################################
#include
###################################################################
INCLUDES 	+= -I./include

###################################################################
#lib
###################################################################
LDFLAGS		+= -L./lib
LIBS		+= -lzookeeper_mt -lpthread -lm

##################################################################
#SRC
##################################################################
SRC_BARRIERS 		+= barrier.c  
SRC_TASKDIST		+= taskdist.c
SRC_TASKHANDLE		+= taskhandle.c
SRC_CONFIGMGR		+= configmgr.c

##################################################################
#OBJ
##################################################################
#OBJECTS_BARRIERS 	+= $(OBJDIR)barrier.o
#OBJECTS_TASKDIST 	+= $(OBJDIR)taskdist.o
#OBJECTS_TASKHANDLE 	+= $(OBJDIR)taskhandle.o
OBJECTS 	+= $(OBJDIR)barrier.o
OBJECTS 	+= $(OBJDIR)taskdist.o
OBJECTS 	+= $(OBJDIR)taskhandle.o
OBJECTS		+= $(OBJDIR)configmgr.o

all: $(PROGRAM)



ifeq ($(RELEASE),1)
#	Release Version
CFLAGS		+= -o2 -Wall -Wunused -fPIC -g $(INCLUDES) -D__STDC_CONSTANT_MACROS -DTIXML_USE_STL 
else
#	Debug Version
CFLAGS		+= -Wall -Wunused -fPIC -g $(INCLUDES) -D__STDC_CONSTANT_MACROS -DTIXML_USE_STL 
endif

# The variable $@ has the value of the target. In this case $@ = psort
$(PROGRAM): $(BIN_PATH) $(LIB_PATH) $(OBJDIR) $(OBJECTS) $(ALL_TEST)
	

$(BIN_PATH):
	mkdir -p $(BIN_PATH)
$(LIB_PATH):
	mkdir -p $(LIB_PATH)
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)barrier.o:$(SRC_BARRIERS)
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJDIR)taskdist.o:$(SRC_TASKDIST)
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJDIR)taskhandle.o:$(SRC_TASKHANDLE)
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJDIR)configmgr.o:$(SRC_CONFIGMGR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_BARRIER):$(OBJDIR)barrier.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LIBS)
$(TEST_TASKDIST):$(OBJDIR)taskdist.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LIBS)
$(TEST_TASKHANDLE):$(OBJDIR)taskhandle.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LIBS)
$(TEST_CONFIGMGR):$(OBJDIR)configmgr.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LIBS)

depend: 
	makedepend ${SRCS}

clean:
	rm -rf $(OBJDIR)*.o 
	rm -rf $(ALL_TEST)

print:
	more Makefile $(SRCS) | enscript -2r -p listing.ps

# DO NOT DELETE
