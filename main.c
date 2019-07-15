#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/conf.h>

#include "vmx.h"

static struct cdevsw vmm_dev_sw = {
    .d_name = "vmm_dev",
    .d_version = D_VERSION,
    //.d_ioctl = vmm
};

static int create_vmm_dev(void)
{
    int err = 0;
    struct cdev *cdev;

    //TODO
    err = make_dev_p(MAKEDEV_CHECKNAME, &cdev, &vmm_dev_sw, NULL, UID_ROOT, GID_WHEEL, 0600, "vmm");
    if(err)
    {
        return err;
    }

    return err;
}

static int vmm_load(struct module* m, int what, void *arg)
{
    int err = 0;
    switch (what)
    {
        case MOD_LOAD:
            printf("vmm: vmm modules is loaded\n");
            err = vmx_init();
            if(err)
            {
                break;
            }
            err = create_vmm_dev();
            if(err)
            {
                vmx_deinit();
                break;
            }
            break;
        case MOD_UNLOAD:
            printf("vmm: vmm modules is unloaded\n");
            break;
        default:
            err = EOPNOTSUPP;
            break;
    }

    return (err);
}

DEV_MODULE(vmm, vmm_load, NULL);
MODULE_VERSION(vmm, 0);
