CC	= gcc
CFLAGS	= -Wall -g -I include
SRC	= $(wildcard src/*.c)
OBJ	= $(SRC:.c=.o)
TARGET	= mini_os

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) $(OBJ)
