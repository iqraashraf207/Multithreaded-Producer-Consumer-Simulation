CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g 
TARGET  = projectcode
SRC     = projectcode.c
LIBS    = -lpthread

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)
	@echo "Build successful!! Run with: ./$(TARGET)"

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
