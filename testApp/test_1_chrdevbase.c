#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(int argc, char *argv[])
{
    a = (struct abc *)5;
    int fd;
    unsigned char databuf[1];
    char *devname;
    char ledstate[1];
    int ret;
    if (argc < 3)
    {
        printf("Usage: need two arguments!\n");
        exit(EXIT_FAILURE);
    }

    devname = argv[1];
    ledstate[0] = atoi(argv[2]);
    if ((fd = open(devname, O_RDWR)) < 0)
    {
        perror("open1");
        printf("%s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if ((ret = write(fd, ledstate, sizeof(ledstate))) < 0)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
    if ((ret = close(fd)) < 0)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
    return 0;
}