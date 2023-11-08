CFLAGS      = -V -mmcs51 --model-large --xram-size 0x1800 --xram-loc 0x0000 \
	--code-size 0xec00 --stack-auto --Werror -I../chlib/src -I../jvsio \
	--opt-code-speed
CC          = sdcc
FLASHER     = ../CH55x_python_flasher/chflasher.py
TARGET      = iona
OBJS  	    = main.rel client.rel controller.rel settings.rel soft485.rel \
  adc.rel ch559.rel gpio.rel hid.rel hid_dualshock3.rel hid_guncon3.rel \
	hid_keyboard.rel hid_mouse.rel hid_switch.rel hid_xbox.rel led.rel pwm1.rel \
	serial.rel timer3.rel usb_host.rel jvsio_node.rel
LOGGER      = logger
LOGGER_OBJS = logger.rel ch559.rel flash.rel gpio.rel hid.rel \
  hid_dualshock3.rel hid_guncon3.rel hid_keyboard.rel hid_mouse.rel \
	hid_switch.rel hid_xbox.rel led.rel serial.rel timer3.rel usb_host_debug.rel

.PHONY: all clean program run clean build prod

all: build $(TARGET).bin $(LOGGER).bin

program: $(TARGET).bin
	$(FLASHER) -w -f $(TARGET).bin

run: program
	$(FLASHER) -s

prod:
	ch559flasher -W ../docs/firmwares/us_v2_data1.bin
	$(FLASHER) -w -f ../docs/firmwares/us_v2_17.bin
	$(FLASHER) -s

logger: $(LOGGER).bin
	$(FLASHER) -w -f $(LOGGER).bin

clean:
	rm -rf build $(TARGET).bin logger.bin

.SILENT:
build:
	mkdir -p build

build/%.rel: %.c ../jvsio/*.h ../chlib/src/*.h *.h
	$(CC) -c $(CFLAGS) -I../ -o $@ $<

build/%.rel: ../chlib/src/%.c ../chlib/src/*.h
	$(CC) -c $(CFLAGS) -o $@ $<

build/usb_host_debug.rel: ../chlib/src/usb_host.c ../chlib/src/*.h
	$(CC) -c $(CFLAGS) -D_USB_HOST_DBG_LOG -o $@ $<

build/jvsio_node.rel: ../jvsio/jvsio_node.c ../jvsio/*.h *.h
	$(CC) -c $(CFLAGS) -I ../chlib/src -o $@ $<

build/$(TARGET).ihx: $(addprefix build/,$(OBJS))
	$(CC) $(CFLAGS) $(addprefix build/,$(OBJS)) -o $@

build/$(LOGGER).ihx: $(addprefix build/,$(LOGGER_OBJS))
	$(CC) $(CFLAGS) $(addprefix build/,$(LOGGER_OBJS)) -o $@

%.bin: build/%.ihx
	sdobjcopy -I ihex -O binary $< $@
