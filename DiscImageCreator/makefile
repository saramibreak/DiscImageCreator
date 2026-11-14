TARGET := DiscImageCreator
ifeq ($(shell uname),Linux)
  INCFLAGS := -I. -I_external -I_linux
  LDLIBS := -larchive -lz -lcrypto -lssl
  CRYPTO_BACKEND := _external/ps3auth/crypto_backend_openssl.o
else ifeq ($(shell uname),Darwin)
  PKG_CONFIG ?= pkg-config
  PC_DEPS := libarchive zlib
  INCFLAGS := -I. -I_external -I_linux \
              $(shell $(PKG_CONFIG) --cflags $(PC_DEPS))
  LDFLAGS := -framework CoreFoundation -framework IOKit \
             $(shell $(PKG_CONFIG) --libs-only-L     $(PC_DEPS)) \
             $(shell $(PKG_CONFIG) --libs-only-other $(PC_DEPS))
  LDLIBS := $(shell $(PKG_CONFIG) --libs-only-l     $(PC_DEPS))
  CRYPTO_BACKEND := _external/ps3auth/crypto_backend_commoncrypto.o
endif
CFLAGS := 
CXXFLAGS := -include _linux/defineForLinux.h -std=c++11

ifneq ($(SANITIZER),)
  CFLAGS   += -fsanitize=$(SANITIZER) $(CFLAGS)
  CXXFLAGS += -fsanitize=$(SANITIZER) $(CXXFLAGS)
  LDFLAGS  += -fsanitize=$(SANITIZER) $(LDFLAGS)
endif

ifeq ($(DEBUG), 1)
  CFLAGS += -O0 -g
  CXXFLAGS += -O0 -g
else
  CFLAGS += -O2 -Wall -Wextra -Wno-unknown-pragmas -Wcast-align -Wcast-qual -Wdisabled-optimization -Wdouble-promotion -Wfloat-equal -Wformat=2 -Wformat-signedness -Winit-self -Winline -Winvalid-pch -Wlogical-op -Wmissing-include-dirs -Wmultichar -Woverlength-strings -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wstack-protector -Wstrict-aliasing=2 -Wswitch-default -Wswitch-enum -Wtrampolines -Wvariadic-macros -Wvector-operation-performance -Wwrite-strings -Wunused-macros
  ifeq ($(shell uname),Linux)
    CXXFLAGS += -O2 -Wall -Wextra -Wno-unknown-pragmas -Wcast-align -Wcast-qual -Wconditionally-supported -Wdisabled-optimization -Wdouble-promotion -Wfloat-equal -Wformat=2 -Wformat-signedness -Winit-self -Winline -Winvalid-pch -Wlogical-op -Wmissing-include-dirs -Wmultichar -Wnoexcept -Woverlength-strings -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wstack-protector -Wstrict-aliasing=2 -Wstrict-null-sentinel -Wswitch-default -Wswitch-enum -Wtrampolines -Wvariadic-macros -Wvector-operation-performance -Wwrite-strings -Wunused-macros
  else ifeq ($(shell uname),Darwin)
    CXXFLAGS += -O2 -Wall -Wextra -Wno-unknown-pragmas -Wcast-align -Wcast-qual -Wdisabled-optimization -Wdouble-promotion -Wfloat-equal -Wformat=2 -Winit-self -Winline -Winvalid-pch -Wmissing-include-dirs -Wmultichar -Woverlength-strings -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wstack-protector -Wstrict-aliasing=2 -Wswitch-default -Wswitch-enum -Wvariadic-macros -Wwrite-strings -Wunused-macros
  endif
endif

SOURCES_C := \
  _external/abgx360.o \
  _external/aes.o \
  _external/aesni.o \
  _external/crc16ccitt.o \
  _external/crc32.o \
  _external/md5c.o \
  _external/platform_util.o \
  _external/rijndael-alg-fst.o \
  _external/sha1.o \
  _external/sha224-256.o \
  _external/sha384-512.o \
  _external/xxhash.o \
  _external/libunshield/component.o \
  _external/libunshield/converter.o \
  _external/libunshield/directory.o \
  _external/libunshield/file.o \
  _external/libunshield/file_group.o \
  _external/libunshield/helper.o \
  _external/libunshield/libunshield.o \
  _external/libunshield/log.o \
  _external/ps3auth/auth_service.o \
  $(CRYPTO_BACKEND) \
  _external/ps3auth/data_unwrap.o \
  _external/ps3auth/drive_proto.o \
  _external/ps3auth/odd_ctx.o \
  _external/ps3auth/odd_tables.o \
  _external/ps3auth/session_keys.o \

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
  execTapeCmd.o \
  fix.o \
  get.o \
  init.o \
  output.o \
  outputFileSystem.o \
  outputIoctlLog.o \
  outputScsiCmdLog.o \
  outputScsiCmdLogforCD.o \
  outputScsiCmdLogforDVD.o \
  set.o \
  xml.o \
  _external/ECCEDC.o \
  _external/NonStandardFunction.o \
  _external/prngcd.o \
  _external/reedSolomonGF64.o \
  _external/tinyxml2.o \
  _external/ps3auth/drive_transport_execscsi.o \
  _linux/defineForLinux.o

OBJECTS := $(SOURCES_C:.c=.o) $(SOURCES_CXX:.cpp=.o)

all: $(TARGET)
$(TARGET): buildDateTime $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS) $(LDLIBS)

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

ifeq ($(PREFIX),)
  PREFIX := /usr/local
endif

install:
ifeq ($(shell uname),Linux)
	install -Dm 0644 ../Release_ANSI/default.dat "$(DESTDIR)$(PREFIX)/share/$(TARGET)/default.dat"
	install -Dm 0644 ../Release_ANSI/driveOffset.txt "$(DESTDIR)$(PREFIX)/share/$(TARGET)/driveOffset.txt"
	install -Dm 0755 "$(TARGET)" "$(DESTDIR)$(PREFIX)/bin/$(TARGET)"
else ifeq ($(shell uname),Darwin)
	install -m 0644 ../Release_ANSI/default.dat "$(DESTDIR)$(PREFIX)/share/$(TARGET)/default.dat"
	install -m 0644 ../Release_ANSI/driveOffset.txt "$(DESTDIR)$(PREFIX)/share/$(TARGET)/driveOffset.txt"
	install -m 0755 "$(TARGET)" "$(DESTDIR)$(PREFIX)/bin/$(TARGET)"
endif

.PHONY: clean clean-objs buildDateTime
