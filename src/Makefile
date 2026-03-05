# Variables
CC = gcc
CFLAGS = -Wall
OBJ = coursework.o dataHandling.o io.o
TARGET = run.exe

# Default rule
all: $(TARGET)

# Link the object files into the final executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile each .c file into a .o file
coursework.o: coursework.c dataHandling.h io.h
	$(CC) $(CFLAGS) -c coursework.c

dataHandling.o: dataHandling.c dataHandling.h
	$(CC) $(CFLAGS) -c dataHandling.c

io.o: io.c io.h
	$(CC) $(CFLAGS) -c io.c

# Clean rule to remove compiled files
clean:
	rm -f $(OBJ) $(TARGET)
