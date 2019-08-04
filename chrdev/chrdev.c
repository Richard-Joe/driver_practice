#include <linux/init.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/kern_levels.h>


static int chrdev_open(struct inode *inode, struct file *file) {
    printk(KERN_ALERT "chrdev_open\n");
    return 0;
}

static int chrdev_release(struct inode *inode, struct file *file) {
    printk(KERN_ALERT "chrdev_release\n");
    return 0;
}

static ssize_t chrdev_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos) {
    int ret;
    char message[] = "This is chrdev test\n";
    ssize_t bytes = sizeof(message);

    if (*ppos) {
        return 0;
    }

    printk(KERN_ALERT "chrdev_read\n");

    ret = copy_to_user(ubuf, message, bytes);
    if (ret) {
        printk(KERN_WARNING "copy_to_user failed\n");
        return -EINVAL;
    }
    *ppos += bytes;

    printk(KERN_ALERT "read success\n");

    return bytes;
}


static ssize_t chrdev_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos) {
    int ret;
    char kbuf[128];

    printk(KERN_ALERT "chrdev_write\n");

    memset(kbuf, 0, sizeof(kbuf));
    ret = copy_from_user(kbuf, ubuf, count);
    if (ret) {
        printk(KERN_WARNING "copy_from_user failed\n");
        return -EINVAL;
    }

    printk(KERN_ALERT "write [%s] success\n", kbuf);

    return count;
}


#define CHRDEV_NAME "chrdev"

static dev_t dev;
static struct cdev chrdev;
static struct class *chr_class;
static struct device *chr_device;

static struct file_operations chrdev_fops = {
    .owner   = THIS_MODULE,
    .open    = chrdev_open,
    .read    = chrdev_read,
    .write   = chrdev_write,
    .release = chrdev_release,
};


static int chrdev_init(void) {
    unsigned int major = 0, minor = 0;
    int ret;

    /* register major/minor device number */
    if (major) {
        dev = MKDEV(major, minor);
        ret = register_chrdev_region(dev, 1, CHRDEV_NAME);
    } else {
        ret = alloc_chrdev_region(&dev, minor, 1, CHRDEV_NAME);
    }

    if (ret) {
        printk(KERN_WARNING "can't get major %d\n", major);
        return -EINVAL;
    }

    printk(KERN_ALERT "register major success\n");

    /* register char device driver */
    cdev_init(&chrdev, &chrdev_fops);
    ret = cdev_add(&chrdev, dev, 1);
    if (ret) {
        printk(KERN_WARNING "cdev_add failed\n");
        goto unregister_chrdev;
    }

    printk(KERN_ALERT "register chrdev success\n");

    chr_class = class_create(THIS_MODULE, "chrdev");
    if (NULL == chr_class) {
        printk(KERN_WARNING "class_create failed\n");
        goto del_cdev;
    }

    chr_device = device_create(chr_class, NULL, dev, NULL, CHRDEV_NAME);
    if (NULL == chr_device) {
        printk(KERN_WARNING "device_create failed\n");
        goto destory_class;
    }

    printk(KERN_ALERT "chrdev_init completed\n");

    return 0;

destory_class:
    class_destroy(chr_class);
del_cdev:
    cdev_del(&chrdev);
unregister_chrdev:
    unregister_chrdev_region(dev, 1);
    return -EINVAL;
}


static void chrdev_exit(void) {
    device_destroy(chr_class, dev);
    class_destroy(chr_class);

    cdev_del(&chrdev);
    unregister_chrdev_region(dev, 1);

    printk(KERN_ALERT "chrdev_exit completed\n");
}


module_init(chrdev_init);
module_exit(chrdev_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("joe");
MODULE_DESCRIPTION("chrdev test");
