//#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/types.h>

#define KEYON 1
#define KEYOFF 0
#define KEY_MINOR 20
#define KEY_NAME "gpiokey"

static int key_probe(struct platform_device *dev);
static int key_remove(struct platform_device *dev);
struct gpiokey_dev
{
    int gpiokey_desc;
    struct input_dev *gpiokey_input_dev;
    unsigned int virq;
    struct timer_list gpiokey_timer;
};

static const struct of_device_id key_of_match[] = {
    {.compatible = "gezhao-gpiokey"},
    {},
};
MODULE_DEVICE_TABLE(of, key_of_match);

static struct platform_driver gpiokey_driver = {
    .driver =
        {
            .name = "gpiokey",
            .of_match_table = key_of_match,
        },
    .probe = key_probe,
    .remove = key_remove,
};

static int __init key_driver_init(void)
{
    int ret;
    ret = platform_driver_register(&gpiokey_driver);
    if (ret < 0)
    {
        printk("platform_driver_register errno=%d!\n", ret);
    }
    return ret;
}

static void __exit key_driver_exit(void)
{
    platform_driver_unregister(&gpiokey_driver);
}

irqreturn_t goiokey0_irq_hander(int irq, void *dev_id)
{
    struct device *dev = (struct device *)dev_id;
    struct gpiokey_dev *gpiokey_pri_data =
        (struct gpiokey_dev *)dev->platform_data;
    mod_timer(&gpiokey_pri_data->gpiokey_timer, jiffies + msecs_to_jiffies(10));
    return IRQ_RETVAL(IRQ_HANDLED);
}

void goiokey0_timer_hander(unsigned long arg)
{
    int value;
    struct gpiokey_dev *gpiokey_pri_data = (struct gpiokey_dev *)arg;
    value = gpio_get_value(gpiokey_pri_data->gpiokey_desc);
    input_report_key(gpiokey_pri_data->gpiokey_input_dev, KEY_0, !value);
    input_sync(gpiokey_pri_data->gpiokey_input_dev);
}

static int key_probe(struct platform_device *pdev)
{
    int ret;
    struct device *dev = &pdev->dev;
    if (!(dev->of_node))
    {
        return -ENODEV;
    }
    else
    {
        struct input_dev *gpiokey_inputdev;
        struct gpiokey_dev *gpiokey_pri_data;
        if ((gpiokey_inputdev = input_allocate_device()) == NULL)
        {
            return -EFAULT;
        }
        if ((gpiokey_pri_data = devm_kzalloc(dev, sizeof(struct gpiokey_dev),
                                             GFP_KERNEL)) == NULL)
        {
            return -EFAULT;
        }

        gpiokey_inputdev->name = pdev->name;
        __set_bit(EV_KEY, gpiokey_inputdev->evbit);
        __set_bit(EV_REP, gpiokey_inputdev->evbit);
        __set_bit(KEY_0, gpiokey_inputdev->keybit);

        ret = input_register_device(gpiokey_inputdev);
        if (ret != 0)
        {
            return ret;
        }
        gpiokey_pri_data->gpiokey_input_dev = gpiokey_inputdev;
        dev->platform_data = gpiokey_pri_data;
        /*get gpio property into gpio index*/
        gpiokey_pri_data->gpiokey_desc =
            of_get_named_gpio(dev->of_node, "key-gpio", 0);
        if (gpiokey_pri_data->gpiokey_desc < 0)
        {
            printk("cant get key-gpio property!,errno=%d\n",
                   gpiokey_pri_data->gpiokey_desc);
            return gpiokey_pri_data->gpiokey_desc;
        }

        if ((ret = gpio_request(gpiokey_pri_data->gpiokey_desc,
                                gpiokey_inputdev->name)) < 0)
        {
            printk("gpio request failed! errno=%d\n", ret);
            return ret;
        }

        /*set key output and close*/
        if ((ret = gpio_direction_input(gpiokey_pri_data->gpiokey_desc)) < 0)
        {
            printk("gpio set failed! errno=%d\n", ret);
            return ret;
        }

        if ((gpiokey_pri_data->virq = irq_of_parse_and_map(dev->of_node, 0)) ==
            0)
        {
            return -EBADR;
        }
        if ((ret = request_irq(gpiokey_pri_data->virq, goiokey0_irq_hander,
                               IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
                               "gpiokey0", dev)) != 0)
        {
            return ret;
        }

        init_timer(&gpiokey_pri_data->gpiokey_timer);
        (gpiokey_pri_data->gpiokey_timer).function = goiokey0_timer_hander;
        gpiokey_pri_data->gpiokey_timer.data = (unsigned long)gpiokey_pri_data;
    }
    return 0;
}

static int key_remove(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct gpiokey_dev *gpiokey_pri_data =
        (struct gpiokey_dev *)dev->platform_data;
    free_irq(gpiokey_pri_data->virq, pdev);
    gpio_free(gpiokey_pri_data->gpiokey_desc);
    del_timer_sync(&gpiokey_pri_data->gpiokey_timer);
    input_unregister_device(gpiokey_pri_data->gpiokey_input_dev);
    return 0;
}

module_init(key_driver_init);
module_exit(key_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("gezhao");