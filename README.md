# hello_drv — your first Linux character driver

Companion code for the BSP Blueprint video "Your First Linux Device Driver".

| File | What it is |
|---|---|
| `hello_drv.c` | The driver. Creates `/dev/hello`; write stores a message, read returns it. |
| `Makefile` | Out-of-tree build (loadable module `hello_drv.ko`). |
| `test_app.c` | User-space program that opens, writes, reads and closes `/dev/hello`. |
| `builtin/` | The Kconfig entry and Makefile line to build the driver *into* the kernel. |

Tested on Linux 6.18: built with `W=1` and no warnings, then loaded, used and removed as a module, and booted built in. It also compiles against Linux 6.6.
It also handles kernels older than 6.4, where `class_create()` still took `THIS_MODULE`.

## 1. Loadable module

```sh
sudo apt install build-essential linux-headers-$(uname -r)   # Ubuntu / Debian
make
sudo insmod hello_drv.ko
modinfo hello_drv.ko
```

Check that it's loaded:

```sh
lsmod | grep hello
sudo dmesg | grep hello          # hello: driver loaded, major=NNN minor=0
grep hello /proc/devices
ls -l /dev/hello                 # crw------- ... NNN, 0 ... /dev/hello
ls /sys/module/hello_drv
```

Use it:

```sh
echo "Hi from BSP Blueprint" | sudo tee /dev/hello
sudo cat /dev/hello
gcc test_app.c -o test_app
sudo ./test_app "Hello from user space"
sudo dmesg | tail -4
sudo rmmod hello_drv
```

If `insmod` says **Key was rejected by service**, Secure Boot is blocking unsigned modules.
Sign the module, or turn off Secure Boot on your practice machine.

Using modprobe instead:

```sh
sudo make -C /lib/modules/$(uname -r)/build M=$PWD modules_install
sudo depmod -a
sudo modprobe hello_drv
sudo modprobe -r hello_drv
```

## 2. Built into the kernel

1. Copy `hello_drv.c` to `<linux>/drivers/char/`.
2. Add the entry from `builtin/Kconfig.snippet` to `drivers/char/Kconfig`.
3. Add the line from `builtin/Makefile.snippet` to `drivers/char/Makefile`.
4. `make menuconfig` → Device Drivers → Character devices → set **Hello character driver** to `<*>`.
5. Rebuild and boot the new image, e.g. for an ARM64 board:
   `make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j$(nproc) Image`

Verify (lsmod will **not** list a built-in driver):

```sh
dmesg | grep hello
ls -l /dev/hello
zcat /proc/config.gz | grep HELLO      # needs CONFIG_IKCONFIG_PROC, or check .config
grep hello /lib/modules/$(uname -r)/modules.builtin
```

## Homework

Add an `ioctl` that clears the buffer. Hint: `.unlocked_ioctl` in `file_operations`, plus `_IO()` to define the command number.
