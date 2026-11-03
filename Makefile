CC      = gcc
CFLAGS  = -Wall -Wextra -g
TARGET  = trabalho
SRCS    = mymemory.c main.c

all: $(TARGET)

$(TARGET): $(SRCS) mymemory.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe *.o

.PHONY: all run clean
