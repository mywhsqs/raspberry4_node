/********************************************************************
 *
 * Time:         20180426
 * Function:     Command Server
 * Version:      v2.01
 * System:       Linux
 * Language:     C
 * 
 * Mode:      
 * XFCE4 0       Debug mode with windows
 * XFCE4 1       Self start mode
 * RASPBERRY 0   PC system 
 * RASPBERRY 1   Raspberry system 
 *
 * README:
 * It is the server of system. 
 * Main function is: A.Send broadcast for client connect. B.Receive 
 * command of client request. C.Connect SDR and save spectrum data in
 * database. D.Send data base client command.
 * 
 ********************************************************************/ 


#include "api.h" //// API


#define MAX_LINE 10240
#define COMMAND_LINE 32

#define BUFLEN 1024

#define DEFAULT_PORT 54321
#define DATABASE_PORT 54322
#define FILE_PORT 9999  
#define MAXLINE 4096 

#define INTERFACE1 "eth0"

#define LENGTH_OF_LISTEN_QUEUE 20 
#define BUFFER_SIZE 1024 
#define FILE_NAME_MAX_SIZE 512 

bool      isScan         = false;
bool      isSend         = false;
pthread_t scan_thread_id = 0;
pthread_t gps_thread_id = 0;
char      start_freq[20] = "700000000";
char      stop_freq[20]  = "800000000";
char      gps_file[BUFFER_SIZE] = "";
char      target_lon[BUFFER_SIZE] = "";
char      target_lat[BUFFER_SIZE] = "";
char      sdr_id[BUFFER_SIZE] = "0000000000000000866863dc26581fcf";
char      task_name[20] = "";
int       sdr_num = 0;

//database export
bool      isFinished     = false;  //database is or not finished (thread signal)  
bool      dataexport     = false;  //database export status

#if URAT
int     urat_fd = 0;
#endif
int    angle = -1; //// Angle
//int    temp = -1; /// Temp--INS
char   temp[64] = "-1"; //// Temp--DHT11
char   hum[64] = "-1"; //// Temp--DHT11
int    height = -1; /// Height
int    lon = -1; //// Location
int    lat = -1;
char   light[64] = "-1"; //// Light
int    year  =  -1;
int    month =  -1;
int    day   =  -1;
int    hour  =  -1;
int    minute = -1;
char   sys_time[30] = "-1";
char   boardcast_name[64] = "-1";
char   AP_name[64] = "-1";
char   cc[20] = "-1";
char   vv[20] = "-1";
char   pp[20] = "-1";
char   iq[40] = "-1";

float   RSSI_angle = -1;

FILE    *fp_log = NULL; //// Log

//// Arg
int  arg_num           = -1;
char arg_serial[50]    = "";
char arg_array[5][50]  = {"", "", "", "", ""};
char arg_device[5][10] = {"", "", "", "", ""};

char eth_name[20] = "";
char wifi_name[20] = "";

////occuaption(single point) 
long      count = 0;
float     average[10000] = {0.0};
long      total[10000] = {0};
long      occupancy[10000] = {0};
char      occupation_start[20] = "600000000";
char      occupation_stop[20]  = "800000000";


////occuaption(multiple point) 
char      s_bandoccupation[15] = "-1"; 
int       multioccupation      = 0;
char      occu_mode[5]       = "-1";

////relay_mode
char relay_com[10] = "";  
char relay_mode[10] = "-1";


////20230308_zjchen_add for backup
char *server_ip = NULL;

//整型转化为字符串
void itoa(long i,char* string)
{
    int power,j;
    j=i;
    for(power=1;j>=10;j/=10)
    power *= 10;  
    for(;power>0;power/=10)
    {
        *string++='0'+i/power;
        i%=power;
    }
    *string='\0';
}

/***************************************************************
 *                              Log
 ***************************************************************/
int open_log()
{
    fp_log = fopen("log", "a+");
    if(fp_log == NULL)
    {
        printf("[Log] Open log file fail !\n");
    }
    else
    {
        printf("[Log] Open log file success !\n");
    }

    return 0;
}

void close_log()
{
    fclose(fp_log);
}

void inset_log(char *log)
{
    char      time_log[20] = "";

    time_t timer = time(NULL); 
    strftime(time_log, 20, "%Y%m%d%H%M%S", localtime(&timer)); 

    fprintf(fp_log, "%s:", time_log);
    fprintf(fp_log, "%s\n", log);
}



/***************************************************************
 *                       Check hackrf_num thread
 ***************************************************************/
void pthread_check_hackrf_num()
{
    while(1)
    {
        int hackrf_num = check_hackrf();
        char check_time[20] = "";

        time_t timer = time(NULL); 
        strftime(check_time, 20, "%Y-%m-%d %H:%M:%S", localtime(&timer));

        if(hackrf_num!=sdr_num)
        {
            open_log();
            inset_log("Reboot.\n");
            close_log();
#if RASPBERRY
            //get_time();
            system("reboot");
#else
            system("shutdown now");
#endif
        }
        else
        {
            printf("[Main] check_time: %s hackrf_num: %d Check success!!!\n",check_time,hackrf_num);
        }
        sleep(30);
    }

}

void create_check_hackrf_num_thread()
{
    pthread_t id;
    int       ret;

    ret = pthread_create(&id, NULL, (void *)pthread_check_hackrf_num, NULL); //// create pthread
    if(ret != 0){
        printf("[Main] Create get angle pthread error!\n");
        //exit(1);
        return;
    }
    return;
}

/***************************************************************

 *                       IQ max power
 ***************************************************************/
int get_max_pow_iq_distance()
{
    char buffer[64] = ""; 

    FILE *fp = NULL;

    //system("python light_sensor.py > light_sensor_data");
    // Open file and read data 
    fp = fopen("./pow_iq_data", "r"); 
    if(NULL == fp) 
    { 
        printf("[Thread] File:max power id distance Not Found\n"); 
    } 
    else
    { 
        bzero(buffer, 64); 

        fread(buffer, sizeof(char), 64, fp); 
        char* tmp = strtok(buffer, "\n");
           
        int i = 0;
        int count = 0;
        for(;i<30;)
        {
            /*


            if(tmp[i] == '\n')
            {
                light[count] = '\0';


                break;
            }*/
            iq[count] = tmp[i];
            count++;
            i++;
        } 
        iq[count] = '\0'; 
        printf("[Thread] File:iq_data :%s\n", iq); 
        // close file
        fclose(fp); 
        return 0;
    } 
    return 1;


}

/***************************************************************
 *                       Angle base UART
 ***************************************************************/
#if URAT

//// Current&Voltage&Power Start
int get_mode_CVP(char mode[1],char V_Data[50],int * fd)
{ 
  //  char mode[1] = "V";
    char AT[10] = "AT+";
    //int fd; 
    int nread,nwrite,i; 
    char rec_buff[1024];
    char send_buff[50];
    int count = 0;
 //   char V_Data[20] = "";
    strcat(AT, mode);
    strcat(AT, "\r\n");
  
 //   printf("fd=%d\n",*fd); 
    //*fd=14;       
    memset(send_buff, 0, 50);
    memcpy(send_buff, AT, strlen(AT));//Write the voltage
    nwrite = write(*fd, send_buff, strlen(send_buff));//Write the serial port 
    if(-1 == nwrite)
    {
  //      perror("at com write");//
    //    exit(1);
        return 1;
    } 
    sleep(1);
    bzero(rec_buff, 50);
    nread=read(*fd, rec_buff, sizeof(rec_buff));//Read the serial port 
  //  printf("%s\n",rec_buff); 
    
    for(i = 3; i < 8; i++)
    {
        V_Data[count] = rec_buff[i];
        count++;
    }
        V_Data[count] = '\0';
  //      printf("%s\n",V_Data);
   return 0;
   
}
int open_port_CVP(int * fd,int comport) 
{ 
	char *dev[]={"/dev/ttyUSB0","/dev/ttyS1","/dev/ttyS2"}; 

	 if (comport==1)//Port1 
	 {
		*fd = open( "/dev/ttyUSB0", O_RDWR|O_NOCTTY|O_NDELAY); 
		if (-1 == *fd)
		{ 
			perror("[URAT-USB] Can't Open Serial Port"); //
			return(-1); 
		} 
     } 
    else if(comport==2)//Port2 
     {     
		*fd = open( "/dev/ttyS1", O_RDWR|O_NOCTTY|O_NDELAY); 
		if (-1 == *fd)
		{ 
			perror("[URAT-USB] Can't Open Serial Port"); //
			return(-1); 
		} 
     } 
     else if (comport==3)//Port3 
     { 
		*fd = open( "/dev/ttyS2", O_RDWR|O_NOCTTY|O_NDELAY); 
		if (-1 == *fd)
		{ 
			perror("[URAT-USB] Can't Open Serial Port"); //
			return(-1); 
		} 
     } 
/*Restore the serial port as a blocking state*/ 
     if(fcntl(*fd, F_SETFL, 0)<0) 
         printf("[UART-USB]fcntl failed!\n"); 
     else 
	//	 printf("fcntl=%d\n",fcntl(*fd, F_SETFL,0)); 
/*Test whether it is terminal equipment*/ 
     if(isatty(STDIN_FILENO)==0) 
	     printf("[UART-USB]standard input is not a terminal device\n"); 
     else 
  //       printf("[UART-USB]isatty success!\n"); 
    //     printf("fd-open=%d\n",*fd); 
     return *fd; 
}

int set_opt_CVP(int fd,int nSpeed, int nBits, char nEvent, int nStop) 
{ 
     struct termios newtio,oldtio; 
/*Save test existing serial port parameter Settings*/ 
     if  ( tcgetattr( fd,&oldtio)  !=  0)
     {  
 //       perror("SetupSerial 1");//
//	    printf("tcgetattr( fd,&oldtio) -> %d\n",tcgetattr( fd,&oldtio)); 
      return -1; 
     } 
     bzero( &newtio, sizeof( newtio ) ); 
/*Set character size*/ 
     newtio.c_cflag  |=  CLOCAL | CREAD;  
     newtio.c_cflag &= ~CSIZE;  
/*Set stop bit*/ 
     switch( nBits ) 
     { 
     case 7: 
      newtio.c_cflag |= CS7; 
      break; 
     case 8: 
      newtio.c_cflag |= CS8; 
      break; 
     } 
/*Set parity bit*/ 
     switch( nEvent ) 
     { 
     case 'o':
     case 'O': //An odd number
      newtio.c_cflag |= PARENB; 
      newtio.c_cflag |= PARODD; 
      newtio.c_iflag |= (INPCK | ISTRIP); 
      break; 
     case 'e':
     case 'E': //An even number
      newtio.c_iflag |= (INPCK | ISTRIP); 
      newtio.c_cflag |= PARENB; 
      newtio.c_cflag &= ~PARODD; 
      break;
     case 'n':
     case 'N':  //Ordinary parity bit
      newtio.c_cflag &= ~PARENB; 
      break;
     default:
      break;
     } 
     /*Set baud rate*/ 
switch( nSpeed ) 
     { 
     case 2400: 
      cfsetispeed(&newtio, B2400); 
      cfsetospeed(&newtio, B2400); 
      break; 
     case 4800: 
      cfsetispeed(&newtio, B4800); 
      cfsetospeed(&newtio, B4800); 
      break; 
     case 9600: 
      cfsetispeed(&newtio, B9600); 
      cfsetospeed(&newtio, B9600); 
      break; 
     case 115200: 
      cfsetispeed(&newtio, B115200); 
      cfsetospeed(&newtio, B115200); 
      break; 
     case 460800: 
      cfsetispeed(&newtio, B460800); 
      cfsetospeed(&newtio, B460800); 
      break; 
     default: 
      cfsetispeed(&newtio, B9600); 
      cfsetospeed(&newtio, B9600); 
     break; 
     } 
/*Set stop bit*/ 
     if( nStop == 1 ) 
      newtio.c_cflag &=  ~CSTOPB; 
     else if ( nStop == 2 ) 
      newtio.c_cflag |=  CSTOPB; 
/*Set wait time and minimum receive characters*/ 
     newtio.c_cc[VTIME]  = 0; 
     newtio.c_cc[VMIN] = 0; 
/*Handle unreceived characters*/ 
     tcflush(fd,TCIFLUSH); 
/*Enable new configuration*/ 
if((tcsetattr(fd,TCSANOW,&newtio))!=0) 
     { 
      perror("[URAT-USB] com set error"); //
      return -1; 
     } 
 //     printf("set done!\n"); 
     return 0; 
} 
//// Current&Voltage&Power End



