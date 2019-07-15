#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/conf.h>

#include "vmx.h"

static int vmm_load(struct module* m, int what, void *arg)
{
    int err = 0;
    switch (what)
    {
        case MOD_LOAD:
            printf("vmm: vmm modules is loaded\n");
            err = vmx_init();
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
