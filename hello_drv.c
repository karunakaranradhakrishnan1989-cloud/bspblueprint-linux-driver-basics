// SPDX-License-Identifier: GPL-2.0
/*
 * hello_drv.c - A simple character device driver for learning.
 *
 * It creates /dev/hello. Whatever you write into it is kept in a
 * small kernel buffer, and whatever you read gives it back.
 *
 * BSP Blueprint - Linux Device Driver Basics
 */
#include <linux/module.h>	/* module_init, MODULE_LICENSE ... */
#include <linux/fs.h>		/* file_operations, alloc_chrdev_region */
#include <linux/cdev.h>		/* struct cdev */
#include <linux/device.h>	/* class_create, device_create */
#include <linux/uaccess.h>	/* copy_to_user, copy_from_user */
#include <linux/version.h>

#define DEVICE_NAME "hello"
#define BUF_SIZE    256

static dev_t hello_dev;			/* major + minor number */
static struct cdev hello_cdev;		/* the char device inside the kernel */
static struct class *hello_class;	/* shows up in /sys/class/hello */
static char kbuf[BUF_SIZE];		/* our "hardware": a kernel buffer */
static size_t data_len;

/* Called when user space does open("/dev/hello") */
static int hello_open(struct inode *inode, struct file *file)
{
	pr_info("hello: device opened\n");
	return 0;
}

/* Called when user space does close(fd) */
static int hello_release(struct inode *inode, struct file *file)
{
	pr_info("hello: device closed\n");
	return 0;
}

/* Called when user space does read(fd, buf, count) */
static ssize_t hello_read(struct file *file, char __user *ubuf,
			  size_t count, loff_t *off)
{
	size_t left;

	if (*off >= data_len)
		return 0;			/* end of file */

	left = data_len - *off;
	if (count > left)
		count = left;

	/* Kernel memory -> user memory. Never use memcpy here! */
	if (copy_to_user(ubuf, kbuf + *off, count))
		return -EFAULT;

	*off += count;
	pr_info("hello: sent %zu bytes to user\n", count);
	return count;
}

/* Called when user space does write(fd, buf, count) */
static ssize_t hello_write(struct file *file, const char __user *ubuf,
			   size_t count, loff_t *off)
{
	if (count > BUF_SIZE - 1)
		count = BUF_SIZE - 1;

	/* User memory -> kernel memory */
	if (copy_from_user(kbuf, ubuf, count))
		return -EFAULT;

	kbuf[count] = '\0';
	data_len = count;
	pr_info("hello: got %zu bytes from user\n", count);
	return count;
}

/* The menu card: which function handles which system call */
static const struct file_operations hello_fops = {
	.owner   = THIS_MODULE,
	.open    = hello_open,
	.release = hello_release,
	.read    = hello_read,
	.write   = hello_write,
	.llseek  = default_llseek,	/* lets the app use lseek() */
};

/* Runs once, when the driver is loaded (insmod) or at boot (built-in) */
static int __init hello_init(void)
{
	int ret;

	/* 1. Ask the kernel for a free major number */
	ret = alloc_chrdev_region(&hello_dev, 0, 1, DEVICE_NAME);
	if (ret)
		return ret;

	/* 2. Connect our file_operations to that number */
	cdev_init(&hello_cdev, &hello_fops);
	ret = cdev_add(&hello_cdev, hello_dev, 1);
	if (ret)
		goto err_region;

	/* 3. Create /sys/class/hello so udev can make /dev/hello */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
	hello_class = class_create(DEVICE_NAME);
#else
	hello_class = class_create(THIS_MODULE, DEVICE_NAME);
#endif
	if (IS_ERR(hello_class)) {
		ret = PTR_ERR(hello_class);
		goto err_cdev;
	}

	/* 4. Create the device node /dev/hello */
	if (IS_ERR(device_create(hello_class, NULL, hello_dev, NULL,
				 DEVICE_NAME))) {
		ret = -ENOMEM;
		goto err_class;
	}

	pr_info("hello: driver loaded, major=%d minor=%d\n",
		MAJOR(hello_dev), MINOR(hello_dev));
	return 0;

err_class:
	class_destroy(hello_class);
err_cdev:
	cdev_del(&hello_cdev);
err_region:
	unregister_chrdev_region(hello_dev, 1);
	return ret;
}

/* Runs when the driver is removed (rmmod). Undo init in reverse order. */
static void __exit hello_exit(void)
{
	device_destroy(hello_class, hello_dev);
	class_destroy(hello_class);
	cdev_del(&hello_cdev);
	unregister_chrdev_region(hello_dev, 1);
	pr_info("hello: driver unloaded\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BSP Blueprint");
MODULE_DESCRIPTION("A simple character driver for learning");
MODULE_VERSION("1.0");