int get_light()
{
    char buffer[64] = ""; 
    FILE *fp = NULL;

    system("python light_sensor.py > light_sensor_data");
    // Open file and read data 
    fp = fopen("light_sensor_data", "r"); 
    if(NULL == fp) 
    { 
        printf("[Thread] File:light_sensor_data Not Found\n"); 
    } 
    else
    { 
        bzero(buffer, 64); 

        fread(buffer, sizeof(char), 64, fp); 
        char* tmp = strtok(buffer, "\n");
           
        int i = 0;
        int count = 0;
        for(;i<7;)
        {
            /*
            if(tmp[i] == '\n')
            {
                light[count] = '\0';
                break;
            }*/
            light[count] = tmp[i];
            count++;
            i++;
        } 
        light[count] = '\0'; 
        printf("[Thread] File:light_sensor_data :%s\n", light); 
        // close file
        fclose(fp); 
        return 0;
    } 
    return 1;


}
int get_temp_hum()
{
    char buffer[64] = ""; 
    FILE *fp = NULL;

    system("python dht11.py > dht11_data");
    // Open file and read data 
    fp = fopen("dht11_data", "r"); 
    if(NULL == fp) 
    { 
        printf("[Thread] File:dht11_data Not Found\n"); 
    } 
    else
    { 
        bzero(buffer, 64); 

        fread(buffer, sizeof(char), 64, fp); 
        char* tmp = strtok(buffer, "\n");
           
        int i = 0;
        int count = 0;
        for(;;)
        {
            if(tmp[i] == ' ')
            {
                temp[count] = '\0';
                break;
            }
            temp[count] = tmp[i];
            count++;
            i++;
        }   

        count = 0;
        i++; 
        for(;;)
        {
            if(tmp[i] == '\n')
            {
                hum[count] = '\0';
                break;
            }
            hum[count] = tmp[i];
            count++;
            i++;
        } 
        // close file
        fclose(fp); 
        return 0;
    } 
    return 1;
}


//get time from gyro and write it into the file
int get_time()
{
    while(1)
{
    //date --s="2018-07-12 10:30:00"
	char time[50] = "";
    char ch[6]   = "";
    if(year==18)
    {
    itoa(year+2000,ch);
    strcat(time, ch);
    strcat(time, "-");
    bzero(ch, 6);

    itoa(month, ch);
    strcat(time, ch);
    strcat(time, "-");
    bzero(ch, 6);

    itoa(day, ch);
    strcat(time, ch);
    strcat(time, " ");
    bzero(ch, 6);

    itoa(hour+8, ch);
    strcat(time, ch);
    strcat(time, ":");
    bzero(ch, 6);
 
    itoa(minute, ch);
    strcat(time, ch);
    strcat(time, ":00");
    
    printf("time:%s\n", time);

    FILE *fptime = fopen("time", "w+");
    if (fptime==0) 
    { 
        printf("can't open file\n"); 
    }
    fseek(fptime, 0, SEEK_END);
    fwrite(time, strlen(time), 1, fptime);
    fclose(fptime); 
    break;
    }
    else{continue;}
}
    return 0;
}

int set_time()
{
    char buffer[30] = ""; 
    FILE *fp = NULL;

    // Open file and read data 
    fp = fopen("time", "r"); 
    if(NULL == fp) 
    { 
        printf("[Thread] File:time Not Found\n"); 
    } 
    else
    { 
        bzero(buffer, 30); 

        fread(buffer, sizeof(char), 30, fp); 
        char* tm = strtok(buffer, "\n");
           
        int i = 0;
        int count = 0;
        for(;;)
        {
            if(tm[i] == '\n')
            {
                sys_time[count] = '\0';
                break;
            }
            sys_time[count] = tm[i];
            count++;
            i++;
        }   
        printf("[Thread] File:time :%s\n", sys_time); 
        // close file
        fclose(fp); 
        if(strstr(sys_time, "2018")!= NULL)
        {
  			char time_command[50] = "date --s=\"";
            strcat(time_command, sys_time);
  			strcat(time_command, "\"");
            printf("command: %s\n", time_command);
            system(time_command);
        }
        return 0;
    } 
    return 1; 
}

int open_uart()
{
    int fd; 

    if ((fd = serialOpen("/dev/ttyAMA0", 115200)) < 0)  
    {  
        printf("[Main] serial err\n");  
        return 0;
    }  
    else  
    {  
        printf("[Main] start serial\n");  
        return fd;
    }  
}

void pthread_urat_data(void)
{
    int data;
    int a_h = -1;
    int a_l = -1;
    int i = 0;
    int fd,s;
    char* d = (char *)malloc(20);

    printf("[Main] This get angle pthread start...\n");
    while (1)  
    {  
        if(serialDataAvail(urat_fd) > 0)
        {
            //Read head data
            data = serialGetchar(urat_fd);
            if(data == 0xa5)
            {
                data = serialGetchar(urat_fd);
                if(data != 0x5a)
                {
                    continue;
                }
                else
                {
                    data = serialGetchar(urat_fd);  
                }
            }

            //Read a1 data
            if(data == 0xa1)
            {
                while (1)
                {
                    data = serialGetchar(urat_fd); 
                    angle = data;
                    data = serialGetchar(urat_fd); 
                    angle = ((angle << 8) + data);

                    for(i = 0; ; i++)
                    {
                        data = serialGetchar(urat_fd); 

                        if(i == 4) /// height high
                        {
                            height = data;
                        }
                        else if(i == 5) /// height low
                        {
                            height = (height << 8) + data;
                        }
/*
                        if(i == 6) /// temp high
                        {
                            temp = data;
                        }
                        else if(i == 7) /// temp low
                        {
                            temp = (temp << 8) + data;
                        }
*/
                        if(data == 0xaa)
                        {
                            break;
                        }
                    }

                    break;
                    
                }  
            }  
            //Read a2 data  
            else if(data == 0xa2)
            {
                while (1)
                {
                    data = serialGetchar(urat_fd); 
                    if(data == 0xaa)
                    {
                        break;
                    }
                } 
            }
            //Read a3 data. After six for lon and lat 
            else if(data == 0xa3)
            {
                while (1)
                {
                    data = serialGetchar(urat_fd); 
                    lon = data;
                    for(int i = 1; i < 4; i++)
                    {
                        data = serialGetchar(urat_fd); 
                        lon = data + (lon << 8);
                    }

                    data = serialGetchar(urat_fd); 
                    lat = data;
                    for(int i = 1; i < 4; i++)
                    {
                        data = serialGetchar(urat_fd); 
                        lat = data + (lat << 8);
                    }

                    data = serialGetchar(urat_fd); 
                    for(int i = 1; i < 7; i++)
                    {
                        data = serialGetchar(urat_fd);
                        //printf("%x/", data);
                    }

					data = serialGetchar(urat_fd); 
                    minute = data;
                    //printf("%x ", data);
                    data = serialGetchar(urat_fd); 
                    hour = data;
                    //printf("%x ", data); 
                    data = serialGetchar(urat_fd); 
                    day = data;
                    //printf("%x ", data);
                    data = serialGetchar(urat_fd); 
                    month = data;
                    //printf("%x ", data);
                    data = serialGetchar(urat_fd); 
                    year = data;
                    //printf("%x ", data);
                    for(;;)
                    {
                        data = serialGetchar(urat_fd); 
                        if(data == 0xaa)
                        {
                            break;
                        }
                    }
                    break;
                    data = serialGetchar(urat_fd);
                } 
                //printf("time: %d-%d-%d %d:%d\n", year, month, day, hour+8, minute);   
            }
        }

        if(get_temp_hum() == 1)
        {
            printf("[URAT] read temp hum error\n");
        }
 

        if(get_light() == 1)
        {
            printf("[URAT] read light error\n");
        }

        //get_time();
        //set_time();
//// Current&Voltage&Power Start
        if((fd=open_port_CVP(&fd,1))<0) //Open the serail port 
        {
            perror("[URAT-USB] open_port error"); 
        
        } 
        else
        {
//     return-1;		
	    }
       if((s=set_opt_CVP(fd,9600,8,'N',1))<0) //Initialize the serial port
        {
            perror("[URAT-USB] set_opt error"); 
        
        } 
  //  printf("fd=%d\n",fd); 


        if((get_mode_CVP("C",cc,&fd)) == 1)
        {
           // perror("[URAT-USB] get_mode error"); 
            strcpy(cc,"-1");
           // printf("aaa%s\n",cc);
        }
    //else
    //{
    // bzero(cc,20);
   //  strcat(cc,d);
    //}
//printf("cc%s",cc);
       if((get_mode_CVP("V",vv,&fd)) == 1)
       {
          //  perror("get_mode error"); 
            strcpy(vv,"-1");
       } 
       else
       { 
        //strcpy(vv,d);
       }
//printf("vv%s",vv);
       if((get_mode_CVP("P",pp,&fd)) == 1)
       {
         //   perror("get_mode error"); 
            strcpy(pp,"-1");
       }
       else
       { 
        //strcpy(pp,d);
       }
//// Current&Voltage&Power Start


        printf("[URAT] lon:%d lat:%d height:%d temp:%s hum:%s light:%s angle:%d C:%s V:%s P:%s\n", lon, lat, height, temp, hum, light, angle, cc, vv, pp);   
   
        //sleep(0.5);
    }  
}

void creat_urat_data_thread()
{
    pthread_t id;
    int       ret;

    ret = pthread_create(&id, NULL, (void *)pthread_urat_data, NULL);    //such as Unix
    if(ret != 0){
        printf("[Main] Create get angle pthread error!\n");
        //exit(1);
        return;
    }
    return;
}

#endif


int get_AP_boardcast_name()
{
    char buffer[200] = ""; 
    char ID[20] = "name";
    FILE *fp = NULL;

    // Open file and read data 
    fp = fopen("boardcast_name", "r"); 
    if(NULL == fp) 
    { 
        printf("[Thread] File:boardcast_name Not Found\n"); 
    } 
    else
    { 
        while(fgets(buffer, 200, fp) != NULL)
        { 
            char *p = (char *)malloc(20);
            p = strstr(buffer, ID);
            //printf("%s\n",buf);
            if (p!=NULL)
            {   
 
                printf("%s\n", buffer);
                //char *name = strtok(buffer, "\n");
           
                int i = 5;
                int count = 0;
                
                //bzero(boardcast_name, 64);
                for(;;)
                {
                    if(buffer[i] == ' ')
            	    {
                	    boardcast_name[count] = '\0';
                	    break;
            	    }
            	    boardcast_name[count] = buffer[i];
            	    count++;
            	    i++;
        	    }   
        		printf("%s\n", boardcast_name);
        		count = 0;
        		i++; 
                //bzero(AP_name, 64);
        		for(;;)
        		{
            		if(buffer[i] == '\n')
                	{
                		AP_name[count] = '\0';
                		break;
            		}
            		AP_name[count] = buffer[i];
            		count++;
            		i++;
        		} 
        		printf("%s\n", AP_name);
        		// close file
        		fclose(fp); 
                return 0;
            }
        }
    } 

    return 1;
}

/***************************************************************
 *                           occulocalsocket
 ***************************************************************/
void pthread_occulocalsocket(void *arg)
{
    
    int  num          = *(int *)arg;
    char num_c[2]     = "";
    itoa(num, num_c);
    char      local_name[20]  = "MAIN";
    strcat(local_name, num_c);

    while(1)
    {
        int       server_sockfd, client_sockfd;  
        int       server_len, client_len;  
        struct    sockaddr_un server_address; /*声明一个UNIX域套接字结构*/  
        struct    sockaddr_un client_address;  
        int       i, bytes;  
        char      ch_send[50]   = "";  
        char      ch_recv[50]   = "";
    
        unlink (local_name); /*删除原有server_socket对象*/  
        /*创建 socket, 通信协议为AF_UNIX, SCK_STREAM 数据方式*/  
        server_sockfd = socket (AF_UNIX, SOCK_STREAM, 0);    
        /*配置服务器信息(通信协议)*/  
    	server_address.sun_family = AF_UNIX;   
    	/*配置服务器信息(socket 对象)*/  
    	printf("%s\n", local_name);
    	strcpy (server_address.sun_path, local_name);   
    	/*配置服务器信息(服务器地址长度)*/  
    	server_len = sizeof (server_address);   
    	/*绑定 socket 对象*/  
    	bind (server_sockfd, (struct sockaddr *)&server_address, server_len);   
    	/*监听网络,队列数为5*/  
    	listen (server_sockfd, 5);   
    	//printf ("Spectrum: Server is waiting for client connect...\n");  
    	client_len = sizeof (client_address);   
    	/*接受客户端请求; 第2个参数用来存储客户端地址; 第3个参数用来存储客户端地址的大小*/  
    	/*建立(返回)一个到客户端的文件描述符,用以对客户端的读写操作*/  
    	client_sockfd = accept(server_sockfd, (struct sockaddr *)&server_address, (socklen_t *)&client_len);  
    	if(client_sockfd == -1) 
    	{  
            perror("Spectrum: accept");  
            exit (EXIT_FAILURE);  
        }  
        //printf("Spectrum: The server is waiting for client data...\n"); 

        if((bytes = read(client_sockfd, ch_recv, sizeof(ch_recv))) == -1) 
        {  
            perror("Spectrum: read");  
            exit(EXIT_FAILURE);  
        }  
        strcpy(s_bandoccupation, ch_recv);

        close(client_sockfd);  
        unlink(local_name);
    }
}

/***************************************************************
 *                          getsensordata

 ***************************************************************/
