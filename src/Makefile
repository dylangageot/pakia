# derived from https://gist.github.com/ryanleary/8250880

DEVICE     = attiny85           # See avr-help for all possible devices
CLOCK      = 8000000            # 8Mhz
PROGRAMMER = -P /dev/ttyACM0 -c avrisp -b 19200  
								# For using Arduino as ISP
OBJECTS    = main.o             # Add more objects for each .c file here

# fuse settings:
# use http://www.engbedded.com/fusecalc
#FUSES      = -U lfuse:w:0x62:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m  # 1mhz
FUSES      = -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m  # 8mhz

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
ACC ?= avr-gcc
CFLAGS = -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

# symbolic targets:
all: main.hex

%.o : %.c
	$(ACC) $(CFLAGS) -c $< -o $@

.S.o:
	$(ACC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

.c.s:
	$(ACC) $(CFLAGS) -S $< -o $@

flash: all
	$(AVRDUDE) -U flash:w:main.hex:i

fuse:
	$(AVRDUDE) $(FUSES)

install: flash fuse

clean:
	rm -f main.hex main.elf $(OBJECTS)

# file targets:
main.elf: $(OBJECTS)
	$(ACC) $(CFLAGS) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size --format=avr --mcu=$(DEVICE) main.elf
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

# Targets for code debugging and analysis:
disasm: main.elf
	avr-objdump -d main.elf
