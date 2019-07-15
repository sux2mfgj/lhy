#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/conf.h>

#include "vmx.h"

struct lhydev_softc {
    struct cdev *cdev;
};

static struct lhydev_softc sc;

static struct cdevsw lhy_dev_sw = {
    .d_name = "lhy_dev",
    .d_version = D_VERSION,
    //.d_ioctl = vmm
};


static int create_lhy_dev(void)
{
    int err = 0;
    struct cdev *cdev;

    //TODO
    err = make_dev_p(MAKEDEV_CHECKNAME, &cdev, &lhy_dev_sw, NULL, UID_ROOT, GID_WHEEL, 0600, "lhy");
    if(err)
    {
        return err;
    }

    sc.cdev = cdev;

    return err;
}

static int destroy_lhy_dev(void)
{
    int err = 0;
    if(sc.cdev != NULL)
    {
        destroy_dev(sc.cdev);
    }

    return err;
}

static int lhy_load(struct module* m, int what, void *arg)
{
    int err = 0;
    switch (what)
    {
        case MOD_LOAD:
            printf("lhy: lhy modules is loaded\n");
            err = vmx_init();
            if(err)
            {
                break;
            }
            err = create_lhy_dev();
            if(err)
            {
                vmx_deinit();
                break;
            }
            break;
        case MOD_UNLOAD:
            printf("lhy: lhy modules is unloaded\n");
            vmx_deinit();
            destroy_lhy_dev();
            break;
        default:
            err = EOPNOTSUPP;
            break;
    }

    return (err);
}

DEV_MODULE(lhy, lhy_load, NULL);
MODULE_VERSION(lhy, 0);
