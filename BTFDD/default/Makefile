###############################################################################
# Makefile for the project BTFDD
###############################################################################

## General Flags
PROJECT = BTFDD
MCU = atmega324p
TARGET = BTFDD.elf
CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99     -DF_CPU=18432000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=BTFDD.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = BTFDD.o uart0.o fdc.o global.o shell.o spi.o timer1.o crc16.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) BTFDD.hex BTFDD.eep BTFDD.lss## Compile
BTFDD.o: ../BTFDD.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

uart0.o: ../uart0.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

fdc.o: ../fdc.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

global.o: ../global.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

shell.o: ../shell.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

spi.o: ../spi.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

timer1.o: ../timer1.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

crc16.o: ../crc16.S
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) BTFDD.elf dep/* BTFDD.hex BTFDD.eep BTFDD.lss BTFDD.map


## Other dependencies
-include $(shell mkdir dep 2>NUL) $(wildcard dep/*)

