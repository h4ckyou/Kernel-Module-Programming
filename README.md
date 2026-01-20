# Kernel Module Programming

This is the setup process for the kernel environment (initramfs + qemu setup) I made use of

### Kernel Build
- Install Dependencies

```
- sudo apt update
- sudo apt install -y \
  qemu-system-x86 \
  build-essential \
  libncurses-dev \
  bison flex \
  libssl-dev \
  libelf-dev \
  bc \
  busybox-static \
  cpio
```

- Build Kernel from source (minimal)

```
- git clone --depth=1 --branch v6.8 https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git
- cd linux
- make x86_64_defconfig
- make menuconfig
```

- The config setup

```
General setup  --->
  [*] Configure standard kernel features (expert users)
  [*] Enable loadable module support
Kernel hacking  --->
  [*] Kernel debugging
  [*] KGDB
  [*] Compile-time checks and compiler options
```

- Build kernel

```
- make -j$(nproc)
```

After build is completed, we would get the compressed kernel bootable image: `arch/x86/boot/bzImage`

### Filesystem

- Create the `initramfs` filesystem

```
- mkdir initramfs
- cd initramfs
```

- Create directories

```
- mkdir -p {bin,sbin,etc,proc,sys,dev,lib,lib64,tmp,usr/{bin,sbin}}
```

- Install busybox

```
- cd bin
- busybox --install -s .
- cp /usr/bin/busybox ../usr/bin
- cd ../
```

- Create `init` script

```
#!/bin/sh
mount -t proc none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev
echo "Booted Linux"
exec /bin/sh
```

- Pack filesystem

```
- find . | cpio -o --format=newc | gzip > ../initramfs.cpio.gz
```

### Execute with qemu

- Boot the linux kernel
  
```
qemu-system-x86_64 \
  -kernel ../linux/arch/x86/boot/bzImage \
  -initrd initramfs.cpio.gz \
  -append "console=ttyS0 nokaslr" \
  -nographic
```

- Debug the linux kernel

```
- qemu-system-x86_64 \
  -kernel arch/x86/boot/bzImage \
  -initrd ../initramfs.cpio.gz \
  -append "console=ttyS0 nokaslr" \
  -nographic -s

- gdb vmlinux
- target remote :1234 
```

### Sample Kernel Mod Program

- kmod.c

```c
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mark Uche");
MODULE_DESCRIPTION("A simple Linux kernel module");

#define DRIVER_NAME "kmod"

struct proc_dir_entry *kmod_proc_entry;
struct proc_ops kmod_proc_ops; 

static int kmod_init(void) {
    printk(KERN_INFO "kmod: Initializing the kmod module\n");

    kmod_proc_entry = proc_create(DRIVER_NAME, 0, NULL, &kmod_proc_ops);
    if (!kmod_proc_entry) {
        printk(KERN_ERR "kmod: Failed to create /proc/%s\n", DRIVER_NAME);
        return -ENOMEM;
    }

    printk(KERN_INFO "kmod: /proc/%s created successfully\n", DRIVER_NAME);
    return 0;
}

static void kmod_exit(void) {
    printk(KERN_INFO "kmod: Cleaning up the kmod module\n");
    proc_remove(kmod_proc_entry);
    printk("kmod: /proc/%s removed successfully\n", DRIVER_NAME);
}

module_init(kmod_init);
module_exit(kmod_exit);
```

- Makefile

```make
obj-m += kmod.o

all:
	make -C ../linux M=$(PWD) modules

clean:
	make -C ../linux M=$(PWD) clean
```

- start.sh

```
#!/bin/bash

# Go to initramfs folder
cd initramfs || { echo "initramfs folder not found"; exit 1; }

# Check if kmod.ko exists
if [ ! -f "kmod.ko" ] && [ -f "../kmod.ko" ]; then
    echo "Copying kmod.ko into initramfs"
    cp ../kmod.ko .
fi

# Create cpio.gz initramfs
echo "Packing initramfs..."
find . | cpio -o -H newc | gzip > ../initramfs.cpio.gz
echo "Created initramfs.cpio.gz"

# Boot QEMU
echo "Booting QEMU..."
qemu-system-x86_64 \
  -kernel ../../linux/arch/x86/boot/bzImage \
  -initrd ../initramfs.cpio.gz \
  -append "console=ttyS0 nokaslr" \
  -nographic -s
```



