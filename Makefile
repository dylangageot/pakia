# derived from https://gist.github.com/ryanleary/8250880

PROJECT    = pakia
VERSION	   = 0.1.0
NAME	   = $(PROJECT)-$(VERSION)

DEVICE     = attiny85           # See avr-help for all possible devices
CLOCK      = 8000000            # 8Mhz
PROGRAMMER = -P /dev/ttyACM0 -c avrisp -b 19200  
								# For using Arduino as ISP

SRCS       = src/main.cc src/amiga.cc src/ps2.cc
OBJS	   = $(SRCS:.cc=.o)

# fuse settings:
# use http://www.engbedded.com/fusecalc
#FUSES      = -U lfuse:w:0x62:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m  # 1mhz
FUSES      = -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m  # 8mhz

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
ACC ?= avr-g++
CFLAGS = -std=c++11 -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

# symbolic targets:
all: $(NAME).hex

%.o : %.cc
	$(ACC) $(CFLAGS) -c $< -o $@

flash: $(NAME).hex
	$(AVRDUDE) -U flash:w:$<:i

fuse:
	$(AVRDUDE) $(FUSES)

install: flash fuse

clean:
	rm -rf $(OBJS) $(NAME).elf $(NAME).hex

# file targets:
$(NAME).elf: $(OBJS)
	$(ACC) $(CFLAGS) -o $@ $^

$(NAME).hex: $(NAME).elf
	rm -f $@
	avr-objcopy -j .text -j .data -O ihex $< $@
	avr-size --format=avr --mcu=$(DEVICE) $<
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

# Targets for code debugging and analysis:
disasm: $<
	avr-objdump -d $<