void pthread_getsensordata(void *arg)
{
    
    int  num          = *(int *)arg;
    char num_c[2]     = "";
    itoa(num, num_c);
    char      local_name[20]  = "Sensor";
    strcat(local_name, num_c);

    //while(1)
    //{
        
    int       server_sockfd, client_sockfd;  
    int       server_len, client_len;  
    struct    sockaddr_un server_address;   
    struct    sockaddr_un client_address;  
    int       i, bytes;  

    
    unlink (local_name); 
 
    server_sockfd = socket (AF_UNIX, SOCK_STREAM, 0);    

    server_address.sun_family = AF_UNIX;   

    //printf("%s\n", local_name);
    strcpy (server_address.sun_path, local_name);   
 
    server_len = sizeof (server_address);   
  
    bind (server_sockfd, (struct sockaddr *)&server_address, server_len);   

    listen (server_sockfd, 5);    
    client_len = sizeof (client_address);   

    client_sockfd = accept(server_sockfd, (struct sockaddr *)&server_address, (socklen_t *)&client_len);  
    if(client_sockfd == -1) 
    {  
        perror("[Main]: accept");  
        exit (EXIT_FAILURE);  
    }  
        //printf("Spectrum: The server is waiting for client data...\n"); 
    while(1){
        
        char      ch_send[50]   = "";  
        char      ch_recv[50]   = "";
        if((bytes = read(client_sockfd, ch_recv, sizeof(ch_recv))) == -1) 
        {  
            perror("[Main]: read");  
            //exit(EXIT_FAILURE); 
            continue; 
        }  
        if(strstr(ch_recv, "Getsensordata!")!= NULL)
        {
            strcat(ch_send, temp);
            strcat(ch_send, ",");
            strcat(ch_send, hum);
            strcat(ch_send, ",");
            char str[10] = "";
            gcvt((float)height / 10, 4, str);
            strcat(ch_send, str);
	    strcat(ch_send, ",");
            strcat(ch_send, light);
            strcat(ch_send, "\n");            
            //ch_send = 'O';
            if ((bytes = write(client_sockfd, ch_send, sizeof(ch_send))) == -1) 
            {  
                perror("[Main] read");  
                //exit(EXIT_FAILURE); 
                continue; 
            }
            //printf("main.c to spectrum.c data:%s\n", ch_send);
        }
        //sleep (2);
        //printf("main.c to spectrum.c data:%s\n", ch_recv);
        //sleep (0.5);
        //close(client_sockfd);  
        //unlink(local_name);
    }
}


void create_getsensordata_thread(int* i)
{
     pthread_t id;
     int       ret;
     ret = pthread_create(&id, NULL, (void *)pthread_getsensordata, i);
     if(ret != 0)
     {
         printf("Thread: Create getsensordata pthread error!\n");
         return;
     }
}
/***************************************************************
 *                           Ethernet
 ***************************************************************/
 #if RK3568
int get_ethernet_interface_name()
{
    char buffer[64] = ""; 
    char eth[64] = "";
    char wifi[64] = "";
    char *tmp;
    FILE *fp = NULL;

    system("ifconfig -a | grep -oE \"wlx............|eth[0~9]\" > net_data");
    fp = fopen("net_data", "r");

    if(NULL == fp) 
    { 
        printf("[Thread] File:dht11_data Not Found\n"); 
    } 
    else
    { 
        bzero(buffer, 64); 
        fread(buffer, sizeof(char), 64, fp);
        tmp = strtok(buffer, "\n");
        strcpy(eth_name,tmp);
        printf("%s\n",eth_name);
        tmp = strtok(NULL,"\n");
        strcpy(wifi_name,tmp);
        printf("%s\n",wifi_name);

    }

    fclose(fp);
    return 0;
}
#else
int get_ethernet_interface_name()
{
    char buffer[64] = ""; 
    char eth[64] = "";
    char wifi[64] = "";
    char *tmp;
    FILE *fp = NULL;

    system("ifconfig -a | grep -oE \"wlan.|eth[0~9]\" > net_data");
    fp = fopen("net_data", "r");

    if(NULL == fp) 
    { 
        printf("[Thread] File:dht11_data Not Found\n"); 
    } 
    else
    { 
        bzero(buffer, 64); 
        fread(buffer, sizeof(char), 64, fp);
        tmp = strtok(buffer, "\n");
        strcpy(eth_name,tmp);
        printf("%s\n",eth_name);
        tmp = strtok(NULL,"\n");
        strcpy(wifi_name,tmp);
        printf("%s\n",wifi_name);

    }

    fclose(fp);
    return 0;
}
#endif


/***************************************************************
 *                              MAC
 ***************************************************************/
int x2c(unsigned char* c, char* mac)
{
    int i;
    char ddl,ddh;
/*    printf("%02x:%02x:%02x:%02x:%02x:%02x\n", 
            (unsigned   char)c[0], 
            (unsigned   char)c[1], 
            (unsigned   char)c[2], 
            (unsigned   char)c[3], 
            (unsigned   char)c[4], 
            (unsigned   char)c[5]);
*/
    for (i=0; i<6; i++)
    {
        ddh = 48 + c[i] / 16;
        ddl = 48 + c[i] % 16;
        if (ddh > 57) 
            ddh = ddh + 7;
        if (ddl > 57) 
            ddl = ddl + 7;
        mac[i*2] = ddh;
        mac[i*2+1] = ddl;

    }

    mac[20] = '\0';
    printf("[Main] Local MAC address : %s\n", mac); 

    return 0;
}
 
int getMac(char *net, unsigned char *mac) 
{ 
    struct   ifreq   ifreq; 
    int      sock; 
    int      i;

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) <0) 
    { 
        perror( "socket "); 
        return   2; 
    } 
    strcpy(ifreq.ifr_name, net); 
    if(ioctl(sock, SIOCGIFHWADDR, &ifreq) <0) 
    { 
        perror( "ioctl "); 
        return   3; 
    } 
    //strcpy(mac, ifreq.ifr_hwaddr.sa_data);
    //strcat(mac, (unsigned   char)ifreq.ifr_hwaddr.sa_data[0]);
    //printf("%s\n",mac);
    for(i = 0; i < 6; i++)
    {
        mac[i] = ifreq.ifr_hwaddr.sa_data[i];
    }

/*
    printf("%02x\n",ifreq.ifr_hwaddr.sa_data);
    printf("%02x:%02x:%02x:%02x:%02x:%02x\n ", 
            (unsigned   char)ifreq.ifr_hwaddr.sa_data[0], 
            (unsigned   char)ifreq.ifr_hwaddr.sa_data[1], 
            (unsigned   char)ifreq.ifr_hwaddr.sa_data[2], 
            (unsigned   char)ifreq.ifr_hwaddr.sa_data[3], 
            (unsigned   char)ifreq.ifr_hwaddr.sa_data[4], 
            (unsigned   char)ifreq.ifr_hwaddr.sa_data[5]); 
  */      
    return   0; 
}



/***************************************************************
 *               Get start_freq stop_freq serial
 ***************************************************************/
int get_freq_serial(char* buffer)
{
    printf("\n======================== Get start_freq stop_freq serial ========================\n");
    printf("[Main] Get spectrum data buffer:%s\n", buffer);
    int count = 0;
    int i = 0;
    int j = 0;
    if(strstr(buffer, "threshold") != NULL)
    {
        i = 10;
    }
    if(strstr(buffer, "mloccupy") != NULL)
    {
        i = 9;
    }
    if(strstr(buffer, "spectrum") != NULL)
    {
        i = 9;
    }
    if(strstr(buffer, "DF") != NULL)
    {
        i = 3;
    }

    //get relay initial or set mode 
    if(strstr(buffer, "relay") != NULL)
    {
        j = 6;
        for(; ; j++)
        {
            if(buffer[j] == '\n')
            {
                relay_com[count] = '\0';
                break;
            }
            relay_com[count] = buffer[j];
            count++;  
        }     
    }
    count = 0;
    //// Get start frequency
    if(strstr(buffer, "relay") == NULL)
    {

    	for(; ; i++)
    	{
    	    if(buffer[i] == ' ')
        	{
            	start_freq[count] = '\0';
            	break;
        	}
        	start_freq[count] = buffer[i];
        	count++;
    	}
    	printf("[Main] start_freq:%s\n", start_freq);
    	count = 0;

    	//// Get stop frequency
    	for(i = i + 1; ; i++)
    	{
        	if(strstr(buffer, "threshold") != NULL)
        	{
            	if(buffer[i] == '\n')
            	{
                	stop_freq[count] = '\0';
                	break;
            	}
        	}
        	else
        	{
            	if(buffer[i] == ' ')
            	{
                	stop_freq[count] = '\0';
                	break;
           		}

        	}
        	stop_freq[count] = buffer[i];
        	count++;
    	}
    	printf("[Main] stop_freq:%s\n", stop_freq);
    	count = 0;

    	//// Get serial
    	if(strstr(buffer, "threshold") != NULL)
    	{
     
        	printf("[Main] NO serial_num:%s\n", buffer);

    	}
    	else if(strstr(buffer, "mloccupy") != NULL)
    	{
        	for(i = i + 1; ; i++)
        	{
            	if(buffer[i] == ' ')
            	{
                	arg_serial[count] = '\0';
                	break;
            	}
            	arg_serial[count] = buffer[i];
            	count++;
        	}
        	printf("[Main] serial_num:%s i:%d\r\n", arg_serial, i);
        	count = 0;

        	///get occupation mode
        	for(i = i + 1; ; i++)
        	{
            	if(buffer[i] == '\n')
            	{
                	occu_mode[count] = '\0';
                	break;
            	}
            	occu_mode[count] = buffer[i];
            	count++;
        	}
    	}
    	else
    	{
        	for(i = i + 1; ; i++)
        	{
            	if(buffer[i] == '\n')
            	{
                	arg_serial[count] = '\0';
                	break;
            	}
            	arg_serial[count] = buffer[i];
            	count++;
        	}
        	printf("[Main] serial_num:%s i:%d\r\n", arg_serial, i);
    	}
    }
    return 1;
}



/***************************************************************
 *            scan spectrum and send data to terminal
 ***************************************************************/
void pthread_scan_spectrum_terminal(void)
{
//    while(1)
/*    {
        if(isScan)
        {
            char command[50] = "xfce4-terminal -e \"./hackrf_spectrum_sense.sh\"";
            printf("[Thread] This is a scan pthread start\n");
            system(command);
            printf("[Thread] This is a scan pthread end\n");
        }
    }
*/
    struct sockaddr_in server_addr; 
    bzero(&server_addr, sizeof(server_addr)); 
    server_addr.sin_family = AF_INET; 
#if BROADCAST_E
    server_addr.sin_addr.s_addr = htons(INADDR_ANY); 
#else
    server_addr.sin_addr.s_addr = inet_addr("192.168.12.1");
#endif
    server_addr.sin_port = htons(FILE_PORT); 
 
    //// Create socket
    int server_socket_fd = socket(PF_INET, SOCK_STREAM, 0); 
    if(server_socket_fd < 0) 
    { 
        perror("[Thread] Create Socket Failed:"); 
        return;//exit(1); 
    } 
  
    int opt = 1; 
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); 
  
    //// Band socket and the addr structure of socket 
    if(-1 == (bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)))) 
    { 
        perror("[Thread] Server Bind Failed:"); 
        return;//exit(1); 
    } 
  
    //// Listen socket
    if(-1 == (listen(server_socket_fd, LENGTH_OF_LISTEN_QUEUE))) 
    { 
        perror("[Thread] Server Listen Failed:"); 
        return;//exit(1); 
    } 

    while(isSend) 
    { 
        //// Addr structure of socket 
        struct sockaddr_in client_addr; 
        socklen_t client_addr_length = sizeof(client_addr); 
 
        //// Receive connect, return one new socket for communication with client 
        //// Accept, write client infor into client_addr
        int new_server_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, &client_addr_length); 
        if(new_server_socket_fd < 0) 
        { 
            perror("[Thread] Server Accept Failed:"); 
            break; 
        } 
 
        //// Recv buffer 
        //char buffer[BUFFER_SIZE] = "spectrum 700000000 800000000\n"; 
        char buffer[BUFFER_SIZE] = ""; 
        //char start_freq[20] = "";
        //char stop_freq[20] = "";
        char start_freq2[20] = "";
        char stop_freq2[20] = "";
        bzero(buffer, BUFFER_SIZE); 
        if(recv(new_server_socket_fd, buffer, BUFFER_SIZE, 0) < 0) 
        { 
            perror("[Thread] Server Recieve Data Failed:"); 
            break; 
        } 
#if 0
        if(strstr(buffer, "spectrum") != NULL)
        {
            get_freq_serial(buffer);
        }
        else
        {
            printf("[Thread] Error spectrum data\n");
            //continue;
        }
#endif
        //// Copy data from buffer to file_name 
        char file_name[FILE_NAME_MAX_SIZE+1] = "send_scan_freq"; 
        char num_c[2]                        = "";

        itoa(arg_num, num_c);
        strcat(file_name, num_c);
        //bzero(file_name, FILE_NAME_MAX_SIZE+1); 
        //strncpy(file_name, buffer, strlen(buffer)>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE:strlen(buffer)); 

        //// Send data start
        //printf("buffer:%s\n",buffer); 
        printf("[Thread] file_name:%s\n",file_name);

        /*******************Angle data in file start*******************/
