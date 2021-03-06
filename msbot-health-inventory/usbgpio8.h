#ifndef _USBGPIO8_H_
#define _USBGPIO8_H_

#ifdef __cplusplus
extern "C"
{
#endif // ifdef __cplusplus

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

// setup /dev/ttuACM0
bool setup_USBGPIO8(void);

bool set_gpio (uint8_t gpio_num);

bool clear_gpio (uint8_t gpio_num);

bool read_gpio(uint8_t gpio_num, uint8_t *result);

bool gpio_mask_bit(uint8_t gpio_num);

bool gpio_unmask_bit(uint8_t gpio_num);

bool gpio_mask_all(void);

bool gpio_unmask_all(void);

bool iodir_gpio_output(uint8_t gpio_num);

bool iodir_gpio_input(uint8_t gpio_num);

bool gpio_unmask_bits(uint8_t mask_pattern);

bool gpio_mask_bits(uint8_t mask_pattern);

bool remove_USBGPIO8(void);

void write_gpio_output(bool sw_on);

void read_gpio_input(void);

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif // _USBGPIO8_H_
