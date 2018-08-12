TARGET := DiscImageCreator
INCFLAGS := -I. -I_external -I_linux
CFLAGS := -include _linux/defineForLinux.h
CXXFLAGS := $(CFLAGS) -std=c++11

ifneq ($(SANITIZER),)
   CFLAGS   := -fsanitize=$(SANITIZER) $(CFLAGS)
   CXXFLAGS := -fsanitize=$(SANITIZER) $(CXXFLAGS)
   LDFLAGS  := -fsanitize=$(SANITIZER) $(LDFLAGS)
endif

ifeq ($(DEBUG), 1)
	CFLAGS += -O0 -g
	CXXFLAGS += -O0 -g
else
	CFLAGS += -O2
	CXXFLAGS += -O2
endif

SOURCES_CXX := \
  calcHash.o \
  check.o \
  convert.o \
  DiscImageCreator.o \
  execIoctl.o \
  execScsiCmd.o \
  execScsiCmdforCD.o \
  execScsiCmdforCDCheck.o \
  execScsiCmdforDVD.o \
  execScsiCmdforFileSystem.o \
  fix.o \
  get.o \
  init.o \
  output.o \
  outputIoctlLog.o \
  outputScsiCmdLog.o \
  outputScsiCmdLogforCD.o \
  outputScsiCmdLogforDVD.o \
  set.o \
  _external/crc16ccitt.o \
  _external/crc32.o \
  _external/md5c.o \
  _external/prngcd.o \
  _external/sha1.o \
  _linux/defineForLinux.o

OBJECTS := $(SOURCES_C:.c=.o) $(SOURCES_CXX:.cpp=.o)

all: $(TARGET)
$(TARGET): buildDateTime $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCFLAGS)

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(INCFLAGS)

clean-objs:
	rm -f $(OBJECTS)

clean:
	rm -f $(OBJECTS)
	rm -f $(TARGET)
	rm -f buildDateTime.h

buildDateTime:
	echo "#pragma once" >buildDateTime.h
	echo >>buildDateTime.h
	echo -ne "#define BUILD_DATE \"" >>buildDateTime.h
	date +%Y%m%d | tr -d '\n' >>buildDateTime.h
	echo -ne "\"\n" >>buildDateTime.h
	echo -ne "#define BUILD_TIME \"" >>buildDateTime.h
	date +%H%M%S | tr -d '\n' >>buildDateTime.h
	echo -ne "\"\n" >>buildDateTime.h

.PHONY: clean clean-objs buildDateTime
