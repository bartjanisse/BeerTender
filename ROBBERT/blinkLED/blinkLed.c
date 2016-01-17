
#include <linux/init.h>
#include <linux/module.h>
#include <asm/io.h>
#include <mach/platform.h>
#include <linux/timer.h>
#include <linux/device.h>
#include <linux/err.h>
#include "led_blink.h"
 
struct GpioRegisters *s_pGpioRegisters;
static struct timer_list s_BlinkTimer;
static int s_BlinkPeriod = 1000;
static const int LedGpioPin = 14;
 
static void SetGPIOFunction(int GPIO, int functionCode)
{
    int registerIndex = GPIO / 10;
    int bit = (GPIO % 10) * 3;
 
    unsigned oldValue = s_pGpioRegisters->GPFSEL[registerIndex];
    unsigned mask = 0b111 << bit;
    printk("Changing function of GPIO%d from %x to %x\n", GPIO, (oldValue >> bit) & 0b111, functionCode);
    s_pGpioRegisters->GPFSEL[registerIndex] = (oldValue & ~mask) | ((functionCode << bit) & mask);
}
 
static void SetGPIOOutputValue(int GPIO, bool outputValue)
{
    if (outputValue)
        s_pGpioRegisters->GPSET[GPIO / 32] = (1 << (GPIO % 32));
    else
        s_pGpioRegisters->GPCLR[GPIO / 32] = (1 << (GPIO % 32));
}
 
 
static void BlinkTimerHandler(unsigned long unused)
{
    static bool on = false;
    on = !on;
    SetGPIOOutputValue(LedGpioPin, on);
    mod_timer(&s_BlinkTimer, jiffies + msecs_to_jiffies(s_BlinkPeriod));
}
 
static ssize_t set_period_callback(struct device* dev,struct device_attribute* attr,
                    const char* buf, size_t count)
{
    long period_value = 0;
    if (kstrtol(buf, 10, &period_value) < 0)
        return -EINVAL;
    if (period_value < 10)   //Safety check
        return -EINVAL;
 
    s_BlinkPeriod = period_value;
    return count;
}
 
static ssize_t show_period_callback(struct device *dev, struct device_attribute *attr,
                        char *buf)
{
        return sprintf(buf, "Period = %x\n", s_BlinkPeriod);
}
 
static DEVICE_ATTR(period, S_IWUSR | S_IWGRP | S_IWOTH, show_period_callback\
                            , set_period_callback);
 
tatic struct attribute *led_attr[] = {
    &dev_attr_period.attr,
    NULL
}; 
 
static struct attribute_group BlinkLed_attr = {
    .attrs = led_attr;
};
 
static int LedBlinkDev_probe(struct platform_device *dev)
{
    int result;
    struct gpio_number *p_data = (struct gpio_number*)dev->dev.platform_data;
     
    LedGpioPin = p_data->gpio_num;
    s_pGpioRegisters = (struct GpioRegisters *)__io_address(GPIO_BASE);
    SetGPIOFunction(LedGpioPin, 0b001); //Configure the pin as output
     
    setup_timer(&s_BlinkTimer, BlinkTimerHandler, 0);
    result = mod_timer(&s_BlinkTimer, jiffies + msecs_to_jiffies(s_BlinkPeriod));
    BUG_ON(result < 0);
    ret = sysfs_create_group(&dev->dev.kobj, &BlinkLed_attr);
     
    return 0;
}
 
/* Driver file */
static struct platform_driver LedBlink_driver = {
    .driver = {
        .name = "Led_Blink",
        .owner = THIS_MODULE,
    },
    .probe = LedBlinkDev_probe,
    .remove = LedBlinkDev_remove,
};
 
 
static int __init LedBlinkDev_init(void)
{
 
    printk(KERN_ALERT "Regstrating Led Blink Device Driver\n");
 
    platform_driver_register(&drivertest_driver);
    return 0;
}
 
static void __exit LedBlinkDev_exit(void)
{
 
    printk(KERN_ALERT "Un-Regstrating Led Blink Device Driver\n");
    platform_driver_unregister(&drivertest_driver);
    SetGPIOFunction(LedGpioPin, 0); //Configure the pin as input
    del_timer(&s_BlinkTimer);
}
 
module_init(LedBlinkDev_init);
module_exit(LedBlinkDev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ROBBAX");
MODULE_DESCRIPTION("LED blinking module");

