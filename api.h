
//// Normal
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

//// Database
//#include <mysql/mysql.h>
//#include "database.h"

//// Reset hackrf
#include "hackrf_setting.h"

//// Socket
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/un.h> 
//#include <wiringPi.h>

//// Semun
#include <sys/stat.h>      
#include <sys/sem.h> 

//// Mac
#include <sys/ioctl.h> 
#include <netinet/in.h> 
#include <net/if.h> 

//// Math
#include <math.h>

//// CVP
#include <termios.h>

//time
#include <sys/timeb.h>
#include <time.h>

//#include "cJSON.h"

#define XFCE4  1 //// Xfce4 windows for debug mode
#define RASPBERRY 1 //// Raspberry
#define BROADCAST_E 1 //// Broadcast: 1.Ethernet  0.AP

#if RASPBERRY
#define URAT 0
#define AP   1 //// No AP now !
#define RK3568 0
#else
#define URAT 0
#define AP   1
#define RK3568 1
#endif

//// Angle
#if URAT
#include <wiringSerial.h> 
#include <wiringPi.h>
#endif 