CC := gcc
override CFLAGS += -O3 -Wall

SOURCE1 := sender.c
BINARY1 := sender

SOURCE2 := receiver.c
BINARY2 := receiver

HEADERS := struct.h

all: $(BINARY1) $(BINARY2)

$(BINARY1): $(SOURCE1) $(patsubst %.c, %.h, $(SOURCE1)) $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

$(BINARY2): $(SOURCE2) $(patsubst %.c, %.h, $(SOURCE2)) $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(BINARY1) $(BINARY2)