USE_ISA_LIB ?= 0

PROGRAM=Crc32Test
LIBS=-lrt
HEADERS=Crc32.h
OBJECTS=Crc32.o Crc32Test.o
FLAGS=-O3 -Wall -pedantic -s
LINKFLAGS=-s
CPP=g++

ifneq ($(USE_ISA_LIB),0)
	CFLAGS += -DUSE_ISA_LIB
	LIBS += -lisal
endif

ARCH=$(shell uname -m)
CFLAGS += -DARCH=$(ARCH)

ifeq ($(ARCH), aarch64)
	CFLAGS += -march=armv8-a+crc
	LDFLAGS += -march=armv8-a+crc
endif

$(info ARCH $(ARCH))
$(info Using ISA-L lib $(USE_ISA_LIB))
$(info CFLAGS $(CFLAGS))
$(info LIBS  $(LIBS))

default: $(PROGRAM)
all: default

Crc32Test: $(OBJECTS)
	$(CPP) $(OBJECTS) $(LIBS) -o $(PROGRAM)

%.o: %.cpp $(HEADERS)
	$(CPP) $(LINKFLAGS) -c $< -o $@  $(CFLAGS)

clean:
	-rm -f $(OBJECTS) $(PROGRAM)

run: $(PROGRAM)
	./$(PROGRAM)
