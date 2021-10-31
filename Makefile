CC=avr-gcc
AR=avr-ar

MCU=atmega328p
CPU_SPEED=16000000
CPPFLAGS=-DF_CPU=$(CPU_SPEED)
CFLAGS=-Wall -Wextra -mmcu=$(MCU) -Os -ffunction-sections -fdata-sections -c

OBJS=twi.o twi_master_tx.o twi_master_rx.o twi_disable.o twi_slave.o
TARGET=libtwi.a

.PHONY:
build: $(TARGET)

$(TARGET): $(OBJS)
	-rm -f $@
	$(AR) rcs $@ $^

%.o: %.c twi.h twi_int.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

.PHONY:
clean:
	@echo Cleaning ...
	rm -f $(OBJS)
	rm -f $(TARGET)
	@echo "done"

# vim: tabstop=8 noexpandtab shiftwidth=8
