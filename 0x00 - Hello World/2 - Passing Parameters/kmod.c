#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/printk.h>
#include <linux/stat.h>  

// insmod kmod.ko param_var=1337

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mark Uche");
MODULE_DESCRIPTION("simple linux kernel module");

#define DRIVER_NAME "kmod"

static int param_var = 1;

module_param(param_var, int, S_IRUGO); // /sys/module/kmod/parameters/param_var
MODULE_PARM_DESC(param_var, "An integer parameter");

static int kmod_init(void) {
    pr_info("kmod: init\n");
    pr_info("kmod: param_var value is: %d\n", param_var);
    return 0;
}

static void kmod_exit(void) {
    pr_info("kmod: exit\n");
}

module_init(kmod_init);
module_exit(kmod_exit);
