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

//NUMETO Lab USBGPIO8
struct termios options;
int fd;


// setup /dev/ttuACM0
void setup_USBGPIO8(void){

    fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd < 0)
    {
        printf("Error opening serial port\n");
        exit(1);
    }

    bzero(&options, sizeof(options));
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD | IGNPAR;
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &options);
}

bool set_gpio (uint8_t gpio_num){

   char numeto_command[80];
   bool status = false;

   if (gpio_num >= 0 && gpio_num <=7) {

        sprintf(numeto_command, "gpio set %d\r\n", gpio_num);
        if (write(fd, numeto_command, strlen(numeto_command)) < strlen(numeto_command))
        {
            printf("Write error - %s \n", strerror(errno));
            exit (1);
        }
        status = true;
    }

    return status;
}

bool clear_gpio (uint8_t gpio_num){

   char numeto_command[80];
   bool status = false;

   if (gpio_num >= 0 && gpio_num <=7) {

        sprintf(numeto_command, "gpio clear %d\r\n", gpio_num);
        if (write(fd, numeto_command, strlen(numeto_command)) < strlen(numeto_command))
        {
            printf("Write error - %s \n", strerror(errno));
            exit (1);
        }
        status = true;
    }

    return status;
}

bool read_gpio(uint8_t gpio_num){

   char numeto_command[80];
   bool status = false;

   if (gpio_num >= 0 && gpio_num <=7) {

        sprintf(numeto_command, "gpio read %d\r\n", gpio_num);
        if (write(fd, numeto_command, strlen(numeto_command)) < strlen(numeto_command))
        {
            printf("Write error - %s \n", strerror(errno));
            exit (1);
        }
        status = true;
    }

    return status;
}

bool gpio_mask_bit(uint8_t gpio_num){

    char numeto_command[80];
    bool status = false;

    uint8_t gpio_mask_hex = ~(1<<gpio_num);

    sprintf(numeto_command, "gpio iomask %x\r\n", gpio_mask_hex);
    if (write(fd, numeto_command, strlen(numeto_command)) < strlen(numeto_command))
    {
            printf("Write error - %s \n", strerror(errno));
            exit (1);
    }
        status = true;


    return status;
}

bool gpio_unmask_bit(uint8_t gpio_num){

    char numeto_command[80];
    bool status = false;

    uint8_t gpio_mask_hex = 1<<gpio_num;

    sprintf(numeto_command, "gpio iomask %x\r\n", gpio_mask_hex);
    if (write(fd, numeto_command, strlen(numeto_command)) < strlen(numeto_command))
    {
            printf("Write error - %s \n", strerror(errno));
            exit (1);
    }
        status = true;


    return status;
}

bool iodir_gpio_output(uint8_t gpio_num){

 char numeto_command[80];
   bool status = false;

   if (gpio_num >= 0 && gpio_num <=7) {

        uint8_t gpio_iodir_hex = 1<<gpio_num;   //output direction bit is 1

        gpio_unmask_bit(gpio_num);
        sprintf(numeto_command, "gpio iodir  %x\r\n", gpio_iodir_hex);
        if (write(fd, numeto_command, strlen(numeto_command)) < strlen(numeto_command))
        {
            printf("Write error - %s \n", strerror(errno));
            exit (1);
        }

        status = true;
    }

    return status;
}

bool iodir_gpio_input(uint8_t gpio_num){

   char numeto_command[80];
   bool status = false;

   if (gpio_num >= 0 && gpio_num <=7) {

        uint8_t gpio_iodir_hex = ~(1<<gpio_num); //same as mask becuase the input bit is 0

        gpio_unmask_bit(gpio_num);

        sprintf(numeto_command, "gpio iodir  %x\r\n", gpio_iodir_hex);
        if (write(fd, numeto_command, strlen(numeto_command)) < strlen(numeto_command))
        {
            printf("Write error - %s \n", strerror(errno));
            exit (1);
        }

        status = true;
    }

    return status;
}

bool gpio_unmask_bits(uint8_t mask_pattern){

    char numeto_command[80];
    bool status = false;

    uint8_t gpio_mask_hex = mask_pattern;

    sprintf(numeto_command, "gpio iomask %x\r\n", gpio_mask_hex);
    if (write(fd, numeto_command, strlen(numeto_command)) < strlen(numeto_command))
    {
            printf("Write error - %s \n", strerror(errno));
            exit (1);
    }
        status = true;


    return status;
}

bool gpio_mask_bits(uint8_t mask_pattern){

    char numeto_command[80];
    bool status = false;

    uint8_t gpio_mask_hex = ~mask_pattern;

    sprintf(numeto_command, "gpio iomask %x\r\n", gpio_mask_hex);
    if (write(fd, numeto_command, strlen(numeto_command)) < strlen(numeto_command))
    {
            printf("Write error - %s \n", strerror(errno));
            exit (1);
    }
        status = true;


    return status;
}
