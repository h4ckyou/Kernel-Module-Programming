Makefile

```make
obj-m += kmod.o

all:
	make -C ../linux M=$(PWD) modules

clean:
	make -C ../linux M=$(PWD) clean
```

View kernel module info

```
modinfo kmod.ko
```

Load kernel module

```
insmod kmod.ko
```

View the ring log buffer

```
dmesg
```

Remove loaded kernel module

```
rmmod kmod.ko
```
