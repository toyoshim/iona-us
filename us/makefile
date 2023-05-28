CFLAGS  = -V -mmcs51 --model-large --xram-size 0x1800 --xram-loc 0x0000 --code-size 0xec00 --stack-auto --Werror -I../chlib/src -I../jvsio --opt-code-speed
CC      = sdcc
FLASHER = ../CH55x_python_flasher/chflasher.py
TARGET  = iona
OBJS	= main.rel client.rel controller.rel hid.rel hid_guncon3.rel hid_keyboard.rel hid_switch.rel hid_xbox.rel settings.rel soft485.rel ch559.rel led.rel pwm1.rel serial.rel timer3.rel usb_host.rel jvsio_node.rel

.PHONY: clean program run clean build

all: build $(TARGET).bin

program: $(TARGET).bin
	$(FLASHER) -w -f $(TARGET).bin

run: program
	$(FLASHER) -s

clean:
	rm -rf build $(TARGET).bin

.SILENT:
build:
	mkdir -p build

build/%.rel: %.c ../jvsio/*.h ../chlib/src/*.h *.h
	$(CC) -c $(CFLAGS) -I../ -o $@ $<

build/%.rel: ../chlib/src/%.c ../chlib/src/*.h
	$(CC) -c $(CFLAGS) -o $@ $<

build/jvsio_node.rel: ../jvsio/jvsio_node.c ../jvsio/*.h *.h
	$(CC) -c $(CFLAGS) -I ../chlib/src -o $@ $<

build/$(TARGET).ihx: $(addprefix build/,$(OBJS))
	$(CC) $(CFLAGS) $(addprefix build/,$(OBJS)) -o $@

%.bin: build/%.ihx
	sdobjcopy -I ihex -O binary $< $@
