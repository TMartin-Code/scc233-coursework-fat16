//Task1.c
# include "Task1.h"
#include <>
// Functions
void readImage(int offset, char * filePath, int toRead, void * buffer)
{
    // Task 1
    int fd = open(filePath, O_RDONLY);
    if (fd < 0) {
        perror("Error Opening to Read\n");
        return;
    }

    lseek(fd, offset, SEEK_SET);

    if (read(fd, buffer, toRead) < 0)
    {
        perror("Error reading from file\n");
        return;
    }

    close(fd);
    return;
}
