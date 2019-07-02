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
	CFLAGS += -O2 -Wall -Wextra -Wno-unknown-pragmas
	CXXFLAGS += -O2 -Wall -Wextra -Wno-unknown-pragmas -Waggregate-return -Wcast-align -Wcast-qual -Wconditionally-supported -Wdisabled-optimization -Wdouble-promotion -Wfloat-equal -Wformat=2 -Wformat-signedness -Winit-self -Winline -Winvalid-pch -Wlogical-op -Wmissing-include-dirs -Wmultichar -Wnoexcept -Woverlength-strings -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wstack-protector -Wstrict-aliasing=2 -Wstrict-null-sentinel -Wswitch-default -Wswitch-enum -Wtrampolines -Wvariadic-macros -Wvector-operation-performance -Wwrite-strings -Wunused-macros
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
  _external/abgx360.o \
  _external/aesni.o \
  _external/aes.o \
  _external/crc16ccitt.o \
  _external/crc32.o \
  _external/md5c.o \
  _external/platform_util.o \
  _external/prngcd.o \
  _external/rijndael-alg-fst.o \
  _external/tinyxml2.o \
  _external/sha1.o \
  xml.o \
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
	$(shell /usr/bin/env echo "#pragma once" >buildDateTime.h)
	$(shell /usr/bin/env echo >>buildDateTime.h)
	$(shell /usr/bin/env echo -n "#define BUILD_DATE \"" >>buildDateTime.h)
	$(shell date +%Y%m%d | tr -d '\n' >>buildDateTime.h)
	$(shell /usr/bin/env echo "\"" >>buildDateTime.h)
	$(shell /usr/bin/env echo -n "#define BUILD_TIME \"" >>buildDateTime.h)
	$(shell date +%H%M%S | tr -d '\n' >>buildDateTime.h)
	$(shell /usr/bin/env echo "\"" >>buildDateTime.h)

.PHONY: clean clean-objs buildDateTime
