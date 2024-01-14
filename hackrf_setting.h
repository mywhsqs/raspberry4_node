#ifndef  USB_H
#define  USB_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>


extern int hackrf_device(char usb[10],char device[5][10]);
extern int hackrf_reset(char input_device[10]);
extern int hackrf_serial(char serial[5][50]);
extern int check_hackrf();

#endif