#if URAT
//// Write the angle value into the file
/*
        FILE *fp_angle = fopen(file_name, "a+");
        float angle_send = (float)angle / 10;
        if(fp_angle == NULL)
        {
            printf("[Thread] Open file fail");
            //return 0;
        }
        fprintf(fp_angle, "angle,%0.1f;\n", angle_send);//Input angle data
        //printf("[Thread] ============ angle: %0.1f ============\n", angle_send);   
        fclose(fp_angle);
*/
#endif
        /*******************Angle data in file end*******************/

        //// Open file and read 
        FILE *fp = fopen(file_name, "r"); 
        if(NULL == fp) 
        { 
            printf("[Thread] File:%s Not Found\n", file_name); 
        } 
        else
        { 
            bzero(buffer, BUFFER_SIZE); 
            int length = 0; 
            //// read one send one，untuil at the end of file 
            while((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0) 
            { 
                //printf("[Thread] Sending data:%s\n", buffer); 
                if(send(new_server_socket_fd, buffer, length, 0) < 0) 
                { 
                    printf("[Thread] Send File:%s Failed.\n", file_name); 
                    break; 
                } 
                bzero(buffer, BUFFER_SIZE); 
            } 
/*
            //// Angle
            char a[8];

            angle = get_angle(urat_fd);
            sprintf(a, "%g", angle);
            strcat(buffer, "Angle,");
            strcat(buffer, a);
            strcat(buffer, ";\0");
            printf("[Thread] %s\n", buffer); 

            send(new_server_socket_fd, buffer, sizeof(buffer), 0);
            bzero(buffer, BUFFER_SIZE);
*/
            //// close file 
            fclose(fp); 
            
            printf("[Thread] File:%s Transfer Successful!\n", file_name); 
        }    
        //// close connection with client 
        close(new_server_socket_fd); 
        //// Send data end
    } 
    //// close socket 
    close(server_socket_fd); 
}

void create_scan_spectrum_terminal_thread()
{
    pthread_t id;
    int       ret;

    ret = pthread_create(&id, NULL, (void *)pthread_scan_spectrum_terminal, NULL);
    if(ret != 0){
        printf("[Thread] Create scan spectrum terminal pthread error!\n");
        //exit(1);
        return;
    }
    return;
}



/****************************************************************************
 *                               Create AP 
 ****************************************************************************/
#if AP
#if RK3568
void pthread_ap(void)
{
    //char command[50] = "./hackrf_spectrum_sense_all 700000000 800000000";

    //char command[50] = "xfce4-terminal -e \"create_ap wlan0 enxb827eb3cac05 My_AP 12345678\"";
    //char command[100] = "create_ap wlan0 enxb827ebfbafd9 My_AP 12345678 -c 7";
    //char command[200] = "create_ap wlx14f5f9f93873 ";
    char command[200] = "create_ap ";
    strcat(command, wifi_name);
    strcat(command, " ");
    strcat(command, eth_name);
    strcat(command, " ");
    strcat(command, AP_name);
    strcat(command, " 12345678 -c 7 --no-virt");
    //strcat(command, " My_AP_SUN 12345678 -c 7");
    printf("%s\n", command);
    system(command);
    printf("[Thread] This AP pthread start...\n");
}
#else
void pthread_ap(void)
{
    //char command[50] = "./hackrf_spectrum_sense_all 700000000 800000000";

    //char command[50] = "xfce4-terminal -e \"create_ap wlan0 enxb827eb3cac05 My_AP 12345678\"";
    //char command[100] = "create_ap wlan0 enxb827ebfbafd9 My_AP 12345678 -c 7";
    char command[200] = "create_ap wlan0 ";

    strcat(command, eth_name);
    strcat(command, " ");
    strcat(command, AP_name);
    strcat(command, " 12345678 -c 7 --no-virt");
    //strcat(command, " My_AP_SUN 12345678 -c 7");
    printf("%s\n", command);
    system(command);
    printf("[Thread] This AP pthread start...\n");
}
#endif

void create_ap_thread()
{
    pthread_t id;
    int       ret;

    ret = pthread_create(&id, NULL, (void *)pthread_ap, NULL);
    if(ret != 0){
        printf("[Thread] Create AP pthread error!\n");
        //exit(1);
        return;
    }
    return;
}
#endif



/****************************************************************************
 *                         Semun for synchronization 
 ****************************************************************************/
union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *arry;
};

static int sem_id = 0;

static int set_semvalue()
{
    //// init signal，important !!!
    union semun sem_union;

    sem_union.val = 1;
    if(semctl(sem_id, 0, SETVAL, sem_union) == -1)
    {
        return 0;
    }
    return 1;
}



/****************************************************************************
 *                               Scan spectrum 
 ****************************************************************************/
void pthread_scan_spectrum(void *arg)
{
#if XFCE4    
    char command[128] = "xfce4-terminal -e \"./spectrum"; 
#else
    char command[128] = "./spectrum";
#endif
    int  num          = -1;
    char num_c[2]     = "";
    char s_multioccupation[2]   = "";
    itoa(multioccupation, s_multioccupation);

    num = *(int*)arg;
    printf("[Thread] scan spectrum. num:%d, serial:%s\n", num, arg_array[num]);

    itoa(num, num_c);
    strcat(command, num_c);
    strcat(command, " -s ");
    strcat(command, arg_array[num]);
    strcat(command, " -n ");
    strcat(command, num_c);
    strcat(command, " -d ");
    strcat(command, arg_device[num]);
    strcat(command, " ");
    //if(start_freq != "" && stop_freq != "")
    {
        strcat(command, start_freq);
        strcat(command, " ");
        strcat(command, stop_freq);
    }
    strcat(command, " -o ");
    strcat(command, s_multioccupation);
    strcat(command, " -m ");
    strcat(command, occu_mode);

#if XFCE4   
    strcat(command, "\"");   
#endif
    //char command[128] = "xfce4-terminal -e \"./main\"";
    printf("%s\n", command);
    system(command);

    printf("This is a spectrum pthread start...\n");
}

void create_scan_spectrum_thread(int* num)
{
    pthread_t id;
    int       ret;

    printf("@@@@@@ num1:%d\n", *num);
    ret = pthread_create(&id, NULL, (void *)pthread_scan_spectrum, num);
    printf("@@@@@@ num2:%d\n", *num);
    if(ret != 0){
        printf("[Thread] Create scan spectrum pthread error!\n");
        //exit(1);
        return;
    }
    return;
}



/****************************************************************************
 *                      Direction finding base on RSSI
 ****************************************************************************/
