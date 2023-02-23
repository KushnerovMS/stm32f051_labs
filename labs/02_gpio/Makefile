#-----------------------
# Compiler/linker flags
#-----------------------

CC = arm-none-eabi-gcc

CFLAGS = \
	-std=c18 \
	-Wall \
	-Wextra \
	-Werror \
	-march=armv6-m \
	-mcpu=cortex-m0

LDFLAGS = \
	-Wall \
	-Wextra \
	-Werror \
	-Wl,--start-group -lgcc -lc -lg -Wl,--end-group -Wl,--gc-sections \
	-march=armv6-m \
	-mcpu=cortex-m0 \
	-Wl,--warn-common \
	-Wl,--fatal-warnings \
	-Wl,-z,max-page-size=8 \
	-Wl,-T,entry.lds

ifeq ($(DEBUG),1)
	CFLAGS += -g
endif

#-------
# Files
#-------

SOURCES = \
	entry.S \
	reaction.c

OBJECTS_HALFWAY_DONE = $(SOURCES:%.c=build/%.o)
OBJECTS              = $(OBJECTS_HALFWAY_DONE:%.S=build/%.o)

EXECUTABLE_FLASH = build/reaction.elf
BINARY_FLASH     = build/reaction.bin

GEN_HEADER 	= STM32F051.h
HEADER_GEN_BIN = ../../STM32HeaderGen/build/STM32HeaderGen
GEN_HEADER_SOURCE = ../../STM32HeaderGen/STM32F051_header

#---------------
# Build scripts
#---------------

all: $(EXECUTABLE_FLASH) $(BINARY_FLASH) $(SOURCES)

$(EXECUTABLE_FLASH): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

$(BINARY_FLASH): $(EXECUTABLE_FLASH)
	arm-none-eabi-objcopy -O binary $< $@

build/%.o: %.c $(GEN_HEADER)
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ -c $<

build/%.o: %.S
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf build

$(GEN_HEADER): $(GEN_HEADER_SOURCE) $(HEADER_GEN_BIN)
	$(HEADER_GEN_BIN) $(GEN_HEADER_SOURCE)

$(HEADER_GEN_BIN):
	cd ../../STM32HeaderGen; make

#----------------------
# Hardware interaction
#----------------------

flash: $(BINARY_FLASH)
	st-flash write $(BINARY_FLASH) 0x08000000

GDB_FLAGS = \
	--eval-command="set architecture arm" \
	--eval-command="set gnutarget elf32-littlearm" \
	--eval-command="target remote localhost:1234" \
	--eval-command="file $(EXECUTABLE_FLASH)"

hardware: $(EXECUTABLE_FLASH)
	st-util -p 1234

gdb: $(BINARY_FLASH)
	arm-none-eabi-gdb $(GDB_FLAGS)

.PHONY: all clean flash hardware gdb
