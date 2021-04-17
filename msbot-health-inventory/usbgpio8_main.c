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
#include <stdbool.h>
#include "usbgpio8.h"

void read_gpio_input(void) {
    uint8_t result = 0xff;

    if (read_gpio(0, &result))
      printf("reading gpio-0 succeeded %d \n", result);

    if(read_gpio(1, &result))
        printf("reading gpio-1 succeeded %d \n", result);

    if(read_gpio(2, &result))
        printf("reading gpio-2 succeeded %d \n", result);

    if(read_gpio(3, &result))
        printf("reading gpio-3 succeeded %d \n", result);
}



int main(int args,char** argv)
{

    bool status = false;
// setup NUMATO USBGPIO8
    status=setup_USBGPIO8();

    if (status)
      printf("usbgpio setup  succeeded \n");

    gpio_unmask_all();
#if 1
    status = false;
    //Set GPIO 0-3 input
    iodir_gpio_input(0);
    if (status)
      printf("iodir_gpio_input 0 setup  succeeded \n");

    status = false;
    status = iodir_gpio_input(1);
    if (status)
      printf("iodir_gpio_input 1 setup  succeeded \n");

    status = false;
    status = iodir_gpio_input(2);
    if (status)
      printf("iodir_gpio_input 2 setup  succeeded \n");

    status = false;
    status = iodir_gpio_input(3);
    if (status)
      printf("iodir_gpio_input 3 setup  succeeded \n");

     //Set GPIO 4-7 as output
     status = false;
    status = iodir_gpio_output(4);
    if (status)
      printf("iodir_gpio_output 4 setup  succeeded \n");

    status = false;
    status = iodir_gpio_output(5);
    if (status)
      printf("iodir_gpio_output 5 setup  succeeded \n");

    status = false;
    status = iodir_gpio_output(6);
    if (status)
      printf("iodir_gpio_output 6 setup  succeeded \n");
#endif
    status = false;
    status = iodir_gpio_output(7);
    if (status)
      printf("iodir_gpio_output 7 setup  succeeded \n");

    bool toggle = true;
    write_gpio_output(toggle);
    sleep(3);

    while(1) {

        read_gpio_input();

     //   toggle = !toggle;

        printf("Sleeping for 6 seconds toggle=%d\n", toggle);
        sleep(6);

    }

    remove_USBGPIO8();

    return 0;
}