float get_max_frequency(char* filename)
{
    char file_name[FILE_NAME_MAX_SIZE + 1] = ""; 
    char buffer[BUFFER_SIZE] = "";
    FILE *fp = NULL; 
    float freq_1 = 0;
    float freq_2 = 0;//// Max frequency

    strcpy(file_name, filename);

    fp = fopen(file_name, "r");
    if(NULL == fp) 
    { 
        printf("[Thread] File:%s Not Found\n", file_name); 
    } 
    else
    { 
        bzero(buffer, BUFFER_SIZE); 
        int length = 0; 

        while((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0) 
        { 
            int count = 0;
            int i = 0;
            char freq_tmp[20] = "";

            do
            {
                //// Get start point
                for(; i < BUFFER_SIZE; i++)
                {
                    if(buffer[i] == ',')
                    {
                        break;
                    }
                }
                
                //// Get energy point
                count = 0;
                for(i = i + 1; i < BUFFER_SIZE; i++)
                {
                    if(buffer[i] == ';')
                    {
                        break;
                    }
                    freq_tmp[count] = buffer[i];
                    count++;
                }
                //printf("[Main] S:%s F:%f\n", freq, atof(freq));
                freq_1 = atof(freq_tmp);
                if(freq_2 == 0 || freq_1 != 0 && freq_2 !=0 && freq_2 < freq_1)
                {
                    freq_2 = freq_1;
                }
            }while(i < BUFFER_SIZE);

            bzero(buffer, BUFFER_SIZE); 
        } 
        fclose(fp);            
    }  

    return freq_2;
}

//// acute_Angle for A B C
//// C > B > A
float calculate_A_acute_angle(float A, float B, float C)
{
    float acute_angle = -1;
    float ll2 = -1;

    ll2 = (exp((C - A) / 20) - 1) / (exp((C - B) / 20) - 1) - 1;
    acute_angle = atan(1 / (1 + 1 / ll2)) * 180 / M_PI;

    printf("RSSI_Angle: A acute_angle. tan():%f radian:%f acute_angle:%f\n", 1 / (1 + 1 / ll2), acute_angle * M_PI / 180, acute_angle);

    return acute_angle;
}

//// DF base on RSSI 
float calculate_coming_RSSI_angle(float ra, float rb, float rc, float rd)
{
    //float RSSI_angle = -1;
    //float ll2 = -1;
    float r[4] = {ra, rb, rc, rd};    
    int id[4] = {0, 1, 2, 3};  
    float tmp = 0;
    RSSI_angle = -1;

    printf("RSSI_Angle: Energy before sort. ra:%f rb:%f rc:%f rd:%f\n", r[0], r[1], r[2], r[3]);
    for(int i = 0; i < 3; i++)
    {
        for( int j = i + 1; j < 4; j++)
        {
            if(r[i] > r[j])
            {
                tmp = r[i];
                r[i] = r[j];
                r[j] = tmp;

                tmp = id[i];
                id[i] = id[j];
                id[j] = tmp;
            }
        }
    }
    printf("RSSI_Angle: Energy after sort. r[0]:%f r[1]:%f r[2]:%f r[3]:%f\n", r[0], r[1], r[2], r[3]);
    printf("RSSI_Angle: ID after sort. ID[0]:%d ID[1]:%d ID[2]:%d ID[3]:%d\n", id[0], id[1], id[2], id[3]);

    //ll2 = (exp((rc - ra) / 20) - 1) / (exp((rc - rb) / 20) - 1) - 1;
    //acute_angle = atan(1 / (1 + 1 / ll2)) * 180 / M_PI;

    //// id[0] smallest, id[3] largest
    if(id[0] == 0 && id[3] == 3) //// First Quadrant
    {
        if(rb < rc)
        {
            RSSI_angle = calculate_A_acute_angle(ra, rb, rc);
            printf("RSSI_Angle: [0, 45]\n");
        }
        else if(rb > rc)
        {
            RSSI_angle = 90 - calculate_A_acute_angle(ra, rc, rb);
            printf("RSSI_Angle: [45, 90]\n");
        }
    }
    else if(id[0] == 2 && id[3] == 1) //// Second Quadrant
    {
        if(ra < rd)
        {
            RSSI_angle = 90 + calculate_A_acute_angle(rc, ra, rd);
            printf("RSSI_Angle: [90, 135]\n");
        }
        else if(ra > rd)
        {
            RSSI_angle = 180 - calculate_A_acute_angle(rc, rd, ra);
            printf("RSSI_Angle: [135, 180]\n");
        }
    }
    else if(id[0] == 3 && id[3] == 0) //// Third Quadrant
    {
        if(rc < rb)
        {
            RSSI_angle = 180 + calculate_A_acute_angle(rd, rc, rb);
            printf("RSSI_Angle: [180, 225]\n");
        }
        else if(rc > rb)
        {
            RSSI_angle = 270 - calculate_A_acute_angle(rd, rb, rc);
            printf("RSSI_Angle: [225, 270]\n");
        }
    }
    else if(id[0] == 1 && id[3] == 2) //// 4th Quadrant
    {
        if(rd < ra)
        {
            RSSI_angle = 270 + calculate_A_acute_angle(rb, rd, ra);
            printf("RSSI_Angle: [270, 315]\n");
        }
        else if(rd > ra)
        {
            RSSI_angle = 360 - calculate_A_acute_angle(rb, ra, rd);
            printf("RSSI_Angle: [315, 360]\n");
        }
    }
    else
    {
        printf("RSSI_Angle: Error combination !!!\n");
        return RSSI_angle = -1;
    }

    //printf("RSSI_Angle: RSSI_angle:%f\n", RSSI_angle);
    return RSSI_angle;
}

void pthread_direction_finding()
{
    while(1)
    {
        char file_name[FILE_NAME_MAX_SIZE + 1] = ""; 
        float max0 = 0;
        float max1 = 0;
        float max2 = 0;
        float max3 = 0;
        float RSSI_angle = -1;

        printf("\n======================== Direction Finding ========================\n"); 

        bzero(file_name, FILE_NAME_MAX_SIZE + 1); 
        strcpy(file_name, "scan_freq0");
        max0 = get_max_frequency(file_name);

        bzero(file_name, FILE_NAME_MAX_SIZE + 1); 
        strcpy(file_name, "scan_freq1");
        max1 = get_max_frequency(file_name);

        bzero(file_name, FILE_NAME_MAX_SIZE + 1); 
        strcpy(file_name, "scan_freq2");
        max2 = get_max_frequency(file_name);

        bzero(file_name, FILE_NAME_MAX_SIZE + 1); 
        strcpy(file_name, "scan_freq3");
        max3 = get_max_frequency(file_name);

        if(!(max0 != 0 && max1 != 0 && max2 != 0 && max3 != 0))
        {
            printf("RSSI_Angle: Get scan data error !!!\n");
            continue;
        }

        RSSI_angle = calculate_coming_RSSI_angle(max2, max1, max0, max3);

        printf("RSSI_Angle: ra:%f rb:%f rC:%f rd:%f RSSI_angle:%f\n", max2, max1, max0, max3, RSSI_angle);

        sleep(1);
    }
}

void create_direction_finding_thread()
{
    pthread_t id;
    int       ret;

    ret = pthread_create(&id, NULL, (void *)pthread_direction_finding, NULL);
    if(ret != 0){
        printf("[Thread] Create direction finding thread error!\n");
        //exit(1);
        return;
    }
    return;
}



/****************************************************************************
 *                        close terminal by ./localsocket
 ****************************************************************************/
void close_terminal_by_localsocket(int num)
{
    char command[BUFFER_SIZE] = "./localsocket SDR";
    char num_c[2]             = "";

    itoa(num, num_c);
    if(strstr(start_freq, "") != NULL)
    strcat(command, num_c);
    strcat(command, " Q");
    system(command);
}



/****************************************************************************
 *                                Create GPS
 ****************************************************************************/
void pthread_gps(void)
{
    //char command[200] = "hackrf_transfer -d 0000000000000000954863c8334e4d23 -t /media/rikirobot/6753ec6b-7b52-43f6-af04-785cf3300d34/";
    char command[200] = "hackrf_transfer -d ";

    strcat(command, sdr_id);
    strcat(command, " -t ./");
    strcat(command, gps_file);
    strcat(command, " -f 1575420000 -s 2600000 -a 1 -x 25 -R");

    system(command);

    printf("[Thread] This GPS pthread start...\n");
}

void create_gps_thread()
{
    //pthread_t id;
    int       ret;

    ret = pthread_create(&gps_thread_id, NULL, (void *)pthread_gps, NULL);
    if(ret != 0){
        printf("[Thread] Create gps pthread error!\n");
        //exit(1);
        return;
    }
    return;
}

void pthread_build(void)
{
    //char command1[200] = "cd /media/rikirobot/6753ec6b-7b52-43f6-af04-785cf3300d34";
    //char command2[200] = "/media/rikirobot/6753ec6b-7b52-43f6-af04-785cf3300d34/gps-sdr-sim -e brdc3540.14n -l ";
    char command2[200] = "./gps-sdr-sim -e brdc3540.14n -l ";

    strcat(command2, target_lat);
    strcat(command2, ",");
    strcat(command2, target_lon);
    strcat(command2, ",100 -b 8 -d 100");

    //system(command1);
    system(command2);

    printf("[Thread] This GPS pthread start...\n");
}

void create_build_thread()
{
    //pthread_t id;
    int       ret;

    ret = pthread_create(&gps_thread_id, NULL, (void *)pthread_build, NULL);
    if(ret != 0){
        printf("[Thread] Create gps pthread error!\n");
        //exit(1);
        return;
    }
    return;
}



/********************************GPIO start*******************************
 * TX  GPIO.25 BCM:26 BOARD:37
 * RX  GPIO.26 BCM:12 BOARD:32
 *************************************************************************/
void init_gpio()
{
    /// TX
    system("gpio -g mode 26 out");
    system("gpio -g write 26 1");
    /// RX
    system("gpio -g mode 12 out");
    system("gpio -g write 12 1");
}

void reset_hackrf_tx()
{
    system("gpio -g write 26 0");
    sleep(1);
    system("gpio -g write 26 1");
}

void reset_hackrf_rx()
{
    system("gpio -g write 12 0");
    sleep(1);
    system("gpio -g write 12 1");
}



/*************************************************************************
 *                                Broadcast
 *************************************************************************/
void pthread_receive_broadcast(void)
{
    struct sockaddr_in peeraddr,ia;  
    int sockfd; 
    char recmsg[BUFLEN + 1]; 
    unsigned int socklen, n; 
    struct ip_mreq mreq; 
    time_t timep;

    printf("[Thread] receive broadcast start\n");

    sockfd = socket (AF_INET, SOCK_DGRAM, 0); 
    if (sockfd < 0)
    {          
        printf("[Thread] socket creating err in udptalk\n");          
        exit (1);        
    } 
    bzero(&mreq, sizeof (struct ip_mreq)); 
    
    inet_pton(AF_INET,"224.0.0.1",&ia.sin_addr);
    bcopy (&ia.sin_addr.s_addr, &mreq.imr_multiaddr.s_addr, sizeof (struct in_addr));  

#if BROADCAST_E
    mreq.imr_interface.s_addr = htonl (INADDR_ANY);  
#else
    mreq.imr_interface.s_addr = inet_addr("192.168.12.1");//// 20170523_zjchen_add for any address AP 
#endif

    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,sizeof (struct ip_mreq)) == -1)
    {     
        perror("[Thread] setsockopt");          
        exit (-1);   
    }

    socklen = sizeof (struct sockaddr_in); 
    memset (&peeraddr, 0, socklen); 
    peeraddr.sin_family = AF_INET;
    peeraddr.sin_port = htons (54320);
    inet_pton(AF_INET, "224.0.0.1", &peeraddr.sin_addr); 

    if (bind(sockfd, (struct sockaddr *) &peeraddr,sizeof (struct sockaddr_in)) == -1)
    {      
        printf("[Thread] Bind error\n");      
        exit (0);    
    }
  
    printf("[Thread] Start receive broadcast\n");
    for (;;)
    {     
        //20221209_zjchen_add for receive broadcast set freq   start
        bzero (recmsg, BUFLEN + 1);     

        n = recvfrom (sockfd, recmsg, BUFLEN, 0, (struct sockaddr *) &peeraddr, &socklen);

        if (n < 0)
        {      
            printf("[Thread] recvfrom err in udptalk!\n");  
            continue;    
            //exit (4);    
        }
        else
        {    
            time_t timep;
            time (&timep);

            recmsg[n] = 0;      
            printf ("[Thread] Receive broadcast peer data : %s, Time:%s\n", recmsg, asctime(gmtime(&timep)));    

            //char tmp_data[50] = "broadcast freq:1000000,8000000,6000000,";
            //if(strstr(tmp_data, "broadcast freq") != NULL)
            if(strstr(recmsg, "broadcast freq") != NULL)
            //if(1)
            {
                //printf("[Thread] Receive broadcast data : in for()\n");
                
                FILE *fpfreq = fopen("./config_freq", "w");
                if (fpfreq == 0) 
                { 
                    printf("[Thread] Receive broadcast data : can't open file\n"); 
                    continue;
                }
                fseek(fpfreq, 0, SEEK_END);
                //printf("[Thread] @@@@@@@@@@@@@@@@@@@@@@@@@2\n");  
                char tmp_freq[50];

                for(int i = 0; i < 30; i++)
                {
                    //printf("%c", tmp_data[i + 15]);
                    printf("%c", recmsg[i + 15]);
                    //if(tmp_data[i + 15] == ' ')
                    if(recmsg[i + 15] == ' ')
                        break;
                    //tmp_freq[i] = tmp_data[i + 15];
                    tmp_freq[i] = recmsg[i + 15];
                }
                printf("\n");
                //printf("[Thread] *************************2\n");
                fwrite(tmp_freq, strlen(tmp_freq), 1, fpfreq);
                fclose(fpfreq); 
                printf("[Thread] Receive broadcast data : %s  Done!\n", tmp_freq);
            }
            else
            {
                printf("[Thread] Receive broadcast error : no freq!\n");
            }
        }
        sleep(1);
        //20221209_zjchen_add for receive broadcast set freq   end
    }

}

void pthread_send_broadcast(void)
{
    struct     sockaddr_in peeraddr, myaddr; 
    int        sockfd;  
    char       recmsg[BUFLEN + 1];
    unsigned   int socklen;
    unsigned   char mac[6];
    char mac_c[20] = {0};

    socklen = sizeof (struct sockaddr_in);  

    printf("[Thread] send broadcast start\n");

    sockfd = socket (AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {      
        printf ("[Thread] socket creating error\n");     
        exit (1);   
    }  
  
    memset (&peeraddr, 0, socklen); 
    peeraddr.sin_family = AF_INET;  
    peeraddr.sin_port = htons (54320);
    inet_pton (AF_INET, "224.0.0.1", &peeraddr.sin_addr);

    memset (&myaddr, 0, socklen); 
    myaddr.sin_family = AF_INET; 
    myaddr.sin_port = htons (23456); 

    //// SSID:My_AP  IP:192.168.12.1
#if BROADCAST_E
    myaddr.sin_addr.s_addr = htonl (INADDR_ANY);  
#else
    myaddr.sin_addr.s_addr = inet_addr("192.168.12.1");//// 20170523_zjchen_add for any address AP 
#endif

    if (bind (sockfd, (struct sockaddr *) &myaddr,     sizeof (struct sockaddr_in)) == -1)
    {     
        printf("[Thread] Bind error\n");     
        exit (0);  
    }
    //getMac("enxb827ebfbafd9", mac);
#if RASPBERRY
    getMac(eth_name, mac);
#else
    getMac("wlp2s0", mac);
#endif
    //getMac("wlan0", mac);
/*    printf("%02x:%02x:%02x:%02x:%02x:%02x\n", 
            (unsigned   char)mac[0], 
            (unsigned   char)mac[1], 
            (unsigned   char)mac[2], 
            (unsigned   char)mac[3], 
            (unsigned   char)mac[4], 
            (unsigned   char)mac[5]); */
//    printf("%02x\n", (unsigned   char)mac[0]); 
//    printf("%d\n", (int)mac[0] / 16);
    //printf("%s\n", a2x((int)mac[0] / 16)); 
    //printf("%d\n", (int)mac[0] % 16); 
    memset(mac_c, 0, 20); 
    x2c(mac, mac_c);
    printf("[Thread] MAC:%s\n", mac_c); 
/*
for (i=0; i<2; i++)
{
ddh = 48 + mac[i] / 16;
ddl = 48 + mac[i] % 16;
if (ddh > 57) ddh = ddh + 7;
if (ddl > 57) ddl = ddl + 7;
str[i*2] = ddh;
str[i*2+1] = ddl;
}

str[2] = '\0';
*/
//printf("!!!%s\n", str); 

//return;


    for (;;)
    {      
        char str[10] = "";

        bzero (recmsg, BUFLEN + 1);   
//printf ("bzero!\n");   
     /*   if (fgets (recmsg, BUFLEN, stdin) == (char *) EOF)    
        {
            printf ("stdin EOF!\n"); 
            exit (0);
        }
  */
        strcpy(recmsg, mac_c);
        /*strcat(recmsg, ':');
        strcat(recmsg, (unsigned   char)mac[1]);
        strcat(recmsg, ':');
        strcat(recmsg, (unsigned   char)mac[2]);
        strcat(recmsg, ':');
        strcat(recmsg, (unsigned   char)mac[3]);
        strcat(recmsg, ':');
        strcat(recmsg, (unsigned   char)mac[4]);
        strcat(recmsg, ':');
        strcat(recmsg, (unsigned   char)mac[5]);*/
        //strcpy(recmsg, "Hello");
        //strcat(recmsg, "/Spectrum_SUN/");
        strcat(recmsg, "/");        
        strcat(recmsg, boardcast_name);
        strcat(recmsg, "/");

        /// GPS
        if(lon != -1)
        {
            gcvt((float)lon / 1000000, 9, str);
        }
        else
        {
            gcvt((float)lon, 9, str);
        }
        //printf("%f, %s\n", (float)lon / 1000000, str);
        //strcat(recmsg, "20.060285");
        strcat(recmsg, str);
        strcat(recmsg, "/");
        if(lon != -1)
        {
            gcvt((float)lat / 1000000, 8, str);
        }
        else
        {
            gcvt((float)lat, 8, str);
        }
        //strcat(recmsg, "110.324653");
        strcat(recmsg, str);
        strcat(recmsg, "/");

        /// Height
        if(height != -1)
        {
            gcvt((float)height / 10, 4, str);
        }
        else
        {
            gcvt((float)height, 4, str);
        } 
        //strcat(recmsg, "2.0");   
        strcat(recmsg, str);
        strcat(recmsg, "/");

        /// Temp--INS
        /*gcvt((float)temp / 10, 1, str);
        strcat(recmsg, str);
        strcat(recmsg, "/");*/
        /// Temp--DHT11
        //strcat(recmsg, "35.6");
        strcat(recmsg, temp);
        strcat(recmsg, "/");
        //strcat(recmsg, "49");
        strcat(recmsg, hum);
        strcat(recmsg, "/");

        /// Light
        //gcvt((float)light, 10, str);
        strcat(recmsg, light);
        strcat(recmsg, "/");

        /// Angle       
        if(height != -1)
        {
            gcvt((float)angle / 10, 4, str);
        }
        else
        {
            gcvt((float)angle, 4, str);
        }
        //strcat(recmsg, "200.2");
        strcat(recmsg, str);
        strcat(recmsg, "/");

        /// Occupation
        strcat(recmsg, s_bandoccupation);
        strcat(recmsg, "/");

        ///Current
        strcat(recmsg, cc);
        strcat(recmsg, "/");

        ///Voltage
        strcat(recmsg, vv);
        strcat(recmsg, "/");

        ///Power
        strcat(recmsg, pp);
        strcat(recmsg, "/");

        ///iq
        get_max_pow_iq_distance();
        strcat(recmsg, iq);
        strcat(recmsg, "/");

        if (sendto(sockfd, recmsg, strlen (recmsg), 0, (struct sockaddr *) &peeraddr, sizeof (struct sockaddr_in)) < 0)
        {      
            printf("[Thread] sendto error!\n");      
            //exit (3);    
        }
        else
        {
            printf("[Thread] Broadcast:%s\n", recmsg);
        }
        
//        printf("'%s' send ok\n", recmsg);
sleep(3);
    } 

}

void create_receive_broadcast_thread()
{
    pthread_t id;
    int       ret;

    ret = pthread_create(&id, NULL, (void *)pthread_receive_broadcast, NULL);
    if(ret != 0){
        printf("[Thread] Create receive pthread error!\n");
        //exit(1);
        return;
    }
    return;
}

void create_send_broadcast_thread()
{
    pthread_t id;
    int       ret;

    ret = pthread_create(&id, NULL, (void *)pthread_send_broadcast, NULL);
    if(ret != 0){
        printf("[Thread] Create send pthread error!\n");
        //exit(1);
        return;
    }
    return;
}


/****************************************************************************
 *                                database  export
 ****************************************************************************/

void pthread_database_state()
{
    struct sockaddr_in server_addr; 
    bzero(&server_addr, sizeof(server_addr)); 
    server_addr.sin_family = AF_INET; 
#if BROADCAST_E
    server_addr.sin_addr.s_addr = htons(INADDR_ANY); 
#else
    server_addr.sin_addr.s_addr = inet_addr("192.168.12.1");
#endif
    server_addr.sin_port = htons(DATABASE_PORT); 
 
    //// Create socket
    int server_socket_fd = socket(PF_INET, SOCK_STREAM, 0); 

    if(server_socket_fd < 0) 
    { 
        perror("[Thread] Create Socket Failed:"); 
        return;//exit(1); 
    } 
  
    int opt = 1; 
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); 
  
    //// Band socket and the addr structure of socket 
    if(-1 == (bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)))) 
    { 
        perror("[Thread] Server Bind Failed:"); 
        return;//exit(1); 
    } 
  
    //// Listen socket
    if(-1 == (listen(server_socket_fd, LENGTH_OF_LISTEN_QUEUE))) 
    { 
        perror("[Thread] Server Listen Failed:"); 
        return;//exit(1); 
    } 
    //// Addr structure of socket 

    struct sockaddr_in client_addr; 
    socklen_t client_addr_length = sizeof(client_addr); 
 
    //// Receive connect, return one new socket for communication with client 
    //// Accept, write client infor into client_addr

    while(!isFinished)
    {
        int new_server_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, &client_addr_length); 
    	//printf("22222222222Database is exporting!\n");
    	if(new_server_socket_fd < 0) 
    	{ 
            perror("[Thread] Server Accept Failed:"); 
    	} 
    	//printf("Database is exporting!\n");
        char buffer[40] = "";
        bzero(buffer,40);
        if(!dataexport)
        {
            strcat(buffer, "Database is exporting!");
        }
        else
        {
            strcat(buffer, "Database export finished!");
            isFinished = true;
            printf("Database is finished!\n");
        }
        if(send(new_server_socket_fd, buffer, 40, 0) < 0) 
        { 
            printf("[Thread] Send Failed.\n"); 
            continue; 
        }  
        close(new_server_socket_fd);
    }
    //close(new_server_socket_fd);
    close(server_socket_fd);
}

