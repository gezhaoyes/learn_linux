//#include <asm/mach/map.h>
#include "linux/platform_device.h"
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/types.h>

#define LEDON 1
#define LEDOFF 0

static int led_open(struct inode *inode, struct file *filp);
static ssize_t led_read(struct file *filp, char __user *buf, size_t cnt,
                        loff_t *offt);
static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt,
                         loff_t *offt);
static int led_release(struct inode *inode, struct file *filp);
static int led_probe(struct platform_device *dev);
static int led_remove(struct platform_device *dev);
struct gpioled_dev
{
    dev_t devid;         //
    struct cdev cdev;    // cdev
    struct class *class; //
    struct device *devicename;
    int major;
    int monor;
    struct device_node *nd;
    int led_gpio;
    struct mutex lock;
};

static struct file_operations gpioled_file_operations = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_release,
};

static const struct of_device_id led_of_match[] = {
    {.compatible = "gezhao-gpioled"},
    {},
};
MODULE_DEVICE_TABLE(of, led_of_match);

static struct platform_driver led_driver = {
    .driver =
        {
            .name = "gpioled",
            .of_match_table = led_of_match,
        },
    .probe = led_probe,
    .remove = led_remove,
};

static struct gpioled_dev gpioled = {0}; /* led  */
static int __init led_driver_init(void)
{
    int ret;
    ret = platform_driver_register(&led_driver);
    if (ret < 0)
    {
        printk("platform_driver_register errno=%d!\n", ret);
    }
    return ret;
}

static void __exit led_driver_exit(void)
{
    platform_driver_unregister(&led_driver);
}

static int led_probe(struct platform_device *dev)
{
    int ret;
    /*get led node*/
    gpioled.nd = of_find_node_by_path("/gpioled");
    if (gpioled.nd == NULL)
    {
        printk("gpioled node can not found!\n");
    }
    else
    {
        printk("gpioled node has been found!\n");
    }

    /*get gpio property into gpio index*/
    gpioled.led_gpio = of_get_named_gpio(gpioled.nd, "led-gpio", 0);
    if (gpioled.led_gpio < 0)
    {
        printk("cant get led-gpio property!,errno=%d\n", gpioled.led_gpio);
        return gpioled.led_gpio;
    }
    printk("led index=%d\n", gpioled.led_gpio);

    if ((ret = gpio_request(gpioled.led_gpio, "led")) < 0)
    {
        printk("gpio request failed! errno=%d\n", ret);
        return ret;
    }

    /*set GPIO1_IO03 output and high*/
    if ((ret = gpio_direction_output(gpioled.led_gpio, 1)) < 0)
    {
        printk("gpio set failed! errno=%d\n", ret);
        return ret;
    }

    /*register chr dev */
    if (gpioled.major) // if define major
    {
        gpioled.devid = MKDEV(gpioled.major, 0);
        if ((ret = register_chrdev_region(gpioled.devid, 1, "led")) < 0)
        {
            printk("fail to register chrdev region, err=%d\n", ret);
            return ret;
        }
    }
    else
    {
        if ((ret = alloc_chrdev_region(&gpioled.devid, 0, 1, "led")) < 0)
        {
            printk("fail to alloc chrdev err=%d\n", ret);
            return ret;
        }
    }

    /*add cdev*/
    gpioled.cdev.owner = THIS_MODULE;
    cdev_init(&gpioled.cdev, &gpioled_file_operations);
    if ((ret = cdev_add(&gpioled.cdev, gpioled.devid, 1)) < 0)
    {
        printk("fail to add gpioled cdev,err=%d\n", ret);
        return ret;
    }

    /*create class*/
    gpioled.class = class_create(THIS_MODULE, "led_class");
    if (IS_ERR(gpioled.class))
    {
        printk("fail to create class,err=%ld\n", PTR_ERR(gpioled.class));
        return PTR_ERR(gpioled.class);
    }

    /*create device*/
    gpioled.devicename =
        device_create(gpioled.class, NULL, gpioled.devid, NULL, "led_device");
    if (IS_ERR(gpioled.devicename))
    {
        printk("fail to add gpioled cdev,err=%ld\n",
               PTR_ERR(gpioled.devicename));
        return PTR_ERR(gpioled.devicename);
    }

    mutex_init(&gpioled.lock);
    return 0;
}

static int led_remove(struct platform_device *dev)
{
    gpio_free(gpioled.led_gpio);
    /*delete device and class*/

    device_destroy(gpioled.class, gpioled.devid);
    class_destroy(gpioled.class);

    cdev_del(&gpioled.cdev);
    unregister_chrdev_region(gpioled.devid, 1);
    return 0;
}

static int led_open(struct inode *inode, struct file *filp)
{
    mutex_lock(&gpioled.lock);
    filp->private_data = &gpioled;
    return 0;
}

static ssize_t led_read(struct file *filp, char __user *buf, size_t cnt,
                        loff_t *offt)
{
    return 0;
}

static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt,
                         loff_t *offt)
{
    unsigned char ledstate;
    unsigned char databuf[1] = {0};
    struct gpioled_dev *dev = filp->private_data;
    if (copy_from_user(databuf, buf, cnt) != 0)
    {
        printk("cppy from user failed!\n");
        return -EFAULT;
    }

    ledstate = databuf[0];

    if (ledstate == LEDON)
    {
        gpio_set_value(dev->led_gpio, 0);
    }
    else
    {
        gpio_set_value(dev->led_gpio, 1);
    }
    return 0;
}

static int led_release(struct inode *inode, struct file *filp)
{
    mutex_unlock(&gpioled.lock);
    return 0;
}
module_init(led_driver_init);
module_exit(led_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("gezhao");