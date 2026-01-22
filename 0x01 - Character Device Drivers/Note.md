
Modules are object files whose symbols get resolved upon running `insmod` or `modprobe` . The definition for the symbols comes from the kernel itself; the only external functions you can use are the ones provided by the kernel.

-  View all exported kernel symbols

```
cat /proc/kallsyms
```

On standard x86 architecture, Unix-like operating systems (such as Linux, BSD, or macOS) use a two-ring system for protection, where user applications operate in **Ring 3**, and the kernel operates in **Ring 0**.

- Ring 0 - Supervisor Mode AKA Kernel Mode
- Ring 3 - User Mode 

Some CPUs support Ring 1 and 2, but most modern Operating Systems just use 0 (kernel) and 3 (user).

When we use a library function such as `printf`, typically it runs in user mode. The library function calls one or more system calls, and these system calls execute on the library function’s behalf, but do so in supervisor mode since they are part of the kernel itself. Once the system call completes its task, it returns and execution gets transferred back to user mode.

Any global variable defined in a loadable kernel module is part of a community of other peoples’ global variables in the kernel. When a program has lots of global variables which aren’t meaningful enough to be distinguished, you get namespace pollution.

One way to fix this issue is to make variables static, that way it limits the scope of the variable to that of the c file / kernel module.

On Unix, each piece of hardware is represented by a file located in /dev named a device file which provides the means to communicate with the hardware.

SCSI - Small Computer Storage Interface

The major number tells you which driver is used to access the hardware. Each driver is assigned a unique major number; all device files with the same major number are controlled by the same driver.

The minor number is used by the driver to distinguish between the various hardware it controls.

- **Major number** → which driver
- **Minor number** → which device instance

Devices are divided into two types: 
- character devices
- block devices

A **character device** transfers data **as a stream of bytes**, **sequentially**, with **no buffering by the kernel for reordering**.

A **block device** transfers data in **fixed-size blocks** and supports **random access**.

We can create a device file using this:

```
mknod /dev/mydev c <major_number> <minor_number>
```

We can register a char device using this:

```c
int register_chrdev(unsigned int major, const char *name, struct file_operations *fops);
```

The kernel can dynamically generate a major number if the major parameter is set to 0.

This makes creating the device file a bit harder since we wouldn't know the major number.

To resolve it, we can read the `/proc/devices` file which would hold the major number of our registered char device, or we can make the driver create a device file using the `device_create` function after a successful registration and `device_destroy` during the call to `cleanup_module` .

Using `register_chrdev` is wasteful however, because on using that, the kernel internally **reserves all minor numbers from 0 to 255** for that **major number**... and that's wasteful if your module only needs say, one device

The modern way is to make use of the `cdev` interface

This is the mental model of using the interface

```
Step 1: Reserve device numbers
          major/minor
          (register_chrdev_region / alloc_chrdev_region)

Step 2: Attach your driver to those numbers
          struct cdev → file_operations
          cdev_add()
```

You provide exactly what major or minor numbers you want (static allocation):

```c
int register_chrdev_region(dev_t first, unsigned count, const char *name)
```

Kernel dynamically picks a free major number for you :

```c
int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name)
```

**`dev_t`** is a combination of **major** and **minor**:

```c
#define MINORBITS 20
#define MINORMASK ((1U << MINORBITS) - 1)
  
#define MAJOR(dev) ((unsigned int) ((dev) >> MINORBITS))
#define MINOR(dev) ((unsigned int) ((dev) & MINORMASK))
#define MKDEV(ma,mi) (((ma) << MINORBITS) | (mi))
```

```c
dev_t dev;
int ret;

// allocate one dynamic major and 1 minor
ret = alloc_chrdev_region(&dev, 0, 1, "mydev");
```

- Kernel picks a free major (say 511)
- Minor = 0 (you asked for 1 minor)
- `dev` now contains the `(major, minor)` pair
 
Initialize and add a `cdev`

```c
int cdev_add(struct cdev *p, dev_t dev, unsigned count);
```

```c
struct cdev my_cdev;
cdev_init(&my_cdev, &fops);   // attach file_operations
my_cdev.owner = THIS_MODULE;

cdev_add(&my_cdev, dev, 1);   // register 1 device with the kernel
```

- `cdev_init()` -> prepares your `struct cdev`
- `cdev_add()` -> links it to the **major/minor range** you registered
- kernel now knows: _"calls to this major/minor go to this cdev’s file_operations"_

