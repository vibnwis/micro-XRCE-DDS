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
    struct termios tty;  /* Create the structure                          */

    if (tcgetattr(fd, &tty) < 0) {  /* Get the current attributes of the Serial port */
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    /* Setting the Baud rate */
    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    /* 8N1 Mode */
    tty.c_cflag |= (CLOCAL | CREAD);    /*  Enable receiver, ignore modem controls */
    tty.c_cflag &= ~CSIZE;      /* Clears the mask for setting the data size             */
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF | IXANY);  /* Disable XON/XOFF flow control both i/p and o/p */
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN); /* Non Cannonical mode                            */
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;  /* Read at least 1 characters */
    tty.c_cc[VTIME] = 1; /* Wait 1ms , 0 = indefinitely */

    if (tcsetattr(fd, TCSANOW, &tty) != 0) { /* Set the attributes to the termios structure*/
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

    fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);  /* ttyUSB0 is the FT232 based USB2SERIAL Converter   */
			   					/* O_RDWR   - Read/Write access to serial port       */
								/* O_NOCTTY - No terminal will control the process   */
								/* Open in blocking mode,read will wait              */
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

bool read_gpio(uint8_t gpio_num, uint8_t *result){

   char numeto_command[80];
   bool status = false;
   int len = 0;
   char buf[255];
   uint8_t temp;

   tcflush(fd, TCIFLUSH);   /* Discards old data in the rx buffer            */
   printf("read_gpio  Discards old data in the rx buffer \n");

   if (gpio_num >= 0 && gpio_num <=7) {

        sprintf(numeto_command, "gpio read %d\r", gpio_num);
        printf(" numeto command %s \n", numeto_command);

        if (write(fd, numeto_command, strlen(numeto_command)) != strlen(numeto_command))
        {
            printf("Write error - %s \n", strerror(errno));
            exit (1);
        }

        printf("read_gpio write() succeeded \n");
        if ((len = read(fd,buf,255)) > 0) {
            buf[len] = '\0';
            temp = buf[13] - '0';
            *result = temp;
            printf("result len = %d GPIO-%d = %d \n", len, gpio_num, temp);
            status = true;
        }
#if 0
        if (status){

            printf("read_gpio read() succeeded buf=%s %d\n", buf, temp );
        }
        else
            printf("read_gpio read() FAILED \n");
#endif
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
