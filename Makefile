CC	= gcc
CFLAGS	= -Wall -g -I include
# Added "Central Point/main.c" to the source list so the compiler finds your main function
SRC	= "Central Point/main.c" $(wildcard src/*.c)
OBJ	= $(SRC:.c=.o)
TARGET	= mini_os.exe

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	del /f $(TARGET) $(OBJ)
