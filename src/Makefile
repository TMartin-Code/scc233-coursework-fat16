# Variables
CC = gcc
CFLAGS = -Wall
OBJ = coursework.o dataHandling.o io.o
TARGET = run.exe

# All rule to compile files
all: $(TARGET)

# Compile each .c file into a .o file
coursework.o: coursework.c dataHandling.h io.h
	$(CC) $(CFLAGS) -c coursework.c

dataHandling.o: dataHandling.c dataHandling.h
	$(CC) $(CFLAGS) -c dataHandling.c

io.o: io.c io.h
	$(CC) $(CFLAGS) -c io.c

# Link the object files into the final executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o ../out/$(TARGET)

# Clean rule to remove compiled files
clear:
	rm -f $(OBJ) ../out/$(TARGET)
