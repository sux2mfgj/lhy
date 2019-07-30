#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "lhy_api.h"

char *commands[] = {
    "debug",
    "run",
};

int main(int argc, char const* argv[])
{
    if(argc <= 1)
    {
        printf("any argument is required.\n");
        return 0;
    }

    long cmd = -1;
    printf("%lu %lu\n", sizeof(commands[0]), sizeof(commands[1]));
    if(strncmp(argv[1], commands[0], sizeof(commands[0]) - 1) == 0)
    {
        cmd = LHY_DEBUG;
        printf("debug\n");
    }
    if(strncmp(argv[1], commands[1], sizeof(commands[1]) - 1) == 0)
    {
        cmd = LHY_VCPU_RUN;
        printf("vcpu_run\n");
    }

    if(cmd == -1)
    {
        perror("invalid argument\n");
        return -1;
    }

    int fd = open("/dev/lhy", O_RDWR);
    if(fd < 0)
    {
        perror("open");
        return -1;
    }

    //int error = ioctl(fd, LHY_DEBUG);
    int error = ioctl(fd, cmd);
    if(error)
    {
        perror("ioctl");
        return -2;
    }

    return 0;
}
