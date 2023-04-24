#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(int argc, char *argv[])
{
    int fd;
    unsigned char databuf[1];
    char *devname;
    struct input_event inputevent;
    int ret;
    if (argc != 2)
    {
        printf("Usage: need one arguments!\n");
        exit(EXIT_FAILURE);
    }

    devname = argv[1];
    if ((fd = open(devname, O_RDWR)) < 0)
    {
        perror("open");
        printf("%s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    while (1)
    {
        if ((ret = read(fd, &inputevent, sizeof(inputevent))) < 0)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        else if (ret == 0)
        {
            printf("EOF!");
        }
        else
        {
            switch (inputevent.type)
            {
            case EV_KEY:
                if (inputevent.code < BTN_MISC)
                {
                    printf("key %d %s ", inputevent.code,
                           inputevent.value ? "press" : "release");
                }
                else
                {
                    printf("button %d %s ", inputevent.code,
                           inputevent.value ? "press" : "release");
                }
                break;
            default:
                printf("something wrong!,not key type!\n");
                break;
            }
        }
    }

    if ((ret = close(fd)) < 0)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
    return 0;
}