void create_send_database_state_thread()
{
    pthread_t id;
    int       ret;

    ret = pthread_create(&id, NULL, (void *)pthread_database_state, NULL);
    if(ret != 0){
        printf("[Thread] Create send pthread error!\n");
        //exit(1);
        return;
    }
    return;
}


/****************************************************************************
 *                                get memory
 ****************************************************************************/
int get_memory(char freememory[10], char usedmemory[10])
{
    char command[50] = "df -h > fdisk";
    system(command);
    //int i = 0;
    FILE* fp=fopen("fdisk", "r");
    char buf[1024];
    //char ID[20] = "/dev/root";
    char ID[20] = "/dev/sda10";
    while(fgets(buf, 1024, fp) != NULL)
    { 
        char *p = (char *)malloc(10);
        p = strstr(buf, ID);
        char memory[10] = "";
        char used[10] = "";
        
        if (p!=NULL)
        {   

            int i = 0;
            int count = 0;
            //// Get ethernet interface name. Such as "enxb827ebfbafd9"
            for(;;)
            {
            	if(buf[i] == 'G')
            	{
                	break;
            
            	}
                i++;
            }   
            //// Point to ":Ethernet"
            for(;;)
            {
            	if(buf[i] == ' ')
            	{
                	break;
            	}
            	i++;
            } 
            for(;;)
            {
            	if(buf[i] == 'G')
            	{
                    used[count] = '\0';
                	break;
            	}
                used[count] = buf[i];
                count++;
            	i++;
            }
            count = 0;
	    for(;;)
            {
            	if(buf[i] == ' ')
            	{
                	break;
            	}
            	i++;
            }
	
            for(;;)
            {
            	if(buf[i] == 'G')
            	{
                	memory[count] = '\0';
                	break;
            	}
            	memory[count] = buf[i];
            	count++;
            	i++;
             } 
            
             int len = strlen(used);
             int m,n = 0;
             //char usedmemory[10] = "";
             for(m = 0;m < len; m++)
             {
            	 if(used[m]!= ' ')
            	 {
	             usedmemory[n] = used[m];
                     n++;
       		 }
            	 //usedmemory[n] = 'G';
     	     }
             printf("[Main] usedmemory:%s %s\n", used, usedmemory); 
             len = strlen(memory);
             m,n = 0;
             //char freememory[10] = "";
             for(m = 0;m < len; m++)
             {
            	 if(memory[m]!= ' ')
            	 {
 		     freememory[n] = memory[m];
                     n++;
       		 }
            	 //freememory[n] = 'G';
     	     }
             printf("[Main] freememory:%s %s\n", memory, freememory); 
        
             // close file
             fclose(fp); 
             return 0;
        }
    } 
    return 0;      
}

/****************************************************************************
 *                                update code
 ****************************************************************************
int pthread_update_version()
{
    char init_version[150] = "";

    char command[100] = "";
    char version[150] = "";

    FILE *fp = NULL;
    // Open file and read data 
    fp = fopen("ver.json", "r"); 
    if(NULL == fp) 
    { 
        printf("[Thread] File:ver.json Not Found\n"); 
        return 0;
    } 
    else
    { 
        bzero(init_version, 150); 

        //fread(init_version, sizeof(char), 150, fp);
        if((fread(init_version, sizeof(char), 150, fp)) == 0)
        {
            //printf("QQQQQQQQQQQQQQQ");
            return 0;
        }
    }
    fclose(fp);
    printf("version_info is %s\n", init_version);

    cJSON* root = cJSON_CreateObject();
    root = cJSON_Parse(init_version);
    int initCode = cJSON_GetObjectItem(root, "verCode")->valueint;
    printf("version_info is %d\n", initCode);
    
    cJSON_Delete(root);

    //char buffer[150] = ""; 
    FILE *fp1 = NULL;
    strcat(command, "curl http://10.0.18.217:8080/spectrum/update/Linux/ver.json >ver.json");
    system(command);
    
    // Open file and read data 
    fp1 = fopen("ver.json", "r"); 
    if(NULL == fp1) 
    { 
        printf("[Thread] File:ver.json Not Found\n"); 
        return 0;
    } 
    else
    { 
        bzero(version, 150); 
        fread(version, sizeof(char), 150, fp1);
    }

    cJSON* root1 = cJSON_CreateObject();
    root1 = cJSON_Parse(version);
    int latestCode = cJSON_GetObjectItem(root1, "verCode")->valueint;

    char *verFil = cJSON_GetObjectItem(root1, "verFile")->valuestring;
    printf("version_info is %s\n", verFil);

    printf("version_info is %d\n", latestCode);
    fclose(fp1);

    
    if(latestCode > initCode)
    { 
        printf("The version is not the latest.Please update!\n");
        char *verFil = cJSON_GetObjectItem(root1, "verFile")->valuestring;
        printf("version_info is %s\n", verFil);
        int filecount = cJSON_GetObjectItem(root1, "verFilecount")->valueint;
        printf("filecount is %d\n", filecount);
        //cJSON_Delete(root1);
        char update[15] = "";
        //printf("%d\n", filecount);
        int j = 0;
        int m = 0;
        for(int i = 0;i < filecount;i ++)
        {
            //printf("%d\n", i);
            //int m = 0;
            //int j = 0;
            bzero(update, 15);
            int count = 0;
            for(j = m; ; j++)
    	    {
                
                //int count = 0;
    	        if(verFil[j] == '\n')
        	{
            	    update[count] = '\0';
            	    break;
        	}
        	update[count] = verFil[j];
        	count++;
                //printf("%s\n", update); 
    	    }
            printf("%s\n", update);
            
            printf("%d\n", j);
            m = j + m + 1;
            printf("%d\n", m);
            char command_update[150] = "";
            strcat(command_update, "curl http://10.0.18.217:8080/spectrum/update/Linux/chattochat/");
            strcat(command_update, update);
            strcat(command_update, ">");
            strcat(command_update, update);
            system(command_update);
            printf("%s\n", command_update); 
            
        }
        sleep(1);
        system("make main");
        sleep(1);
        system("make spectrum0");
	sleep(1);
        system("make spectrum1");
	sleep(1);
        system("make spectrum2");
	sleep(1);
        system("make spectrum3");
	sleep(1);
        system("make localsocket");
	sleep(1);
        system("make occulocalsocket");
	sleep(1);
        system("make");
    sleep(1);
        system("make cleanall");
        system("reboot");
 
    }
    else
    {
        printf("The version is the latest!\n"); 
      
    }

    return 1;
}


void create_update_version_thread()
{
    pthread_t id;
    int       ret;

    ret = pthread_create(&id, NULL, (void *)pthread_update_version, NULL);
    if(ret != 0){
        printf("[Thread] Create update version pthread error!\n");
        //exit(1);
        return;
    }
    return;
}

/****************************************************************************

 *                                update code
 ****************************************************************************/
int pthread_iq()
{
    system("python3 ./sample.py");
    return 1;
}

void create_scan_iq_thread()
{
    pthread_t id;
    int       ret;

    ret = pthread_create(&id, NULL, (void *)pthread_iq, NULL);
    if(ret != 0){
        printf("[Thread] Create iq pthread error!\n");
        //exit(1);
        return;
    }
    return;
}

/****************************************************************************

 *                                reboot code

 ****************************************************************************/
int pthread_scan_iq_reboot()
{
    system("python3 ./reboot.py");
    return 1;
}

void create_scan_iq_reboot_thread()
{
    pthread_t id;
    int       ret;

    ret = pthread_create(&id, NULL, (void *)pthread_scan_iq_reboot, NULL);
    if(ret != 0){
        printf("[Thread] Create scan iq reboot pthread error!\n");
        //exit(1);
        return;
    }
    return;
}

/****************************************************************************
 *                       20220404 backup data
 ****************************************************************************/
