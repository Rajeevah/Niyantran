#chmod a+rw /dev/parport0 
#The makefile
#

PROJECT = niyantran
MMCU = atmega32
P = m32
#PATHA = C:\winavr\bin\
#lpt1
#PATHA = /opt/cross/bin/
PORT = lpt1
INT = pony-stk200
HFUSE=0xd9
LFUSE=0xd4

#Beyond this line things should be pretty much constant

AVRDUDE = $(PATHA)avrdude -c $(INT) -p $(P) -P $(PORT)

CFLAGS=-Os -Wall -mmcu=$(MMCU)
#-O -mmcu=atmega32 -Wall -Wstrict-prototypes -Os -mcall-prologues

all: $(PROJECT).hex

$(PROJECT).hex : $(PROJECT).out 
	$(PATHA)avr-objcopy -R .eeprom -O ihex $(PROJECT).out $(PROJECT).hex 

$(PROJECT).out : $(PROJECT).o 
	$(PATHA)avr-gcc $(CFLAGS) -o $(PROJECT).out -Wl,-Map,$(PROJECT).map $(PROJECT).o 

$(PROJECT).o : $(PROJECT).c 
	$(PATHA)avr-gcc $(CFLAGS) -Os -c $(PROJECT).c

##test
flash: $(PROJECT).hex
	$(AVRDUDE) -U flash:w:$(PROJECT).hex

##test
fuse:
	$(AVRDUDE) -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m

clean:
	rm -f *.o *.map *.out *.hex
