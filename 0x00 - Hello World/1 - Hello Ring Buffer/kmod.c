#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h> 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mark Uche");
MODULE_DESCRIPTION("simple linux kernel module");

#define DRIVER_NAME "kmod"

static int kmod_init(void) {
    pr_info("kmod: Hello world called from init\n");
    return 0;
}

static void kmod_exit(void) {
    pr_info("kmod: Goodbye world called from exit\n");
}

module_init(kmod_init);
module_exit(kmod_exit);