int pthread_backup_data()
{
    system("python3 ./backup.py");
    return 1;
/*
    int result = 0;
    char buffer[1024] = ""; 

    FILE *fp = NULL;

    ////20230308_zjchen_add for server ip
    //printf("@@@\n");
  	char command_ping[50] = "ping ";
    strcat(command_ping, server_ip);
    strcat(command_ping, " -c 1 > ping_data");
    //printf("%s\n",command_ping);
    //return 0;
    do
    {
        system(command_ping);
        //printf("$$$\n");
        //system("ping 192.168.2.102 -c 1 > ping_data");
        sleep(1);
        //printf("###\n");
        // Open ping file and read data 
        fp = fopen("./ping_data", "r"); 
        if(NULL == fp) 
        { 
            printf("[Thread] File: ping Not Found\n"); 
        } 
        else
        { 
            bzero(buffer, 1024); 

            fread(buffer, sizeof(char), 1024, fp); 
            char* tmp = strtok(buffer, "");
            char* p = "1 received";
            //printf("%s\n", tmp);
            //printf("[Thread] File: ping 1\n"); 
            if(strstr(tmp, p))
            {
                result = 1;
                printf("[Thread] File: ping success !!!\n"); 
            }
        } 
        // close file
        fclose(fp); 
    }while(result == 0);
    printf("@@@\n");
    ////20230308_zjchen_add for server ip

    //// 20220404 backup data thread
  	char command_mount[1024] = "mount -t cifs -o username=TEST,password=1q2w3e4r //";
    strcat(command_mount, server_ip);
    strcat(command_mount, "/Share /mnt > mnt_data");
    //system("mount -t cifs -o username=TEST,password=1q2w3e4r //192.168.2.102/Share /mnt > mnt_data");
    //system(command_mount);
    //printf("%s\n",command_mount);
    //return 0;

    ////20230308_zjchen_remove for mount
    /*
    // Open mnt file and read data 
    fp = fopen("./mnt_data", "r"); 
    if(NULL == fp) 
    { 
        printf("[Thread] File: mnt Not Found\n"); 
    } 
    else
    { 
        bzero(buffer, 1024); 

        fread(buffer, sizeof(char), 1024, fp); 
        char* tmp = strtok(buffer, "");
        char* p = "failed:";
        if(strstr(tmp, p))
        {
            printf("[Thread] mnt false !!!\n"); 
            return 0;
        }
    }
    printf("[Thread] File: mnt success !!!\n"); 
    system("./data_copy.sh");
    printf("[Thread] File: backup data end !!!\n"); 
    return 1;
*/
}

void create_backup_data_thread()
{
    pthread_t id;
    int       ret;

    ret = pthread_create(&id, NULL, (void *)pthread_backup_data, NULL);
    if(ret != 0){
        printf("[Thread] Create backup data pthread error!\n");
        //exit(1);
        return;
    }
    return;
}

/*******************************************************
 *******************************************************
 *                    Main loop function               *
 *******************************************************
 *******************************************************/
int main(int argc,char *argv[]) 
{

    char      buf[MAX_LINE];
    FILE      *fp;
    int       len;
    char      *array[1]  = {"quit"};

    int       socket_fd, connect_fd;  
    struct    sockaddr_in     servaddr;  
    char      buff[4096] = "";//// "Build target GPS:110.123,20.123\n";  
    int       n;  
    char      time_log[20] = "";

    //int       sdr_num = 0;
    char      bus[5] = "";

    //// 20221222_zjchen_modify get boardcast ethernet first
    //// Get Ethernet name
    get_AP_boardcast_name();
    get_ethernet_interface_name();


    //// 20221222_zjchen_modify for create ap first
#if AP
    create_ap_thread();  
    sleep(3);/// Waite for create AP
    int result_scan = 0;
    do
    {
        char buffer_config[1024] = ""; 
        FILE *fp = NULL;
        fp = fopen("./config_scan", "r"); // Open ping file and read data 
        //printf("@@@\n");
        if(NULL == fp) 
        { 
            printf("[main] File: config_scan Not Found\n"); 
        } 
        else
        { 
            bzero(buffer_config, 1024); 
            fread(buffer_config, sizeof(char), 1024, fp); 
            char* tmp = strtok(buffer_config, "");
            char* p = "on";
            if(strstr(tmp, p))
            {
                result_scan = 1;
                printf("[main] File: start scan !!!\n"); 
            }
        } 
        fclose(fp); // close file
        sleep(10);
    }while(result_scan == 0);
#endif


    //// Get Hackrf serial

    //create_update_version_thread();
    hackrf_device(bus, arg_device);
    sdr_num = hackrf_serial(arg_array); 
    printf("[Main] SDR0 Device:%s Serial:%s\n", arg_device[0], arg_array[0]);
    printf("[Main] SDR1 Device:%s Serial:%s\n", arg_device[1], arg_array[1]);
    printf("[Main] SDR2 Device:%s Serial:%s\n", arg_device[2], arg_array[2]);
    printf("[Main] SDR3 Device:%s Serial:%s\n", arg_device[3], arg_array[3]);
    printf("[Main] SDR4 Device:%s Serial:%s\n", arg_device[4], arg_array[4]);

    //// Save Log
    open_log();
    inset_log("Start script.");
    close_log();
#if URAT
    wiringPiSetup();
#endif
    //// Reset Hackrf
    //reset_hackrf();


    //system("rdate -s -u time.nist.gov");
    system("hwclock -s");
    system("rm -r /var/log/syslog");
    system("rm -r /var/log/kern.log");
    //// Init Angle
#if URAT
    urat_fd = open_uart();
    //create_get_angle_thread();
    creat_urat_data_thread();

#endif
    //sleep(1);


    //// broadcast
    //create_receive_broadcast_thread();
    create_send_broadcast_thread();
    create_check_hackrf_num_thread();

    create_scan_iq_thread();


    ////20230308_zjchen_add for server ip
    char buffer_ip[1024] = ""; 
    FILE *fp_ip = fopen("./config_ip", "r");//Open ping ip 
    if(NULL == fp_ip) 
    { 
        printf("[main] File: config_scan Not Found\n"); 
    } 
    else
    { 
        bzero(buffer_ip, 1024); 
        fread(buffer_ip, sizeof(char), 1024, fp_ip); 
        server_ip = strtok(buffer_ip, " ");
    } 
    fclose(fp_ip); // close file
    //printf("%s\n",server_ip);
    //exit(0);

    create_backup_data_thread();
    //exit(0);


    /******************* databse start ******************
    if(init_database() == 0)
    {
        return 0;
    }

    if(Check_table("Tasktable") == 0)
    {
        create_task_table(); 
    }
   
    //// Semvalue. Create signal
    sem_id = semget((key_t)1234, 1, 0666 | IPC_CREAT); 

    //// Semvalue. First use and init signal
    if(!set_semvalue())
    {
        fprintf(stderr, "Failed to initialize semaphore\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        //// Scan spectrum start
        for(int i = 0; i < sdr_num; i++)
        {
            char s_step[1024];
            int  num = select_task_table(arg_array[i], start_freq, s_step, stop_freq);

            printf("[Main] sdr_num %d, num %d\n", sdr_num, num);

            if(num == 1)
            {
                printf("\n======================== scan spectrum ========================\n");
                printf("[Main] scan spectrum. num:%d, serial:%s\n", num, arg_serial);
             
                

                create_scan_spectrum_thread(&i);

                create_getsensordata_thread(&i);

                sleep(2);
            }
            else
            {
                printf("[Thread] Serial Number %s has not task before !\n", arg_array[i]);
            }
        }
    }
    /******************* databse end ******************/

#if URAT
    //// Open GPIO
    init_gpio();
#endif

    /**************************************************
     ******************* TEST start *******************
     **************************************************
            printf("\n======================== Direction Finding ========================\n"); 
            /*char tmp[BUFFER_SIZE] = "DF 710000000 740000000 \n";
            get_freq_serial(tmp);
            for(int i = 0; i < 4; i++)
            {
                strcpy(arg_serial, arg_array[i]);
                printf("Direction: i:%d, serial:%s\n", i, arg_serial);
                close_terminal_by_localsocket(i);
                create_scan_spectrum_thread(&i);
                sleep(2);
            }*/
//            create_direction_finding_thread();
    /**************************************************
     ******************* TEST end *********************
     **************************************************/


    create_scan_iq_reboot_thread();


    //// Init receive command Socket  
    if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
    {  
        printf("[Main] create socket error: %s(errno: %d)\n",strerror(errno),errno);  
        exit(0);  
    }  
  
    memset(&servaddr, 0, sizeof(servaddr));  
    servaddr.sin_family = AF_INET;  
#if BROADCAST_E
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);/// Local IP 
#else
    servaddr.sin_addr.s_addr = inet_addr("192.168.12.1");/// AP IP
