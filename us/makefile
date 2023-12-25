INCLUDES		= -I../chlib/src -I../jvsio
CFLAGS      = -V -mmcs51 --model-large --xram-size 0x1800 --xram-loc 0x0000 \
	--code-size 0xec00 --stack-auto --Werror --opt-code-speed $(INCLUDES)
WCFLAGS			=	-O3 $(INCLUDES)
WLFLAGS			=	\
	-s WASM=1 \
	-s EXPORTED_RUNTIME_METHODS='["ccall"]' \
	-s NO_EXIT_RUNTIME=1 \
	-s WARN_ON_UNDEFINED_SYMBOLS=0
CC          = sdcc
TARGET      = iona
OBJS  	    = main.rel client.rel controller.rel settings.rel soft485.rel \
  adc.rel ch559.rel gpio.rel hid.rel hid_dualshock3.rel hid_guncon3.rel \
	hid_keyboard.rel hid_mouse.rel hid_switch.rel hid_xbox.rel led.rel pwm1.rel \
	serial.rel timer3.rel usb_host.rel jvsio_node.rel
LOGGER      = logger
LOGGER_OBJS = logger.rel ch559.rel flash.rel gpio.rel hid.rel \
  hid_dualshock3.rel hid_guncon3.rel hid_keyboard.rel hid_mouse.rel \
	hid_switch.rel hid_xbox.rel led.rel serial.rel timer3.rel usb_host_debug.rel
WASM_OBJS		= wasm.o controller.o hid.o hid_dualshock3.o hid_guncon3.o \
	hid_keyboard.o hid_mouse.o hid_switch.o hid_xbox.o serial.o

.PHONY: all clean program run clean build prod wasm serv

all: build $(TARGET).bin $(LOGGER).bin

program: $(TARGET).bin
	ch559flasher -w $(TARGET).bin

run: program
	ch559flasher -b

prod:
	ch559flasher -W ../docs/firmwares/us_v2_data1.bin -w ../docs/firmwares/us_v2_22.bin -b

logger: $(LOGGER).bin
	ch559flasher -w $(LOGGER).bin

wasm: iona.html

serv:
	python -m http.server 8000

clean:
	rm -rf build $(TARGET).bin logger.bin iona.*

.SILENT:
build:
	mkdir -p build

build/%.rel: %.c ../jvsio/*.h ../chlib/src/*.h *.h
	$(CC) -c $(CFLAGS) -I../ -o $@ $<

build/%.rel: ../chlib/src/%.c ../chlib/src/*.h ../chlib/src/usb/*.h ../chlib/src/usb/hid/*.h
	$(CC) -c $(CFLAGS) -o $@ $<

build/%.rel: ../chlib/src/usb/%.c ../chlib/src/*.h ../chlib/src/usb/*.h ../chlib/src/usb/hid/*.h
	$(CC) -c $(CFLAGS) -o $@ $<

build/%.rel: ../chlib/src/usb/hid/%.c ../chlib/src/*.h ../chlib/src/usb/*.h ../chlib/src/usb/hid/*.h
	$(CC) -c $(CFLAGS) -o $@ $<

build/%.o: %.c ../jvsio/*.h ../chlib/src/*.h *.h
	emcc -c $(WCFLAGS) -I../ -o $@ $<

build/%.o: ../chlib/src/%.c ../chlib/src/*.h
	emcc -c $(WCFLAGS) -o $@ $<

build/usb_host_debug.rel: ../chlib/src/usb/usb_host.c ../chlib/src/*.h ../chlib/src/usb/*.h ../chlib/src/usb/hid/*.h
	$(CC) -c $(CFLAGS) -D_USB_HOST_DBG_LOG -o $@ $<

build/jvsio_node.rel: ../jvsio/jvsio_node.c ../jvsio/*.h *.h
	$(CC) -c $(CFLAGS) -I ../chlib/src -o $@ $<

build/$(TARGET).ihx: $(addprefix build/,$(OBJS))
	$(CC) $(CFLAGS) $(addprefix build/,$(OBJS)) -o $@

build/$(LOGGER).ihx: $(addprefix build/,$(LOGGER_OBJS))
	$(CC) $(CFLAGS) $(addprefix build/,$(LOGGER_OBJS)) -o $@

%.bin: build/%.ihx
	sdobjcopy -I ihex -O binary $< $@

iona.html: build $(addprefix build/,$(WASM_OBJS))
	emcc $(WLFLAGS) $(addprefix build/,$(WASM_OBJS)) -o $@