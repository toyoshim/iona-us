CFLAGS  = -V -mmcs51 --model-large --xram-size 0x1800 --xram-loc 0x0000 --code-size 0xf000 --stack-auto
CC      = sdcc
FLASHER = ../CH55x_python_flasher/chflasher.py
TARGET  = iona
OBJS	= main.rel jamma.rel dipsw.rel ch559.rel pwm1.rel rs485.rel serial.rel client.rel JVSIO_c.rel

all: $(TARGET).bin

program: $(TARGET).bin
	$(FLASHER) -w -f $(TARGET).bin

run: program
	$(FLASHER) -s

clean:
	rm -f *.asm *.lst *.rel *.rst *.sym $(TARGET).bin $(TARGET).ihx $(TARGET).lk $(TARGET).map $(TARGET).mem

%.rel: %.c ../jvsio/JVSIO_c.h ../chlib/*.h *.h
	$(CC) -c $(CFLAGS) -I../ $<

%.rel: ../chlib/%.c ../chlib/*.h
	$(CC) -c $(CFLAGS) $<

JVSIO_c.rel: ../jvsio/JVSIO_c.c ../jvsio/JVSIO_c.h *.h
	$(CC) -c $(CFLAGS) -I ../chlib $<

$(TARGET).ihx: $(OBJS) 
	$(CC) $(CFLAGS) $(OBJS) -o $@

%.bin: %.ihx
	sdobjcopy -I ihex -O binary $< $@
