CC = g++
CFLAGS = -Wno-deprecated

SOURCES = main.cpp

PROGRAM_NAME = build/STM32HeaderGen

all : $(PROGRAM_NAME)

$(PROGRAM_NAME): $(SOURCES) build
	$(CC) $(CFLAGS) -o $@ $<

build:
	mkdir -p build

clean:
	rm -rf build
