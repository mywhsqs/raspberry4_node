
#include "api.h"

#define BUFFER_SIZE 1024

char      serial_num[50]  = {"0000000000000000866863dc26581fcf"};
char      device_num[10]  = "";
int       sdr_num         = -1;
char      start_freq[20]  = "700000000";
char      stop_freq[20]   = "800000000";
bool      isScan          = false;
char      task_name[20]   = "";
char      local_name[20]  = "SDR";
////Log
FILE      *fp_log         = NULL;

bool      isTerminal      = false;


////occuaption 
long      count = 0;
float     average[10000] = {0.0};
long      total[10000] = {0};
long      occupancy[10000] = {0};
char      occupation_start[20] = "600000000";
char      occupation_stop[20]  = "800000000";
char      s_bandoccupation[15] = ""; 
char      multioccupation[2]   = "";
char      occumode[5]          = "";


////sensor data
char temp[10] = "-1";
char hum[10]  = "-1";
char height[10] = "-1";
char light[10]  = "-1";

/***************************************************************
 *                           semvalue
 ***************************************************************/
union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *arry;
};

static int sem_id = 0;


static int set_semvalue()
{
    //用于初始化信号量，在使用信号量前必须这样做
    union semun sem_union;

    sem_union.val = 1;
    if(semctl(sem_id, 0, SETVAL, sem_union) == -1)
    {
        return 0;
    }
    return 1;
}

