CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Iinclude
SRCS    = src/system_time.c \
          src/sensor_adc.c \
          src/sensor_i2c.c \
          src/gpio_status.c \
          src/fault_manager.c \
          src/telemetry.c \
          src/serial_protocol.c \
          src/main.c
TARGET  = firmware_sim

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