#endif
    servaddr.sin_port = htons(DEFAULT_PORT);/// Set PORT  
  
    //// Band addr to socket  
    if(bind(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
    {  
        printf("[Main] bind socket error: %s(errno: %d)\n",strerror(errno),errno);  
        exit(0); 
        return 0;  
    } 
 
    //// Listen socket  
    if(listen(socket_fd, 10) == -1)
    {  
        printf("[Main] listen socket error: %s(errno: %d)\n",strerror(errno),errno);  
        exit(0);  
        return 0;
    }  

    while(1)
    {  
        /*scanf("%s", buf);
        printf("%s\n", buf);
        if(!strcmp(buf, array[0]))
        {
            break;
        }*/

        //// Wait for connection  
        if( (connect_fd = accept(socket_fd, (struct sockaddr*)NULL, NULL)) == -1)
        {  
            printf("[Main] accept socket error: %s(errno: %d)",strerror(errno),errno);  
            continue;  
        } 
 
        //// Receive data from client  
        n = recv(connect_fd, buff, MAXLINE, 0); 
 
        //// Send information to client  
        /*if(!fork())
        { 
            if(send(connect_fd, "Hello,you are connected!\n", 26,0) == -1)  
            perror("send error");  
            close(connect_fd);  
            exit(0);  
        }  */
        buff[n] = '\0'; 
        printf("[Main] recv msg from client: %s\n", buff);  

        //// Send control command
        if(strstr(buff, "control") != NULL)
        {
            //char command[50] = "./signal_replay.py ";
            char command[BUFFER_SIZE] = "hackrf_transfer -d ";

            strcat(command, sdr_id);
            if(strstr(buff, "up") != NULL)
            {
                printf("[Main] It is up\n");  
                //strcat(command, "/media/rikirobot/6753ec6b-7b52-43f6-af04-785cf3300d34/record_command/up.cfile");
                //system ("hackrf_transfer -d 0000000000000000954863c8334e4d23 -t ./signal/up.iq -f 921000000 -s 12500000 -a 1 -l 40 -x 25");
                strcat(command, " -t ./signal/up.iq");
            }
            else if(strstr(buff, "down") != NULL)
            {
                printf("[Main] It is down\n");  
                //strcat(command, "/media/rikirobot/6753ec6b-7b52-43f6-af04-785cf3300d34/record_command/down.cfile");
                //system ("hackrf_transfer -d 0000000000000000954863c8334e4d23 -t ./signal/down.iq -f 921000000 -s 12500000 -a 1 -l 40 -x 25");
                strcat(command, " -t ./signal/down.iq ");
            }
            else if(strstr(buff, "left_rotate") != NULL)
            {
                printf("[Main] It is left_rotate\n");  
                //strcat(command, "/media/rikirobot/6753ec6b-7b52-43f6-af04-785cf3300d34/record_command/left_rotate.cfile");
                //system ("hackrf_transfer -d 0000000000000000954863c8334e4d23 -t ./signal/left_rotate.iq -f 921000000 -s 12500000 -a 1 -l 40 -x 25");
                strcat(command, " -t ./signal/left_rotate.iq ");
            }
            else if(strstr(buff, "right_rotate") != NULL)
            {
                printf("[Main] It is right_rotate\n"); 
                //strcat(command, "/media/rikirobot/6753ec6b-7b52-43f6-af04-785cf3300d34/record_command/right_rotate.cfile"); 
                //system ("hackrf_transfer -d 0000000000000000954863c8334e4d23 -t ./signal/right_rotate.iq -f 921000000 -s 12500000 -a 1 -l 40 -x 25");
                strcat(command, " -t ./signal/right_rotate.iq ");
            }
            else if(strstr(buff, "forward") != NULL)
            {
                printf("[Main] It is forward\n");  
                //strcat(command, "/media/rikirobot/6753ec6b-7b52-43f6-af04-785cf3300d34/record_command/forward.cfile");
                //system ("hackrf_transfer -d 0000000000000000954863c8334e4d23 -t ./signal/forward.iq -f 921000000 -s 12500000 -a 1 -l 40 -x 25");
                strcat(command, " -t ./signal/forward.iq ");
            }
            else if(strstr(buff, "back") != NULL)
            {
                printf("[Main] It is back\n");  
                //strcat(command, "/media/rikirobot/6753ec6b-7b52-43f6-af04-785cf3300d34/record_command/back.cfile");
                //system ("hackrf_transfer -d 0000000000000000954863c8334e4d23 -t ./signal/back.iq -f 921000000 -s 12500000 -a 1 -l 40 -x 25");
                strcat(command, " -t ./signal/back.iq ");
            }
            else if(strstr(buff, "left") != NULL)
            {
                printf("[Main] It is left\n");  
                //strcat(command, "/media/rikirobot/6753ec6b-7b52-43f6-af04-785cf3300d34/record_command/left.cfile");
                //system ("hackrf_transfer -d 0000000000000000954863c8334e4d23 -t ./signal/left.iq -f 921000000 -s 12500000 -a 1 -l 40 -x 25");
                strcat(command, " -t ./signal/left.iq ");
            }
            else if(strstr(buff, "right") != NULL)
            {
                printf("[Main] It is right\n");  
                //strcat(command, "/media/rikirobot/6753ec6b-7b52-43f6-af04-785cf3300d34/record_command/right.cfile");
                //system ("hackrf_transfer -d 0000000000000000954863c8334e4d23 -t ./signal/right.iq -f 921000000 -s 12500000 -a 1 -l 40 -x 25");
                strcat(command, " -t ./signal/right.iq ");
            }   
            strcat(command, " -f 921000000 -s 12500000 -a 1 -l 40 -x 25");        
            system(command);
        }
        if(strstr(buff, "Quit scan") != NULL)
        {
            isSend = false;
            //isScan = false;

            printf("[Main] It is Quit scan, quit scan thread\n");  
            pthread_cancel(scan_thread_id);
#if URAT
            reset_hackrf_rx();
            reset_hackrf_tx();
#endif
            //printf("[Main] It is Quit scan, recreate scan thread\n"); 
            //create_scan_spectrum_terminal_thread();
        }
        if(strstr(buff, "GPS alert start") != NULL)
        {
            printf("[Main] It is GPS alert area signal thread\n");  
            bzero(gps_file, BUFFER_SIZE);
            strcpy(gps_file, "gpssim_beijin.bin");
            create_gps_thread();
        }
        if(strstr(buff, "Arrest start") != NULL)
        {
            printf("[Main] It is GPS arrest signal thread\n");  
            bzero(gps_file, BUFFER_SIZE);
            strcpy(gps_file, "gpssim.bin");
            create_gps_thread();
        }
        if(strstr(buff, "GPS alert stop") != NULL || strstr(buff, "Arrest stop") != NULL)
        {
            printf("[Main] Kill GPS arrest signal thread\n");  
            pthread_cancel(gps_thread_id);
#if URAT
            reset_hackrf_tx();
#endif
        }
        if(strstr(buff, "Build target GPS") != NULL)
        {
            //char lon[20] = "";
            //char lat[20] = "";
            int  count = 0;
 
            bzero(target_lat, BUFFER_SIZE);
            bzero(target_lon, BUFFER_SIZE);

            for(; ; count++)
            {
                if(buff[count] == ':')
                    break;
            }
            count++;
            for(int i = 0; ; i++)
            {
                target_lat[i] = buff[count];
                count++;
                if(buff[count] == ',')
                    break;
            }
            count++;
            for(int i = 0; ; i++)
            {
                target_lon[i] = buff[count];
                count++;
                if(buff[count] == '\n')
                    break;
            }
            printf("[Main] Get target GPS:%s,%s\n", target_lat, target_lon);  
            create_build_thread();
        }

        if(strstr(buff, "Get serial") != NULL)
        {
            char s[1024] = "";

            for(int i = 0; i < 5; i++)
            {
                strcat(s, arg_array[i]);
                if(i == 4)
                {
                    strcat(s, "\n");
                }
                else
                {
                    strcat(s, " ");
                }
            }
            if(send(connect_fd, s, 1024,0) == -1) 
            { 
                perror("send error");
            }
            printf("[Thread] send message success !\n");
        }



        //// Create task table 
        if(strstr(buff, "spectrum") != NULL)
        { 
            isSend = true;
            //get_freq_serial("spectrum 100000000 200000000 0000000000000000866863dc26581fcf\n");
            get_freq_serial(buff);
            int num = 0;
            for(; num < 5; num++)
            {
                //printf("@@@:serial, num, serial:%s %d %s\n", arg_serial, num, arg_array[num]);
                if(strstr(arg_serial, arg_array[num]) != NULL)
                {
                    //printf("@@@:serial, num, serial:%s %d %s\n", arg_serial, num, arg_array[num]);
                    break;
                }
            }
            //// for angle of uart
            arg_num = num;

            if(strcmp(start_freq,"0")!=0)
            {
                close_terminal_by_localsocket(num);
                //system("./localsocket SDR1 Q");


                create_scan_spectrum_thread(&num);
                create_getsensordata_thread(&num);
                //printf("@@@ command:, num:%d\n", num); 

                //// Start terminal receive thread
                create_scan_spectrum_terminal_thread();
            }
            else
            {

                create_scan_spectrum_terminal_thread();
            }
        }


        ////single point occupation
        if(strstr(buff, "threshold") != NULL)
        {   get_freq_serial(buff);
            //get_start_stop(buff);
            char thre_buffer[BUFFER_SIZE] = "";
            //f_occupancy(start_freq, "100000", stop_freq, 9, average, total, occupancy);
            //printf("Main:%s\n", threshold);
            char threshold_file[FILE_NAME_MAX_SIZE+1] = "threshold"; 
            FILE *fp = fopen(threshold_file, "r");
             
            if(NULL == fp) 
            { 
                printf("Thread: File:%s Not Found\n", threshold_file); 
            } 
            else
            { 
                bzero(thre_buffer, BUFFER_SIZE); 
                int length = 0; 
                // 每读取一段数据，便将其发送给客户端，循环直到文件读完为止 
                while((length = fread(thre_buffer, sizeof(char), BUFFER_SIZE, fp)) > 0) 
                { 
                    //printf("Thread: Sending data:%s\n", buffer); 
                    if(send(connect_fd, thre_buffer, length, 0) < 0) 
                    { 
                        printf("Thread: Send File:%s Failed.\n", threshold_file); 
                        break; 
                    } 
                    bzero(thre_buffer, BUFFER_SIZE); 
                }  
                fclose(fp); 
            }
            printf("Thread: File:%s Transfer Successful!\n", threshold_file); 
        }
        if(strstr(buff, "occupancy") != NULL)
        {   
            //f_occupancy(start_freq, "100000", stop_freq, 20);
            //printf("Main:%s\n", soccupancy);
	    char occu_buffer[BUFFER_SIZE] = "";
            char occupancy_file[FILE_NAME_MAX_SIZE+1] = "occupancy"; 
            FILE *fp = fopen(occupancy_file, "r");
             
            if(NULL == fp) 
            { 
                printf("Thread: File:%s Not Found\n", occupancy_file); 
            } 
            else
            { 
                bzero(occu_buffer, BUFFER_SIZE); 
                int length = 0; 
                // 每读取一段数据，便将其发送给客户端，循环直到文件读完为止 
                while((length = fread(occu_buffer, sizeof(char), BUFFER_SIZE, fp)) > 0) 
                { 
                    //printf("Thread: Sending data:%s\n", buffer); 
                    if(send(connect_fd, occu_buffer, length, 0) < 0) 
                    { 
                        printf("Thread: Send File:%s Failed.\n", occupancy_file); 
                        break; 
                    } 
                    bzero(occu_buffer, BUFFER_SIZE); 
                }  
                fclose(fp); 
            }
            printf("Thread: File:%s Transfer Successful!\n", occupancy_file); 
        }   


        ////multipoint occupation
        if(strstr(buff, "mloccupy") != NULL)
        {   
            multioccupation   =  1;
            char command[BUFFER_SIZE] = "./localsocket SDR";
            char num_c[2]             = "";

            isSend = true;
            //get_freq("spectrum 100000000 200000000 0000000000000000866863dc26581fcf\n");
            get_freq_serial(buff);

            int num = 0;
            for(; num < 5; num++)
            {
                if(strstr(arg_serial, arg_array[num]) != NULL)
                {
                    break;
                }
            }
            //// for angle of uart
            arg_num = num;
            
            if(strcmp(start_freq,"0")!=0)
            {
                close_terminal_by_localsocket(num);
                //system("./localsocket SDR1 Q");

                create_scan_spectrum_thread(&num);
                create_scan_spectrum_terminal_thread();
         
                
                pthread_t id0;
                int       ret0;
                ret0 = pthread_create(&id0, NULL, (void *)pthread_occulocalsocket, &arg_num);
                if(ret0 != 0)
                {
                    printf("Thread: Create occulocalsocket pthread error!\n");
                    return 0;
                }
            }
            else
            {
                create_scan_spectrum_terminal_thread();
            }      
        }


        //// Direction Finding
        if(strstr(buff, "DF") != NULL)
        {
            printf("\n======================== Reset freq for DF ========================\n");
            get_freq_serial("DF 710000000 740000000\n");
            for(int i = 0; i < 4; i++)
            {
                strcpy(arg_serial, arg_array[i]);
                close_terminal_by_localsocket(i);
                create_scan_spectrum_thread(&i);
            }

/*
    printf("\n======================== scan spectrum ========================\n");
    get_freq_serial("spectrum 710000000 740000000 0000000000000000a27466e627215b0f\n");
    num = 0;
    for(; num < 5; num++)
    {
        if(strstr(arg_serial, arg_array[num]) != NULL)
        {
            break;
        }
    }
    printf("[Main] scan spectrum. num%d, serial:%s\n", num, arg_serial);
    close_terminal_by_localsocket(num);
    create_scan_spectrum_thread(&num);

    printf("\n======================== scan spectrum ========================\n");
    get_freq_serial("spectrum 710000000 740000000 0000000000000000925866e62abd4de3\n");
    num = 0;
    for(; num < 5; num++)
    {
        if(strstr(arg_serial, arg_array[num]) != NULL)
        {
            break;
        }
    }
    printf("[Main] scan spectrum. num%d, serial:%s\n", num, arg_serial);
    close_terminal_by_localsocket(num);
    create_scan_spectrum_thread(&num);*

    create_direction_finding_thread();
*/
            printf("[Thread] send message success !\n");
        }
        //wiringPiSetup();
        if(strstr(buff, "relay") != NULL)
        {
           
           get_freq_serial(buff); 
           if(strcmp(relay_com,"initial") == 0)
           {
               //wiringPiSetup();
#if URAT
               pinMode(29, OUTPUT); 
               int a = digitalRead(29);
               printf("[Thread] Camera_Relay: %d\n", a);
               itoa(a, relay_mode);
#endif
               if(send(connect_fd, relay_mode, 10, 0) < 0) 
               { 
                   printf("Thread: Send Failed.\n"); 
                   return 0; 
               } 
               bzero(relay_mode, 10); 
           }

           if(strcmp(relay_com,"on") == 0)
           {
               system("gpio -g write 21 1");      
           }
           if(strcmp(relay_com,"off") == 0)
           {
               system("gpio -g write 21 0");     
           }  
      
        }


        if(strstr(buff, "Backup start") != NULL)
        {   
            char data_size[20] = "";
            //database_size(data_size);
            create_send_database_state_thread();
            char command[70] = "mysqldump -uroot -p1q2w3e4r spectrum > spectrum1.sql";
            system(command);
            dataexport = true;
        }

        if(strstr(buff, "Get filelength") != NULL)
        {   
            //f_occupancy(start_freq, "100000", stop_freq, 20);
            //printf("Main:%s\n", soccupancy);
	    char filelength_buffer[20] = "";
            char filelength_file[FILE_NAME_MAX_SIZE+1] = "spectrum.sql"; 
            long fileLength = 0;
            FILE *fp = fopen(filelength_file, "r");
            fseek(fp,0L,SEEK_END);
            fileLength=ftell(fp);   
            itoa(fileLength,filelength_buffer);
            printf("[Thread]: %s\n", filelength_buffer);
            if(send(connect_fd, filelength_buffer, 20, 0) < 0) 
            { 
                printf("[Thread]: Send filelength Failed.\n"); 
                return 0; 
            } 
            bzero(filelength_buffer, 20);        
            fclose(fp); 
            printf("Thread: Filelength Transfer Successful!\n"); 
        }



        if(strstr(buff, "Get backup") != NULL)
        {  
	    char backup_buffer[BUFFER_SIZE *20] = "";
            char backup_file[FILE_NAME_MAX_SIZE+1] = "spectrum.sql"; 
            FILE *fp = fopen(backup_file, "r");
            int SendFinish = 1;
            if(NULL == fp) 
            { 
                printf("Thread: File:%s Not Found\n", backup_file); 
            } 
            else
            { 
                bzero(backup_buffer, BUFFER_SIZE*20); 
                int length = 0; 
                // 每读取一段数据，便将其发送给客户端，循环直到文件读完为止 
                while((length = fread(backup_buffer, sizeof(char), BUFFER_SIZE*20, fp)) > 0) 
                { 
                    //printf("Thread: Sending data:%s\n", buffer); 
                    if(send(connect_fd, backup_buffer, length, 0) < 0) 
                    { 
                        printf("Thread: Send File:%s Failed.\n", backup_file); 
                        SendFinish = 0;
                        break; 
                    } 
                    bzero(backup_buffer, BUFFER_SIZE*20); 
                }  
                fclose(fp); 
            }
            if(SendFinish ==1)
            {
                printf("Thread: File:%s Transfer Successful!\n", backup_file); 
                //delete();
                system("reboot");
            }
        } 


        if(strstr(buff, "Delete database"))
        {
            //delete();
            system("reboot");

        }


        if(strstr(buff, "Get memory") != NULL)
        {   
            char freememory[10] = "";
            char usedmemory[10] = "";
            char memory_buffer[20] = "";
            get_memory(freememory, usedmemory);
            strcat(memory_buffer, freememory);
            strcat(memory_buffer, ",");
            strcat(memory_buffer, usedmemory);
            strcat(memory_buffer, "\n");
            if(send(connect_fd, memory_buffer, 20, 0) < 0) 
            { 
                printf("[Thread]: Send memorybuffer Failed.\n"); 
                return 0; 
            } 
            bzero(memory_buffer, 20);        
            
        }

        
  
        close(connect_fd); 
         
    }  
    close(socket_fd);  

}
