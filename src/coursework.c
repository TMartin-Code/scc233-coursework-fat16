// Header Files

#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdint.h>
#include<errno.h>
#include<stdio.h>
#include<unistd.h>
#include<stddef.h>

// fat16 FileSystem in C

char * file = "fat16.img";

/*
Task 1 - Demonstrate you can open a file, perhaps a text file, jump to a specific point given a byte offset into the file, 
and read a given number of bytes or characters before closing the file.

open( ) -- tells the system you want to read the FAT16 image
lseek( ) – move to a given point in the file
read( ) – read a number of bytes from a file into a given area of memory
close( ) – tells the system any resources used for reading the image can be released.

*/

// Function that opens a file as read only
// Takes filename
int fileOpen(const char *filePath)
{
    int fd = open(filePath, O_RDONLY);
    if (fd < 0)
    {
        perror("Couldn't Open File To Read");
    }
    return fd;
}

void fileClose(int fd)
{
    close(fd);
}

int fileRead(int fd, uint32_t offset, void *buffer, size_t size)
{
    if (lseek(fd, offset, SEEK_SET) < 0)
    {
        perror("lseek");
        return -1;
    }

    ssize_t bytes = read(fd, buffer, size);
    if (bytes < 0) 
    {
        perror("read");
        return -1;
    }

    if ((size_t)bytes != size) 
    {
        perror("Short Read");
    }

    return 0;
}
int main(int argc, char *argv[])
{ 
    if (argc != 2)
    {
        printf("Error");
    }

    int fd = fileOpen(argv[1]);
    if (fd < 0) 
    {
        return 0;
    }

    u_int8_t buffer[16];

    if (fileRead(fd, 0, buffer, sizeof(buffer)) == 0)
    {
        for (int i = 0; i < 16; i++)
        {
            printf("%02X ", buffer[i]);
        }
        printf("\n");

    }
    fileClose(fd);

    return 0;
}