#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>

#include <linux/gpio.h>
#include <linux/kthread.h>
#include <linux/input.h>
#include <linux/delay.h>

#define NB_COLUMN                 2
#define NB_LINE                   2

#define GPIO_OUT_0                4
#define GPIO_OUT_1                17
#define GPIO_IN_0                 27
#define GPIO_IN_1                 22

struct input_dev *input;
static struct task_struct *checkGpioThread;

int gpioOut[] = {GPIO_OUT_0, GPIO_OUT_1};
int gpioIn[]  = {GPIO_IN_0, GPIO_IN_1};

static unsigned short keyboardKeymap[] = {
    KEY_A, KEY_B,
    KEY_C, KEY_D
};

int checkGpioThread_fn(void *data)
{
    int col,lin,i;
    short key;

    while (!kthread_should_stop())
    {
        for(col=0 ; col<NB_COLUMN ; col++)
        {
            // On allume la gpio de la colonne courante et on eteint les autres
            for(i=0 ; i<NB_COLUMN ; i++)
            {
                char state = (i==col ? 1:0);
                gpio_direction_output(gpioOut[i], state);
            }

            // On lit chaque ligne
            for(lin=0 ; lin<NB_LINE ; lin++)
            {
                msleep(100);

                if(gpio_get_value(gpioIn[lin]))
                {
                    key = keyboardKeymap[lin + (col*NB_LINE)];
                    // Touche appuyee
                    input_report_key(input, key, 1);
                    msleep(50);
                    // Touche relachee
                    input_report_key(input, key, 0);
                }
            }
        }
    }

    return 0;
}

int clavier_init(void)
{
    int err, i;
    char string[20];

    // Initialisation des GPIO

    for(i=0 ; i<NB_COLUMN ; i++)
    {
        sprintf(string, "out_%d", i);
        gpio_request(gpioOut[i], string);
    }

    for(i=0 ; i<NB_COLUMN ; i++)
    {
        sprintf(string, "in_%d", i);
        gpio_request(gpioIn[i], string);
        gpio_direction_input(gpioIn[i]);
    }

    // Enregistrement du clavier aupres du noyau
    input = input_allocate_device();
    if (!input) {
        err = -ENOMEM;
    }

    input->name = "Awesome GPIO keyboard";
    input->keycode = keyboardKeymap;
    input->keycodesize = sizeof(unsigned short);
    input->keycodemax = NB_COLUMN * NB_LINE;
    input->evbit[0] = BIT(EV_KEY);

    for (i = 0; i < NB_COLUMN * NB_LINE; i++)
        __set_bit(keyboardKeymap[i], input->keybit);

    err = input_register_device(input);
    if (err)
    {
        printk(KERN_CRIT "input_register_device failed");
        goto end;
    }

    checkGpioThread = kthread_create(checkGpioThread_fn, NULL, "checkGpioThread");
    if(!checkGpioThread)
    {
        printk(KERN_CRIT "kthread_create failed");
        goto end;
    }

    wake_up_process(checkGpioThread);

    printk(KERN_NOTICE "Keyboard driver initialised\n");

end:
    return 0;
}
void clavier_exit(void)
{
    kthread_stop(checkGpioThread);
    input_unregister_device(input);
    input_free_device(input);

    printk(KERN_NOTICE "Keyboard driver exit\n");

    return;
}

module_init(clavier_init);
module_exit(clavier_exit);

MODULE_LICENSE("GPL");
