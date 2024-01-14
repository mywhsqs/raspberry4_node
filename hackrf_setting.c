#include "hackrf_setting.h"




int hackrf_device(char usb[10],char device[5][10])
{
    char command[50] = "lsusb > usbfile";
    system(command);
    int i = 0;
    FILE* fp=fopen("usbfile", "r");
    char buf[1024];
    char ID[20] = "OpenMoko";
    while(fgets(buf, 1024, fp) != NULL)
    { 
        char *p = (char *)malloc(20);
        p = strstr(buf, ID);
        //printf("%s\n",buf);
        if (p!=NULL)
        {   
            i++;
            char *p1 = (char *)malloc(20);
            char *p2 = (char *)malloc(20);
            char *p3 = (char *)malloc(20);
            char *p4 = (char *)malloc(20);
            char *p5 = (char *)malloc(20);
            char dev[20] = "";
            char s1[2] = " ";
            char s2[10]= " Device ";
            char s3[2] = ":";
            char s4[2] = "\n";
            p1 = strstr(buf,s1);
            p2 = strstr(buf,s2);
            p3 = strstr(buf,s3);
            if(p1&&p2)
            {
                p1++;
                bzero(usb, 20);
                strncpy(usb, p1, p2-p1);
                //printf("%s\n",usb);
            }
            if(p2&&p3)
            {
                p2++;
                bzero(dev, 20);
                strncpy(dev, p2, p3-p2);
                strcat(dev,"\n");
                //printf("%s\n",dev);
                p4 = strstr(dev,s1);
                p5 = strstr(dev,s4);
                if(p4&&p5)
                {
                    p4++;
                    bzero(device[i-1], 20);
                    strncpy(device[i-1], p4, p5-p4);
                    //printf("%s\n",device[i-1]);
                }
                
            } 
        }
    } 
    return 0;      
}


int hackrf_reset(char input_device[10])
{   
    int hackrf_device(char usb[10],char device[5][10]);
    
    char *bus = (char *)malloc(20); 
    char device[5][10];
    memset(device,0,sizeof(device));
    hackrf_device(bus,device);
    char dev_header[30]= "/dev/bus/usb/";
    char outreset[30] = "";
    strcpy(outreset,dev_header);
    strcat(outreset,bus);
    strcat(outreset,"/"); 
    strcat(outreset,input_device);
    //printf("%s\n",outreset);
    char *filename = (char *)malloc(20);
    int fd;
    int rc;
    filename = outreset;

    fd = open(filename, O_WRONLY);
    if (fd < 0) {
        perror("Error opening output file");
        return 1;
    }

    printf("Resetting USB device %s\n", filename);
    rc = ioctl(fd, USBDEVFS_RESET, 0);
    if (rc < 0) {
        perror("Error in ioctl");
        return 1;
    }
    printf("Reset successful\n");

    close(fd);
    return 0;
}

int hackrf_serial(char serial[5][50])
{ 
    int n = 0;
    int i = 0;
    char command[50] = "hackrf_info > hackrf_file";
    system(command);
    FILE* fp=fopen("hackrf_file", "r");
    char buf[1024];
    char ID[25] = "Serial number";
    while(fgets(buf, 1024, fp) != NULL)
    { 
        char *p = (char *)malloc(20);
        int m = strncmp(buf, "Index", 5);
        if(m == 0) 
        {
            n++;
        }
        p = strstr(buf, ID);//p为hello的出现位置,NULL则为没找到
        if (p!=NULL)
        {   
            i++;
            char *p1 = (char *)malloc(50);
            char *p2 = (char *)malloc(50);
            char *ser = (char *)malloc(50);
            char s1[2] = "0";
            char s2[2] = "\n";
            p1 = strstr(buf,s1);
            p2 = strstr(buf,s2);
            if(p1&&p2)
            {
                //p1++;
                bzero(ser, 50);
                strncpy(ser, p1, p2-p1);
                //strcpy(serial[i-1],s1);
                strcpy(serial[i-1],ser);
                //printf("%s",serial[i-1]);
            }
         }
      }
    //printf("%s\n",device_serial[1]);
    //printf("%d\n",n);
    return n; 
}

/*
int check_hackrf()
{ 
    int n = 0;
    int i = 0;
    char command[50] = "hackrf_info > check_hackrf_file";
    system(command);
    FILE* fp=fopen("check_hackrf_file", "r");
    char buf[1024];
    char ID[25] = "Serial number";
    while(fgets(buf, 1024, fp) != NULL)
    { 
        int m = strncmp(buf, "Index", 5);
        if(m == 0) 
        {
            n++;
        }
    }
    //printf("%d\n",n);
    return n; 
}

*/


int check_hackrf()
{ 
    char command[50] = "lsusb > check_usbfile";
    system(command);
    int i = 0;
    FILE* fp=fopen("check_usbfile", "r");
    char buf[1024];
    char ID[20] = "OpenMoko";
    while(fgets(buf, 1024, fp) != NULL)
    { 
        char *p = (char *)malloc(20);
        p = strstr(buf, ID);
        //printf("%s\n",buf);
        if (p!=NULL)
        {   
            i++;
        }
    }
    return i; 
}