static void del_semvalue()
{
    //删除信号量
    union semun sem_union;

    if(semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
    {
        fprintf(stderr, "Failed to delete semaphore\n");
    }
}

static int semaphore_p()
{
    //对信号量做减1操作，即等待P（sv）
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1;//P()
    sem_b.sem_flg = SEM_UNDO;
    if(semop(sem_id, &sem_b, 1) == -1)
    {
        fprintf(stderr, "semaphore_p failed\n");
        return 0;
    }
    return 1;
}

static int semaphore_v()
{
    //这是一个释放操作，它使信号量变为可用，即发送信号V（sv）
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;//V()
    sem_b.sem_flg = SEM_UNDO;
    if(semop(sem_id, &sem_b, 1) == -1)
    {
        fprintf(stderr, "semaphore_v failed\n");
        return 0;
    }
    return 1;
}

/*******************************************************
*                    Log File
********************************************************/
int open_log()
{
    fp_log = fopen("log", "a+");
    if(fp_log == NULL)
    {
        printf("Log: Open log file fail !\n");
    }
    else
    {
        printf("Log: Open log file success !\n");
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


/********************************Occupation start*********************************/
int frequency_occuaption(char start_freq[20], char s_step[20], char stop_freq[20], FILE *fp, float band_threshold, char band_occuaption[15])
{ 
    double j = 0;
    long startfreq = atol(start_freq);
    long step = atol(s_step);
    long stopfreq = atol(stop_freq);
    long m = (stopfreq - startfreq) / step;
    //printf("%ld\n", startfreq);
    //printf("%ld\n", stopfreq);
    //printf("%ld\n", m);
    char *p1 = (char *)malloc(50);
    char *p2 = (char *)malloc(50);
    char pwr[50] = "";
    //char *e_pwr = (char *)malloc(100000);
    char s1[2],s2[2];
    s1[0] = ',';
    s2[0] = ';';
    s1[1]=s2[1]=0;
    char strLine[MAX_LINE];
    float power = 0.0;
    //FILE *fp;
    //fp = fopen ("send_scan_freq","r+"); 
    count++;
    //printf("Database: %ld\n", count);
    float channel_occupation[10000] = {0.0};
    for(int k=0; k<m; k++) 
    {
        fgets(strLine, MAX_LINE, fp);     //从文件中一行一行读取数据
        strLine[strlen(strLine)-1]='\0';
        p1 = strstr(strLine, s1);        //返回，和；的地址
        p2 = strstr(strLine, s2);
        if(p1&&p2)
        {
            p1++;
            bzero(pwr, 50);              //将pwr里面的值全部置0,防止出现脏数据
            strncpy(pwr, p1, p2 - p1);   //截取能量值
            //printf("Database: %s\n", pwr);
            power = atof(pwr);
            //printf("Database: %ld %ld\n", occupancy[k], total[k]);
            if(power > average[k])
            {
                //printf("Database: %ld %ld\n", occupancy[k], total[k]);
                //printf("Database: %f\n", power);
                occupancy[k]++;
                //printf("Database: %ld %ld\n", occupancy[k], total[k] + count);
            }
            channel_occupation[k] = (double)occupancy[k] / (count + total[k]);
            //printf("Database: %ld\n", count);
        }
        if(channel_occupation[k] > band_threshold)
        {
            j++; 
            //printf("Occupation: %lf %f\n", j, channel_occupation[k]);  
        }
    }
    if(strcmp(occumode, "0") == 0)    //0 history occuaption else current occupation
    { 
        double boccuaption = j / m; 
        gcvt(boccuaption, 9, band_occuaption);
        printf("%lf %s\n", boccuaption, band_occuaption);  
    }
    
    else
    { 
        int occu_frame = atoi(occumode);
        if(count % occu_frame == 0)
        {
            count = 0;
            for(int k=0; k<m; k++)
            {
                occupancy[k] = 0;
            }
            double boccuaption = j / m; 
            gcvt(boccuaption, 9, band_occuaption);
            printf("%lf %s\n", boccuaption, band_occuaption);  
        }  
    }
    return 0;
}
/********************************Occupation end*********************************/

/********************************get sensor data start*********************************/
int get_sensor_data(char* buffer)
{
    int count = 0;
    int i = 0;
    int j = 0;
    for(; ; i++)
    {
        if(buffer[i] == ',')
        {
            temp[count] = '\0';
            break;
        }
        temp[count] = buffer[i];
        count++;
    }
    printf("[Spectrum] temp:%s\n", temp);

    count = 0;
    for(i = i + 1; ; i++)
    {
        if(buffer[i] == ',')
        {
            hum[count] = '\0';
            break;
        }
        hum[count] = buffer[i];
        count++;
    }
    printf("[Spectrum] hum:%s\n", hum);

    count = 0;
    for(i = i + 1; ; i++)
    {
        if(buffer[i] == ',')
        {
            height[count] = '\0';
            break;
        }
        height[count] = buffer[i];
        count++;
    }
    printf("[Spectrum] height:%s\n", height);

    count = 0;
    for(i = i + 1; ; i++)
    {
        if(buffer[i] == '\n')
        {
            light[count] = '\0';
            break;
        }
        light[count] = buffer[i];
        count++;
    }
    printf("[Spectrum] light:%s\n", light);
    	
    return 1;
}
/********************************get sensor data end*********************************/

/***************************************************************
 *               scan spectrum command
 ***************************************************************/
void scan_spectrum(void)
{
    char command[200] = "soapy_power";
    char num_c[2]     = "";

    itoa(sdr_num, num_c);
    //strcat(command, num_c);
#if RASPBERRY
    strcat(command, " -r 2000000 -B 100000 -O scan_freq");
#else
    strcat(command, " -r 6000000 -B 100000 -O scan_freq");
#endif
    strcat(command, num_c);

    strcat(command, " -D constant -F rtl_power_fftw -n 1 -d serial=");
    strcat(command, serial_num);

    strcat(command, " -f ");
    strcat(command, start_freq);      //strcat是用于将两个char类型连接的函数
    strcat(command, ":");
    strcat(command, stop_freq);
    system(command);
    //printf("%s\n", command);
    printf("[Spectrum]: This scan pthread start...\n");
}

/***************************************************************
 *               scan spectrum and update database
 ***************************************************************/
void pthread_scan_spectrum_databse(void)
{
    char table_time[30] = "";
    char time_ms[10] = "";
    int  frame          = 1;
    int  insert_error   = 0;
    int  insert_return  = 1;

    char num_c[2]     = "";
    itoa(sdr_num, num_c);
    char copy[50]       = "cp scan_freq";
    strcat(copy, num_c); 
    strcat(copy, " send_scan_freq"); 
    strcat(copy, num_c);
    if(strcmp(multioccupation, "1") == 0)
    { 
        
        if(strcmp(occumode, "0") == 0)   //0  history occupation  else current occupation
        {   
            f_occupancy(start_freq, "100000", stop_freq, 9, average, total, occupancy);
            printf("[Spectrum] Occupation complete!");
        } 
        else
        {
            f_average(start_freq, "100000", stop_freq, 9, average);
            printf("[Spectrum] Average complete!");
        }
        
    }
    count = 0;
    while(isScan)
    {
        ////Scan spectrum use hackrf
        printf("[Spectrum]Thread: Start to scan spectrum\n");
        /******************* semvalue start ******************/
        //进入临界区
        if(!semaphore_p())
        {
            exit(EXIT_FAILURE);
        }
        scan_spectrum();
        system(copy);
        if(strcmp(multioccupation, "1") == 0)     //1 compute the bandoccupation in real time
        {
            char file_name[20] = "send_scan_freq"; 
            strcat(file_name, num_c);
            FILE *fp1 = fopen(file_name, "r"); 
            if(NULL == fp1) 
            { 
                 printf("[Spectrum]Thread: File:%s Not Found\n", file_name); 
            } 
            else
            {
                frequency_occuaption(start_freq, "100000", stop_freq, fp1, 0.1, s_bandoccupation);
                char command[BUFFER_SIZE] = "./occulocalsocket MAIN";
        	    strcat(command, num_c);
        	    strcat(command, " ");
        	    strcat(command, s_bandoccupation);
                system(command);
                printf("[Spectrum]localsocket: %s\n", command);
                
            }
            fclose(fp1);
        }
        if(!semaphore_v())
        {
            exit(EXIT_FAILURE);
        }
        /******************* semvalue end ******************/
        printf("[Spectrum]Thread: Stop to scan spectrum\n");

        time_t timer = time(NULL); 
        bzero(table_time, 30);
        strftime(table_time, 30, "%Y-%m-%d %H:%M:%S", localtime(&timer)); 
        struct timeb t;
    	ftime(&t);//ftime函数
    	long ms = t.millitm;
    	printf("time: %ld ms\n", ms);
    	itoa(ms, time_ms);
    	printf("time: %s ms\n", time_ms);
    	strcat(table_time,".");
    	strcat(table_time,time_ms);
    	printf("%s\n",table_time);
/*
        char command_data[BUFFER_SIZE] = "./getsensordata Sensor";
        strcat(command_data, num_c);
        strcat(command_data, " Getsensordata!");
        system(command_data);
        printf("[Spectrum]localsocket: %s\n", command_data);
*/      

        ////Insert spectrum data
        insert_return = insert_spectrum_table(task_name, table_time, temp, hum, height, light, start_freq, "100000", stop_freq, num_c);
        printf("[Spectrum]Thread: insert error num: %d\n", insert_error);

        ////Reset Hackrf
        if(insert_return == 1)////Insert error
        {
            char log[200] = "Insert database error. Error Frame ";
            char num[20]  = "";

            ////Insert error number
            itoa(insert_error, num);
            strcat(log, num);

            ////Insert scan frame
            strcat(log, ". Scan frame ");
            itoa(frame, num);
            strcat(log, num);
            strcat(log, ".");

            ////Insert SDR number
            strcat(log, local_name);
            strcat(log, ".");

            ////Save Log
            open_log();
            inset_log(log);
            close_log();

            printf("[Spectrum]Thread: insert error !\n");
            hackrf_reset(device_num);
            insert_error++;
        }

        printf("=====================================================================\n[Spectrum]Scan frame:%d, time:%s,\n", frame, table_time);
        //sleep(1);
        frame++;
    }

}

void pthread_getsensordata()
{
    char local_name[10] = "Sensor";
    char num_c[2]     = "";

    itoa(sdr_num, num_c);
    strcat(local_name,num_c);
    //while(1)
    //{
    	int       server_sockfd, client_sockfd;  
    	int       server_len, client_len;  
    	struct    sockaddr_un server_address; /*声明一个UNIX域套接字结构*/  
    	struct    sockaddr_un client_address;  
    	int       i, bytes;  
        int       result  = -1;
    	//char      ch_send[50]   = "";  
    	//char      ch_recv[50]   = "";
        char str_recv[50] = "";

        //unlink (local_name); /*删除原有server_socket对象*/  
        /*创建 socket, 通信协议为AF_UNIX, SCK_STREAM 数据方式*/  
        server_sockfd = socket (AF_UNIX, SOCK_STREAM, 0);    
        /*配置服务器信息(通信协议)*/  
    	server_address.sun_family = AF_UNIX;   
    	/*配置服务器信息(socket 对象)*/  
    	//printf("%s\n", local_name);
    	strcpy (server_address.sun_path, local_name);   
    	/*配置服务器信息(服务器地址长度)*/  
    	server_len = sizeof (server_address);   
    	/*绑定 socket 对象*/  
    	//bind (server_sockfd, (struct sockaddr *)&server_address, server_len);   
    	/*监听网络,队列数为5*/  
    	//listen (server_sockfd, 5);   
    	//printf ("Spectrum: Server is waiting for client connect...\n");  
    	//client_len = sizeof (client_address);   
    	//printf("%s\n", local_name);        
    	/*接受客户端请求; 第2个参数用来存储客户端地址; 第3个参数用来存储客户端地址的大小*/  
    	/*建立(返回)一个到客户端的文件描述符,用以对客户端的读写操作*/  
        /*向服务器发送连接请求*/  
/*
        result = connect (sockfd, (struct sockaddr *)&address, len);  
        if (result == -1) 
    	{  
        	printf("OccuLocalSocket: ensure the server is up\n");  
        	perror("OccuLocalSocket: connect");  
        	exit(EXIT_FAILURE);  
    	}
*/      
    	result = connect (server_sockfd, (struct sockaddr *)&server_address, server_len); 
        if(result == -1) 
    	{  
            perror("Spectrum: accept");  
            exit (EXIT_FAILURE);  
        } 
        sleep(1);
        while(1){
        char      ch_send[50]   = "Getsensordata!";
        if ((bytes = write(server_sockfd, ch_send, sizeof(ch_send))) == -1) 
        {  
            printf("SensorLocalSocket: ensure the server is up\n");  
        	perror("SensorLocalSocket: connect"); 
                continue; 
        	//exit(EXIT_FAILURE);   
        }
        //printf("main.c to spectrum.c data:%s\n", ch_send);
        
        sleep(1);
        if((bytes = read(server_sockfd, str_recv, sizeof(str_recv))) == -1) 
        {  
            perror("Spectrum: read");  
            //exit(EXIT_FAILURE); 
            continue; 
        }
        //printf("%s\n", str_recv);  
        get_sensor_data(str_recv);
        //close(server_sockfd);  
        //unlink(local_name);
    }
}

void create_getsensordata_thread()
{
    pthread_t id;
    int       ret;

    ret = pthread_create(&id, NULL, (void *)pthread_getsensordata, NULL);
    if(ret != 0){
        printf("[Spectrum]Thread: Create getsensordata pthread error!\n");
        //exit(1);
        return;
    }
    return;
}


void create_scan_spectrum_database_thread()
{
    pthread_t id;
    int       ret;

    ret = pthread_create(&id, NULL, (void *)pthread_scan_spectrum_databse, NULL);
    if(ret != 0){
        printf("[Spectrum]Thread: Create scan spectrum pthread error!\n");
        //exit(1);
        return;
    }
    return;
}




/*******************************************************
*                    Main loop function                *
********************************************************/
int main(int argc,char *argv[]) 
{
    int       num = 0;
    char      *array[4]  = {"./spectrum", "-s", "-n", "-d"}; 
    char      s_step[1024];
  
    /******************* socket start ******************/ 
    int       server_sockfd, client_sockfd;  
    int       server_len, client_len;  
    struct    sockaddr_un server_address; /*声明一个UNIX域套接字结构*/  
    struct    sockaddr_un client_address;  
    int       i, bytes;  
    char      ch_send, ch_recv; 
    /******************* socket end ******************/  

    if(argc == 9)
    {
        isTerminal = false;
        printf("[Spectrum] argc: %d\n", argc);
    }
    else if(argc == 13)
    {
        isTerminal = true;
        strcpy(start_freq, argv[7]);
        strcpy(stop_freq, argv[8]);
        strcpy(multioccupation, argv[10]);
        strcpy(occumode, argv[12]);
        printf("[Spectrum] argc: %d\n", argc);
        printf("[Spectrum] start_freq:%s\n", start_freq);
        printf("[Spectrum] stop_freq:%s\n", stop_freq);
        printf("[Spectrum] multioccupation:%s\n", multioccupation);
        printf("[Spectrum] occumode:%s\n", occumode);
    }
    else
    {
        printf("[Spectrum] Please input parameters !\n");
        return 1;
    }

    if(!strcmp(argv[1], array[1])) ////Serial
    {
        if(argv[2] == NULL)
        {
            printf("[Spectrum] Please input Serial Number !\n");
            return 1;
        }
        strcpy(serial_num, argv[2]);
        printf("[Spectrum] serial_num:%s\n", serial_num);
    }
    else
    { 
        printf("[Spectrum] Please look at the help !\n");
        return 1;
    }

    if(!strcmp(argv[3], array[2])) ////SDR num
    {
        char num_c[2] = "";

        if(argv[4] == NULL)
        {
            printf("[Spectrum] Please input SDR Number !\n");
            return 1;
        }
        strcpy(num_c, argv[4]);
        strcat(local_name, argv[4]);////Local socket name
        sdr_num = atoi(num_c);
        printf("[Spectrum] sdr_num:%d\n", sdr_num);
    }
    else
    { 
        printf("[Spectrum] Please look at the help !\n");
        return 1;
    }

    if(!strcmp(argv[5], array[3])) ////device num
    {
        if(argv[6] == NULL)
        {
            printf("[Spectrum] Please input Device Number !\n");
            return 1;
        }
        strcpy(device_num, argv[6]);
        printf("[Spectrum] device_num:%s\n", device_num);
    }
    else
    { 
        printf("[Spectrum] Please look at the help !\n");
        return 1;
    }
    ////Reset Hackrf
    hackrf_reset(device_num);


    /******************* semvalue start ******************/ 
    //创建信号量
    /*sem_id = semget((key_t)1234, 1, 0666 | IPC_CREAT);
    if(sdr_num == 0)
    {
        //程序第一次被调用，初始化信号量
        if(!set_semvalue())
        {
            fprintf(stderr, "Failed to initialize semaphore\n");
            exit(EXIT_FAILURE);
        }
        //sleep(2);
    }
    /******************* semvalue end ******************/ 
    create_getsensordata_thread();

    /******************* databse start ******************/
    if(init_database() == 0)
    {
        return 0;
    }

    if(!isTerminal)
    {
        char s_step[1024];
        int  num = select_task_table(serial_num, start_freq, s_step, stop_freq);
 
        ////Exit task before. Continue scan use last parameters.
        if(num == 1)
        {
            time_t timer = time(NULL); 
            bzero(task_name, 20);
            strftime(task_name, 20, "%Y%m%d%H%M%S", localtime(&timer)); 
            ////Update task table
            insert_task_table(task_name, serial_num, start_freq, "100000", stop_freq);
            ////Create new spectrum table
            create_spectrum_table(task_name, start_freq, "100000", stop_freq);

            ////Start scan spectrum thread
            isScan = true;
            create_scan_spectrum_database_thread();
        }
    }

    time_t timer = time(NULL); 
    bzero(task_name, 20);
    strftime(task_name, 20, "%Y%m%d%H%M%S", localtime(&timer)); 
    ////Update task table
    insert_task_table(task_name, serial_num, start_freq, "100000", stop_freq);
    ////Create new spectrum table
    create_spectrum_table(task_name, start_freq, "100000", stop_freq);
    ////Start scan spectrum thread
    isScan = true;
    create_scan_spectrum_database_thread();
    /******************* databse end ******************/


    /******************* socket start ******************/ 
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
    printf ("[Spectrum] Server is waiting for client connect...\n");  
    client_len = sizeof (client_address);   
    /*接受客户端请求; 第2个参数用来存储客户端地址; 第3个参数用来存储客户端地址的大小*/  
    /*建立(返回)一个到客户端的文件描述符,用以对客户端的读写操作*/  
    client_sockfd = accept(server_sockfd, (struct sockaddr *)&server_address, (socklen_t *)&client_len);  
    if(client_sockfd == -1) 
    {  
        perror("[Spectrum] accept");  
        exit (EXIT_FAILURE);  
    }  
    printf("[Spectrum] The server is waiting for client data...\n"); 
  
    while(1)
    {
        if((bytes = read(client_sockfd, &ch_recv, 1)) == -1) 
        {  
            perror("[Spectrum] read");  
            exit(EXIT_FAILURE);  
        }  
        sleep (1);  

        if(ch_recv == 'Q')
        {
            printf("[Spectrum] Message from client :%c\n", ch_recv);  
  
            ch_send = 'O';
            if ((bytes = write(client_sockfd, &ch_send, 1)) == -1) 
            {  
                perror("[Spectrum] read");  
                exit(EXIT_FAILURE);  
            }
            isScan = false;
            sleep(1);

            break;
        }
    }
    close(client_sockfd);  
    unlink(local_name);
    /******************* socket end ******************/ 
}


