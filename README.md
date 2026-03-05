[Fat 16 File-System Implementation Coursework]h
  Module - SCC.233 Operating Systems
  Date - 05/03/2026
  Name - Thomas Martin
  
## Project Overview
  This project implements the fat-16 file system, to read data from a disk image and present it to the user. Additionally, this program includes as command-line based ui where users can request data for files.
  
## Requirements 
  GCC - GNU Compiler
  Make - To Compile the project
  
## Compiling the Program
```
  [~/src]$ make all - Will compile the .c and .h file into the run.exe file.
  [~/src]$ make clear - Will remove the .o and run.exe file created by the make all command.
```
  --- Compiling without make ---
```
  [~/src]$ gcc coursework.c dataHandling.c io.c -o ../out/run.exe
```  
## Running the Program
```
  [~/out]$ ./run.exe <integer for byte offset (task1)> <integer for length of bytes to read (task1)> <Integer for starting cluster number (task3)> <filepath of disk image to read from (default is 'fat16.img')>
```
