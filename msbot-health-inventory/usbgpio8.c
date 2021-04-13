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

int set_interface_attribs(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

// setup /dev/ttuACM0
bool setup_USBGPIO8(void){

    bool status = false;
    char *portname = "/dev/ttyACM0";

    //fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NDELAY);

    fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        printf("Error opening %s: %s\n", portname, strerror(errno));
        return -1;
    }
    /*baudrate 115200, 8 bits, no parity, 1 stop bit */
    set_interface_attribs(fd, B115200);

    return status;
}

bool remove_USBGPIO8(void){

    bool status = false;

    if (fd > 0)
    {
        close(fd);
        printf("close /dev/ttyACM0\n");
    }

    status = true;

}

bool set_gpio (uint8_t gpio_num){

   char numeto_command[80];
   bool status = false;

   if (gpio_num >= 0 && gpio_num <=7) {

        sprintf(numeto_command, "gpio set %d\r", gpio_num);
        printf("numeto command %s \n", numeto_command);
        if (write(fd, numeto_command, strlen(numeto_command)) != strlen(numeto_command))
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

        sprintf(numeto_command, "gpio clear %d\r", gpio_num);
        printf("numeto command %s \n", numeto_command);
        if (write(fd, numeto_command, strlen(numeto_command)) != strlen(numeto_command))
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
   int res = 0;
   char buf[255];
   int temp;

   printf("read_gpio entered \n");

   if (gpio_num >= 0 && gpio_num <=7) {

        sprintf(numeto_command, "gpio read %d\r", gpio_num);
        printf("numeto command %s \n", numeto_command);

        if (write(fd, numeto_command, strlen(numeto_command)) != strlen(numeto_command))
        {
            printf("Write error - %s \n", strerror(errno));
            exit (1);
        }

        printf("read_gpio write() succeeded \n");
        if ((res = read(fd,buf,255)) > 0) {
            temp = atoi(buf);
            printf("result len = %d GPIO-%d = %d \n", res, gpio_num, temp);
            status = true;
        }

        if (status)
            printf("read_gpio read() succeeded buf=%s %d\n", buf, temp );
        else
            printf("read_gpio read() FAILED \n");
    }

    return status;
}

bool gpio_mask_bit(uint8_t gpio_num){

    char numeto_command[80];
    bool status = false;

    uint8_t gpio_mask_hex = ~(1<<gpio_num);

    sprintf(numeto_command, "gpio iomask %x\r", gpio_mask_hex);
    printf("numeto command %s \n", numeto_command);
    if (write(fd, numeto_command, strlen(numeto_command)) != strlen(numeto_command))
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

    sprintf(numeto_command, "gpio iomask %x\r", gpio_mask_hex);
    printf("numeto command %s \n", numeto_command);
    if (write(fd, numeto_command, strlen(numeto_command)) != strlen(numeto_command))
    {
            printf("Write error - %s \n", strerror(errno));
            exit (1);
    }
        status = true;


    return status;
}

bool gpio_unmask_all(void){

    char numeto_command[80];
    bool status = false;

    uint8_t gpio_mask_hex = 0xff;

    sprintf(numeto_command, "gpio iomask %x\r", gpio_mask_hex);
    printf("numeto command %s \n", numeto_command);
    if (write(fd, numeto_command, strlen(numeto_command)) != strlen(numeto_command))
    {
            printf("Write error - %s \n", strerror(errno));
            exit (1);
    }
        status = true;

    return status;
}

bool gpio_mask_all(void){

    char numeto_command[80];
    bool status = false;

    uint8_t gpio_mask_hex = 0x0;

    sprintf(numeto_command, "gpio iomask %x\r", gpio_mask_hex);
    printf("numeto command %s \n", numeto_command);
    if (write(fd, numeto_command, strlen(numeto_command)) != strlen(numeto_command))
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

        uint8_t gpio_iodir_hex = ~(1<<gpio_num);   //output direction bit is 1

        sprintf(numeto_command, "gpio iodir %x\r", gpio_iodir_hex);
        printf("numeto command %s \n", numeto_command);

        if (write(fd, numeto_command, strlen(numeto_command)) != strlen(numeto_command))
        {
            printf("Write error - %s \n", strerror(errno));
            exit (1);
        }

        printf("iodir_gpio_output succeeded \n");
        status = true;
    }

    return status;
}



bool iodir_gpio_input(uint8_t gpio_num){

   char numeto_command[80];
   bool status = false;

   if (gpio_num >= 0 && gpio_num <=7) {

        uint8_t gpio_iodir_hex = 1<<gpio_num; //same as mask becuase the input bit is 0

        gpio_unmask_bit(gpio_num);

        sprintf(numeto_command, "gpio iodir %d\r", gpio_iodir_hex);
        printf("numeto command %s \n", numeto_command);
        if (write(fd, numeto_command, strlen(numeto_command)) != strlen(numeto_command))
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

    sprintf(numeto_command, "gpio iomask %x\r", gpio_mask_hex);
    printf("numeto command %s \n", numeto_command);
    if (write(fd, numeto_command, strlen(numeto_command)) != strlen(numeto_command))
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

    sprintf(numeto_command, "gpio iomask %x\r", gpio_mask_hex);
    printf("numeto command %s \n", numeto_command);
    if (write(fd, numeto_command, strlen(numeto_command)) != strlen(numeto_command))
    {
            printf("Write error - %s \n", strerror(errno));
            exit (1);
    }
        status = true;


    return status;
}
