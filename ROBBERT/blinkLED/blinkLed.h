
#ifndef _LEDBLINK_H_
#define _LEDBLINK_H_ 
 
 
 
struct GpioRegisters
{
    uint32_t GPFSEL[6];
    uint32_t Reserved1;
    uint32_t GPSET[2];
    uint32_t Reserved2;
    uint32_t GPCLR[2];
};
 
struct gpio_number {
    int gpio_num;
};
 
 
#endif /* _LEDBLINK_H_ */
