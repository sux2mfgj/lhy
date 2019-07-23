#ifndef __LHY_API_H
#define __LHY_API_H

#include <sys/ioccom.h>

enum lhy_ioctl_api {
    // TODO
    LHY_DEBUG = _IO('L', 255),
    LHY_VCPU_RUN = _IO('L', 0),
};

#endif //__LHY_API_H
