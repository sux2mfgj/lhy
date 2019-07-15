#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "lhy_api.h"

int main(int argc, char const* argv[])
{
    int fd = open("/dev/lhy", O_RDWR);
    if(fd < 0)
    {
        perror("open");
        return -1;
    }

    int error = ioctl(fd, LHY_DEBUG);
    //int error = _IO(fd, LHY_DEBUG);
    if(error)
    {
        perror("ioctl LHY_DEBUG");
        return -2;
    }

    return 0;
}
