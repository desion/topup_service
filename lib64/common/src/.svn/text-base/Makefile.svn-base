TARGET = libcommon.a
all : $(TARGET)

TOP_SRCDIR	:=	$(shell cd ../../..;pwd)
include $(TOP_SRCDIR)/make/common.mk

CFLAGS	= -g -fPIC
CPPFLAGS= -I../include -I$(TOP_SRCDIR)/include

LIBS := -pthread

OBJS1 = dllcall.o mkdirs.o searchfile.o selog.o thread.o profile.o \
	urlformat.o CRC.o termid.o CGIParser.o conf.o slog.o phrasehash.o \
	common.o ul_func.o ul_file.o ul_conf.o

libcommon.a : $(OBJS1)
	$(AR) -r $@ $(OBJS1)
	$(CP) $@ ../lib

clean:
	$(RM) $(TARGET) *.o *.d
