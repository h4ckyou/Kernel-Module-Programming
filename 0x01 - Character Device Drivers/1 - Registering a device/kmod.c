#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>

static dev_t dev_number;
static struct cdev my_cdev;

static int my_open(struct inode *inode, struct file *file) { return 0; }

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open  = my_open,
};

static int my_init(void)
{
    int ret = alloc_chrdev_region(&dev_number, 0, 1, "my_cdev");
    if (ret < 0) {
        pr_err("Failed to allocate char device region\n");
        return ret;
    }
    cdev_init(&my_cdev, &fops);
    cdev_add(&my_cdev, dev_number, 1);
    return 0;
}

static void my_exit(void)
{
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_number, 1);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");

// cat /proc/devices
// mknod /dev/my_cdev c 247 0
