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


