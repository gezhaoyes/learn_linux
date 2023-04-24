//#include <asm/mach/map.h>
#include "linux/platform_device.h"
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/types.h>

#define BEEPON 1
#define BEEPOFF 0
#define BEEP_MINOR 20
#define BEEP_NAME "gpiobeep"

static int beep_open(struct inode *inode, struct file *filp);
static ssize_t beep_read(struct file *filp, char __user *buf, size_t cnt,
                         loff_t *offt);
static ssize_t beep_write(struct file *filp, const char __user *buf, size_t cnt,
                          loff_t *offt);
static int beep_release(struct inode *inode, struct file *filp);
static int beep_probe(struct platform_device *dev);
static int beep_remove(struct platform_device *dev);
struct gpiobeep_dev
{
    dev_t devid;         //
    struct cdev cdev;    // cdev
    struct class *class; //
    struct device *devicename;
    int major;
    int monor;
    struct device_node *nd;
    int gpiobeep_desc;
    struct mutex lock;
};

static struct file_operations gpiobeep_file_operations = {
    .owner = THIS_MODULE,
    .open = beep_open,
    .read = beep_read,
    .write = beep_write,
    .release = beep_release,
};

static const struct of_device_id beep_of_match[] = {
    {.compatible = "gezhao-gpiobeep"},
    {},
};
MODULE_DEVICE_TABLE(of, beep_of_match);

static struct platform_driver gpiobeep_driver = {
    .driver =
        {
            .name = "gpiobeep",
            .of_match_table = beep_of_match,
        },
    .probe = beep_probe,
    .remove = beep_remove,
};
static struct miscdevice gpiobeep_miscdev = {
    .minor = BEEP_MINOR,
    .name = BEEP_NAME,
    .fops = &gpiobeep_file_operations,
};

static struct gpiobeep_dev gpiobeep = {0};
static int __init beep_driver_init(void)
{
    int ret;
    ret = platform_driver_register(&gpiobeep_driver);
    if (ret < 0)
    {
        printk("misc_register errno=%d!\n", ret);
    }
    return ret;
}

static void __exit beep_driver_exit(void)
{
    platform_driver_unregister(&gpiobeep_driver);
}

static int beep_probe(struct platform_device *pdev)
{
    int ret;
#if 0
    /*get led node*/
    gpiobeep.nd = of_find_node_by_path("/gpioled");
    if (gpiobeep.nd == NULL)
    {
        printk("gpioled node can not found!\n");
    }
    else
    {
        printk("gpioled node has been found!\n");
    }

    /*get gpio property into gpio index*/
    gpiobeep.gpiobeep_desc = of_get_named_gpio(gpiobeep.nd, "led-gpio", 0);
    if (gpiobeep.gpiobeep_desc < 0)
    {
        printk("cant get led-gpio property!,errno=%d\n",
               gpiobeep.gpiobeep_desc);
        return gpiobeep.gpiobeep_desc;
    }
    printk("led index=%d\n", gpiobeep.gpiobeep_desc);

    if ((ret = gpio_request(gpiobeep.gpiobeep_desc, "led")) < 0)
    {
        printk("gpio request failed! errno=%d\n", ret);
        return ret;
    }

    /*set beep output and high*/
    if ((ret = gpio_direction_output(gpiobeep.gpiobeep_desc, 1)) < 0)
    {
        printk("gpio set failed! errno=%d\n", ret);
        return ret;
    }

#endif
    struct device *dev = &pdev->dev;
    if (!(dev->of_node))
    {
        return -ENODEV;
    }
    else
    {
        /*get gpio property into gpio index*/
        gpiobeep.gpiobeep_desc =
            of_get_named_gpio(dev->of_node, "beep-gpio", 0);
        if (gpiobeep.gpiobeep_desc < 0)
        {
            printk("cant get beep-gpio property!,errno=%d\n",
                   gpiobeep.gpiobeep_desc);
            return gpiobeep.gpiobeep_desc;
        }

        if ((ret = gpio_request(gpiobeep.gpiobeep_desc, "beep")) < 0)
        {
            printk("gpio request failed! errno=%d\n", ret);
            return ret;
        }

        /*set beep output and close*/
        if ((ret = gpio_direction_output(gpiobeep.gpiobeep_desc, 1)) < 0)
        {
            printk("gpio set failed! errno=%d\n", ret);
            return ret;
        }
    }

    ret = misc_register(&gpiobeep_miscdev);
    if (ret < 0)
    {
        printk("misc_register errno=%d!\n", ret);
        return ret;
    }

    mutex_init(&gpiobeep.lock);
    return 0;
}

static int beep_remove(struct platform_device *pdev)
{
    gpio_free(gpiobeep.gpiobeep_desc);
    misc_deregister(&gpiobeep_miscdev);
    return 0;
}

static int beep_open(struct inode *inode, struct file *filp)
{
    mutex_lock(&gpiobeep.lock);
    filp->private_data = &gpiobeep;
    return 0;
}

static ssize_t beep_read(struct file *filp, char __user *buf, size_t cnt,
                         loff_t *offt)
{
    return 0;
}

static ssize_t beep_write(struct file *filp, const char __user *buf, size_t cnt,
                          loff_t *offt)
{
    unsigned char ledstate;
    unsigned char databuf[1] = {0};
    struct gpiobeep_dev *dev = filp->private_data;
    if (copy_from_user(databuf, buf, cnt) != 0)
    {
        printk("cppy from user failed!\n");
        return -EFAULT;
    }

    ledstate = databuf[0];

    if (ledstate == BEEPON)
    {
        gpio_set_value(dev->gpiobeep_desc, 0);
    }
    else
    {
        gpio_set_value(dev->gpiobeep_desc, 1);
    }
    return 0;
}

static int beep_release(struct inode *inode, struct file *filp)
{
    mutex_unlock(&gpiobeep.lock);
    return 0;
}
module_init(beep_driver_init);
module_exit(beep_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("gezhao");