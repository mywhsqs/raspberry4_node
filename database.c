
#include "database.h"




int init_database() 
{ 
    conn = mysql_init(NULL);

    if (mysql_real_connect(conn, "localhost", "root", "1q2w3e4r", "test", 0, NULL, CLIENT_MULTI_STATEMENTS)) {  
        printf("Database: Connection success!\n"); 
        int res = mysql_query(conn, "create database if not exists spectrum;");
        int res1 = mysql_query(conn, "use spectrum;"); 
        return 1;
    } else {  
        fprintf(stderr, "Database: Connection failed!\n");  
        if (mysql_errno(conn)) {  
            fprintf(stderr, "Database: Connection error %d: %s\n", mysql_errno(conn), mysql_error(conn));  
        }  
        exit(EXIT_FAILURE);  
    } 
    return 0;
}

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

/********************************create tasktable start*******************************/

int insert_task_table(char table_name[20],char serial[50],char startfreq[20],char step[20],char stopfreq[20])
{ 
    char insert_header[50] = "insert into Tasktable values('";
    char* insert_task = (char *)malloc(300);
    strcpy(insert_task,insert_header);
    strcat(insert_task,table_name);
    strcat(insert_task,"','");
    strcat(insert_task,serial);       //hackrf_serial
    strcat(insert_task,"','");
    strcat(insert_task,startfreq);
    strcat(insert_task,"','");
    strcat(insert_task,step);
    strcat(insert_task,"','");
    strcat(insert_task,stopfreq);
    strcat(insert_task,"');");
    int r = mysql_query(conn,insert_task);  
    if (!r) {  
        printf("Database: Inserted %lu rows\n", (unsigned long)mysql_affected_rows(conn));  
    } else {  
        fprintf(stderr, "Database: Insert error %d: %s\n", mysql_errno(conn), mysql_error(conn));  
    }
    return 0;  
}
  
int Check_table(char table_name[20])
{
    char header[20] = "select * from ";
    char* query = (char *)malloc(300);
    bzero(query,300);
    int exist;
    
    strcat(query, header);
    strcat(query, table_name);

    exist = mysql_query(conn, query);
    if (exist){ 
        printf("Database: Tasktable is not exist!\n"); 
    }
    else {
        printf("Database: Tasktable is exist!\n"); 
        res_str = mysql_store_result(conn);//保存查询结果
        return 1;
    }
    return 0;
}

int create_task_table()
{
    int tab = mysql_query(conn,"create table if not exists Tasktable(time varchar(25) PRIMARY KEY,serial varchar(50),startfreq int,step int,stopfreq int);");
    if (!tab) {  
        printf("Database: create tasktable success!\n"); 
        //insert_tasktable(table_name,startfreq,step,stopfreq); 
        return 1;
    } 
    else {  
        printf("Database: create table failed!\n");  
    }  
    return 0;  
}  
    


int select_task_table(char input_serial[50],char start_freq[20],char s_step[20],char stop_freq[20])
{
    char row_num[100] = "";
    char select_task[100] = "";
    int row_count = 0;
    char select_head[80] = "select count(*) from Tasktable where serial ='";
    char select_serial_num[100] = "";
    strcpy(select_serial_num,select_head);
    strcat(select_serial_num,input_serial);
    strcat(select_serial_num,"';");
    if (mysql_query(conn, select_serial_num)) {
        fprintf(stderr, "Database: %s\n", mysql_error(conn));
    }
    else{
        res_str = mysql_store_result(conn);//保存查询结果  
    }
    if(res_str){
        row = mysql_fetch_row(res_str);
    }
    strcpy(row_num,row[0]);
    row_count = atoi(row_num);
    if(row_count == 0) {
        return 0;
    }
    else {       
        char select_header[60] = "select * from Tasktable where serial ='";
        strcpy(select_task,select_header);
        strcat(select_task,input_serial);
        strcat(select_task,"';");
        printf("Database: %s\n",select_task);
        if (mysql_query(conn, select_task)){
            fprintf(stderr, "Database: %s\n", mysql_error(conn));
        }
        else{
            //printf("Database: %d\n",row_count);
            res_str = mysql_store_result(conn);//保存查询结果  
        }
        if(res_str){
            while(row = mysql_fetch_row(res_str))  {   
                if(row==NULL){  
                    break; 
                } 
                for(int t=0;t<mysql_num_fields(res_str);t++){
                //printf("Database: %s\n",row[t]);
                }
                strcpy(start_freq,row[2]);
                strcpy(s_step,row[3]);
                strcpy(stop_freq,row[4]);
                //printf("Database: %s\n",start_freq);
                //mysql_free_result(res_str); 
            }
            mysql_free_result(res_str);
        }
    }
    return 1;
}



/********************************create tasktable end*******************************/

/********************************create data tables start*******************************/
void create(char m_create[80000]) 
{
    int res = mysql_query(conn, m_create);
    if (!res) {  
        printf("Database: create table success!\n");  
    } 
    else {  
        printf("Database: create table failed!\n");  
    }   
}  


int create_freqpoint(long startfreq,long bandwidth,long stopfreq,char s_freqpoint[80000])
{
     char s_startfreq[20] = "";
     long b = (stopfreq - startfreq)/bandwidth;
     itoa(startfreq,s_startfreq);
     strcpy(s_freqpoint,s_startfreq);
     char freqpoint[80000] = "";
     long freq_point[80000];
     int i;
     for (i = 0;i < b;i++){
         freq_point[i] = startfreq + i * bandwidth;
         itoa(freq_point[i],freqpoint);
         if(i!=0) {
         strcat(s_freqpoint,"f ");
         strcat(s_freqpoint,"decimal(6,3),");
         strcat(s_freqpoint,freqpoint);
         }
     }
     strcat(s_freqpoint,"f decimal(6,3));");
     //free(freqpoint);   
     return 0;   
} 

int create_spectrum_table(char table_name[20],char start_freq[20],char s_step[20],char stop_freq[20])
{
    //创建数据
    char c_freqpoint[1024] = ""; 
    char d_freqpoint[100] = ""; 
    char a_freqpoint[80000] = "";     //建表  
    char b_create[80000] = "";        //建表
    char row_count[20] = ""; 
    char a_create[40] = "create table if not exists "; 
    strcpy(c_freqpoint,a_create);                   //建表
    //char table_name[256] = {0}; 
    //取当前时间 
    /*time_t timer = time(NULL);  
    strftime(table_name, sizeof(table_name), "%Y%m%d%H%M%S", localtime(&timer)); */
    strcpy(d_freqpoint,table_name);


    char row[10] = "R";  //列数
    strcat(d_freqpoint,row);
    strcat(c_freqpoint,d_freqpoint);                //建表
    long startfreq = atol(start_freq);
    long step = atol(s_step);
    long stopfreq = atol(stop_freq) ;
    long count = (stopfreq - startfreq)/step;     //一共多少行数据
    //判断需要创建几个表格
    long b = count%1000;    
    int k = 0;
    if(b==0){
        k = count/1000;
    }
    else{
        k = count/1000+1;
        //printf("Database: %d\n",k);
    }
    //连续创建k个表格
    for(int j = 0;j < k ;j++) {
        //printf("Database: %d\n",j);
        strcpy(a_freqpoint,c_freqpoint);//建表
        char i_create[150] = "(time varchar(255) PRIMARY KEY,temp varchar(255),humidity varchar(255),height varchar(255),light varchar(255),";//建表
        itoa(j,row_count);
        strcat(a_freqpoint,row_count);  //建表
        strcat(a_freqpoint,i_create);   //建表
        //printf("Database: %s\n",a_freqpoint);
        //创建1000行
        if(count-j*1000>=1000) { 
            create_freqpoint(startfreq+j*1000*step,step,startfreq+(j+1)*1000*step,b_create);
            strcat(a_freqpoint,b_create);   //建表
            create(a_freqpoint);            //创建表的语句  create函数
            //printf("Database: %s\n",a_freqpoint);
            //free(a_freqpoint);
        }
        //创建少于1000行
        else{
            create_freqpoint(startfreq+(k-1)*1000*step,step,stopfreq,b_create);
            strcat(a_freqpoint,b_create);
            create(a_freqpoint);
        }
    }
    return 0;  
}
/********************************create data tables end*******************************/



/********************************insert data start*******************************/
//插入数据    
int insert(char m_insert[100000]) 
{ 
    int res = mysql_query(conn, m_insert);
    //printf("Database: %d\n",res);
    if (!res) {  
        printf("Database: Inserted %lu rows\n", (unsigned long)mysql_affected_rows(conn));  
    } else {  
        fprintf(stderr, "Database: Insert error %d: %s\n", mysql_errno(conn), mysql_error(conn));
    }
    return res;  
}

//读取要插入的数据
int frequency_fileread(int m,char e_pwr[100000],FILE *fp)
{
    char *p1 = (char *)malloc(50);
    char *p2 = (char *)malloc(50);
    char pwr[50] = "";
    //char *e_pwr = (char *)malloc(100000);
    char s1[2],s2[2];
    s1[0] = ',';
    s2[0] = ';';
    s1[1]=s2[1]=0;
    char strLine[MAX_LINE];
    //FILE *fp;
    //fp = fopen ("scan_freq","r+"); 
    for(int k=0;k<m;k++) {
        fgets(strLine,MAX_LINE,fp);     //从文件中一行一行读取数据
        strLine[strlen(strLine)-1]='\0';
        p1 = strstr(strLine,s1);        //返回，和；的地址
        p2 = strstr(strLine,s2);
        if(p1&&p2){
            p1++;
            bzero(pwr, 50);              //将pwr里面的值全部置0,防止出现脏数据
            strncpy(pwr, p1, p2 - p1);   //截取能量值
            //printf("Database: %s\n",pwr);
        }
        if(k==0){
            strcpy(e_pwr,pwr);         //将所有能量值拼接成字符串
            //printf("Database: %s\n",e_pwr);
        }
        else{
            strcat(e_pwr,"','");
            strcat(e_pwr,pwr);
            //printf("Database: %s\n",e_pwr);   
        }
     }
     strcat(e_pwr,"');");
     return 0;
}

//拼接要插入数据的字符串语句
int insert_spectrum_table(char table_name[20], char table_time[20], char temp[10], char hum[10], char height[10], char light[10], char start_freq[20],char s_step[20], char stop_freq[20], char num[2]) 
{
    char a_insert[20] = "insert into "; 
    char b_insert[1000] = ""; 
    char c_insert[100000] = "";
    char i_row_count[100] = "";
    char i_row[10] = "R";  //列数
    int a = 1;
    //char table_name[256];
    //printf("Database: %s\n",table_name);
    strcpy(b_insert,a_insert);
    strcat(b_insert,table_name); 
    strcat(b_insert,i_row);
    FILE *fptr;
    char fname[10] = "scan_freq";
    strcat(fname, num);
    fptr = fopen (fname,"r+");
    printf("Database: %s\n",num);
    //取要插入的当前时间
    /*char table_time[256]={0}; 
    time_t timer = time(NULL);  
    strftime(table_time, sizeof(table_time), "%Y%m%d%H%M%S", localtime(&timer));*/

    long startfreq = atol(start_freq);
    long step = atol(s_step);
    long stopfreq = atol(stop_freq) ;
    long i_count = (stopfreq - startfreq)/step;
    long i_b = i_count%1000;
    int i_k = 0;
    if(i_b==0){
        i_k = i_count/1000;
    }
    else{
        i_k = i_count/1000+1;
    }
    for(int n = 0;n < i_k ;n++) {
        strcpy(c_insert,b_insert);      
        char i_insert[20] = " values ('";
        itoa(n,i_row_count);
        strcat(c_insert,i_row_count);     
        strcat(c_insert,i_insert);      

        strcat(c_insert,table_time);    //插入字段的时间

        strcat(c_insert,"','"); 
        strcat(c_insert,temp);
        strcat(c_insert,"','"); 
        strcat(c_insert,hum);
        strcat(c_insert,"','");
        strcat(c_insert,height);
        strcat(c_insert,"','"); 
        strcat(c_insert,light);
        strcat(c_insert,"','");
      
        if(i_count-n*1000>=1000) {
            char e_insert[100000] = ""; 
            frequency_fileread(1000,e_insert,fptr);  
            strcat(c_insert,e_insert);
            a = insert(c_insert);                       
        }
        else{
            char e_insert[100000] = ""; 
            long extra = (stopfreq-startfreq-(i_k-1)*1000*step)/step;
            frequency_fileread(extra,e_insert,fptr);
            strcat(c_insert,e_insert);
            a = insert(c_insert);                     
            //printf("Database: %s\n",c_insert);
        }
    }
    //free(c_insert);
    fclose(fptr);
    return a;
} 
/********************************insert data end*******************************/


/*******************************threshold and occupancy *******************************/

/*
//找到表中某一频点的数据,并求出平均值aver
float select_string(char table_time[30], char start_freq[20], char s_step[20], char stop_freq[20], char freq_point[20])
{
    //平均值计算
    float sum=0.0;
    float aver=0.0;

    char select_head[10] = "select ";
    long startfreq = atol(start_freq);
    long step = atol(s_step);
    long stopfreq = atol(stop_freq);
    long freqpoint = atol(freq_point);
    long count = (stopfreq - startfreq)/step;
    char select_str[1024] = "";
    char s_m[10] = "";

    //判断有几个表格    
    int k = 0;
    long m = 0;
    int n = 0;
    int s = 0;
    if(count%1000==0){
        k = count/1000;
    }
    else{
        k = count/1000+1;
        //printf("Database: %d\n",k);
    }
    
    for(int j=0;j<k;j++)
    {   
        long m_stopfreq = startfreq + 1*1000*step;
        if(freqpoint>=startfreq&&freqpoint<m_stopfreq)
        {
            m = j;
        }
        //printf("Database: %ld\n%ld\n%ld\n",startfreq,m_stopfreq,m);
        startfreq = startfreq + 1*1000*step;
        
        
        //printf("Database: %ld\n%ld\n%ld\n",startfreq,m_stopfreq,m);
        itoa(m,s_m);
    } 
    strcpy(select_str, select_head);
    strcat(select_str, freq_point);
    strcat(select_str, "f "); 
    strcat(select_str, "from ");
    strcat(select_str, table_time);
    strcat(select_str, "R");
    strcat(select_str, s_m);
    strcat(select_str, ";");
    //printf("%s\n", select_str);
    //free(freqpoint);
    if (mysql_query(conn, select_str))
    {
        fprintf(stderr, "Database: %s\n", mysql_error(conn));
    }
    else
    {
        //printf("Database: %d\n",row_count);
        res_str = mysql_store_result(conn);//保存查询结果  
    }
    if(res_str)
    {
        while(row = mysql_fetch_row(res_str))  
        {   
            n++;
            if(row == NULL)
            {  
                break; 
            } 
            for(int t=0;t<mysql_num_fields(res_str);t++)
            {
                float power_value = atof(row[t]);
                //printf("%f ", power_value);
                sum+=power_value;   //计算历史帧某一频点数据之和，求其平均值
            }
        }
        //printf("%f\n", sum);
        aver = sum/n;    
        mysql_free_result(res_str);
        //printf("%f\n", aver);
    }
    return aver;
}  
//计算历史帧数据各频点能量的平均值,平滑噪声,加上4dB,得到占用度的阈值
int frequency_aver(char table_time[30], char start_freq[20], char s_step[20], char stop_freq[20], float dB, float aver[10000], char average[300000], char table_start[20], char table_stop[20])
{
    long startfreq = atol(start_freq);
    long step = atol(s_step);
    long stopfreq = atol(stop_freq);
    long tablestart = atol(table_start);
    long tablestop = atol(table_stop);
    long freqpoint = startfreq;
    char freq_point[10000][20];
    memset(freq_point, 0, sizeof(freq_point));
    int i;
    char s_aver[10000][10];
    memset(s_aver, 0, sizeof(s_aver));
    strcpy(average, "threshold ");
    strcat(average, table_start);
    strcat(average, " ");
    strcat(average, table_stop);
    strcat(average, " ");
    if(startfreq < tablestart)
    {
        startfreq = tablestart;
    }
    if(stopfreq > tablestop)
    {
        stopfreq = tablestop;
    }
    long count = (stopfreq - startfreq)/step;
    printf("%d %d\n", startfreq, stopfreq);
    for(i=0;i<count;i++)
    {
        freqpoint = startfreq + i * step;
        itoa(freqpoint,freq_point[i]);
        aver[i]=select_string(table_time, table_start, s_step, table_stop, freq_point[i]);

    }

    //平滑噪声
    aver[0]=aver[1]=aver[100];
    for(int j=0;j<i-1;j++)
    {
        if(fabs(aver[j+1]-aver[j])>0.4)
        {
            aver[j+1] = aver[j];
        }
    }

    for(int m=0;m<count;m++)
    {
        aver[m] += dB;
        gcvt(aver[m], 9, s_aver[m]);
        strcat(average, freq_point[m]);
        strcat(average, ",");
        strcat(average, s_aver[m]);
        strcat(average, ";");
        //printf("average:%f ", aver[m]);
    }
    strcat(average, "\n");
    FILE *fp = fopen("sthreshold", "w+");
    if (fp==0) { printf("can't open file\n"); return 0;}
    fseek(fp, 0, SEEK_END);
    //char sz_add[] = "hello world\n";
    fwrite(average, strlen(average), 1, fp);
    fclose(fp);
    //strcat(average, "\0");

    return count;
}
int select_time(char time[1000][30], char start_freq[20], char s_step[20], char stop_freq[20], char table_start[1000][20], char table_stop[1000][20])
{
    long startfreq = atol(start_freq);
    long stopfreq = atol(stop_freq);
    char select_head[400] = "select time,startfreq,stopfreq from Tasktable where(startfreq<='";
    //char select_head[100] = "select * from Tasktable where serial='";
    strcat(select_head,start_freq);
    strcat(select_head,"' and '");
    strcat(select_head,start_freq);
    strcat(select_head,"'<=stopfreq)");
    strcat(select_head," or (startfreq<='");
    strcat(select_head,stop_freq);
    strcat(select_head,"' and '");
    strcat(select_head,stop_freq);
    strcat(select_head,"'<=stopfreq)");
    strcat(select_head," or (startfreq>='");
    strcat(select_head,start_freq);
    strcat(select_head,"' and stopfreq<='");
    strcat(select_head,stop_freq);
    printf("%s\n", select_head);
    int n = 0;
    //float average[10000] = {0.0};
    char t_start_freq[20] = "";
    char t_stop_freq[20] = "";
    if (mysql_query(conn, select_head))
    {
        fprintf(stderr, "Database: %s\n", mysql_error(conn));
    }
    else
    {
        res_str = mysql_store_result(conn);//保存查询结果  
    }
    if(res_str)
    {
        while(row = mysql_fetch_row(res_str))            //行数
        {   

            if(row==NULL)
            {  
                break; 
            } 
            for(int t=0;t<mysql_num_fields(res_str);t++)  //行中字段的数目
            {   
                if(t==0)
                {
                    strcpy(time[n], row[0]); 
                }
                if(t==1)
                {
                    strcpy(table_start[n], row[1]); 
                }
                if(t==2)
                {
                    strcpy(table_stop[n], row[2]); 
                }
                //printf("%s %s %s", row[0], row[1], row[2]);
            }
            n++;
        }
        //printf("%s\n",time[n-1]);
        mysql_free_result(res_str);
        
    }

    return n;
}



//计算占用度

int f_occupancy(char start_freq[20], char s_step[20], char stop_freq[20], char soccupancy[100000], char threshold[300000])
{
    char select_head[10] = "select ";
    long startfreq = atol(start_freq);
    long step = atol(s_step);
    long stopfreq = atol(stop_freq);
    long freqpoint;
    char freq_point[20] = ""; 
    //long count = (stopfreq - startfreq)/step;
    char select_str[1024] = "";
    char s_m[10] = "";   
    char s_occupancy[10000][10];
    memset(s_occupancy, 0, sizeof(s_occupancy));
    //char soccupancy[100000] = "";
    char time[1000][30];
    memset(time, 0, sizeof(time));
    //char average[100000] = "";
    float aver[10000] = {0.0};
    //float occupancy[10000] = {0.0};
    //char stable_start[1000][20] = "";
    //char stable_stop[1000][20]  = "";
    char table_start[1000][20];
    memset(table_start, 0, sizeof(table_start));
    char table_stop[1000][20];
    memset(table_stop, 0, sizeof(table_stop));
    long int total[10000] = {0};
    long int occupancy[10000] = {0};
    int table_number = select_time(time, start_freq, s_step, stop_freq, table_start, table_stop);   //查找出表的名字
    frequency_aver(time[table_number-2], start_freq, s_step, stop_freq, 4.0, aver, threshold, table_start[table_number-1], table_stop[table_number-1]);     //算出每一个频点的阈值
    //printf("Database: %s\n",average);
    
    strcpy(soccupancy, "occupancy ");
    strcat(soccupancy, table_start[table_number-1]);
    strcat(soccupancy, " ");    
    strcat(soccupancy, table_stop[table_number-1]); 
    strcat(soccupancy, " ");  
    long tablestart = atol(table_start[table_number-1]);
    printf("Database: %s\n",table_start[table_number-1]);
    long tablestop = atol(table_stop[table_number-1]);
    long t_count = (tablestop - tablestart)/step;
    //判断有几个表格    
    int k = 0;
    //int a = 0;
    int s = 0;
    //int m = 0;
    
    if(t_count%1000==0){
        k = t_count/1000;
    }
    else{
        k = t_count/1000+1;
        //printf("Database: %d\n",k);
    }
    
    if(startfreq < tablestart)
    {
        startfreq = tablestart;
    }
    if(stopfreq > tablestop)
    {
        stopfreq = tablestop;
    }
    long count = (stopfreq - startfreq)/step;
    for(int i=0;i<count;i++)
    {   
        
            
        freqpoint = startfreq + i * step;
        //printf("Database: %ld\n",freqpoint);
        itoa(freqpoint,freq_point);
        for(int t=table_number-2;t<table_number;t++)
        {
            for(int a=0;a<k;a++)
            {   int m = 0;
                long m_stopfreq = tablestart + 1*1000*step;
        	if(freqpoint>=tablestart&&freqpoint<m_stopfreq)
        	{
                    m = a;
                    //printf("Database: %d\n", m);
        	}
        	//printf("Database: %d\n", m);

            	tablestart = tablestart + 1*1000*step;

        
        	//printf("Database: %ld\n%ld\n%ld\n",tablestart,m_stopfreq,m);
        	itoa(m,s_m);
    	    } 
            strcpy(select_str, select_head);
            strcat(select_str, freq_point);
            strcat(select_str, "f "); 
            strcat(select_str, "from ");
            strcat(select_str, time[t]);
            strcat(select_str, "R");
            strcat(select_str, s_m);
            strcat(select_str, ";");
            //printf("%s\n", select_str);
            if (mysql_query(conn, select_str))
            {
                fprintf(stderr, "Database: %s\n", mysql_error(conn));
            }
            else
            {
                //printf("Database: %d\n",row_count);
                res_str = mysql_store_result(conn);//保存查询结果  
            }
            if(res_str)
            {
                while(row = mysql_fetch_row(res_str))  
                {   
                    total[i]++;
                    if(row == NULL)
                    {  
                        break; 
                    } 
                    for(int t=0;t<mysql_num_fields(res_str);t++)
                    {
                        //printf("%d\n", t);
                        float power_value = atof(row[t]);
                        if(power_value >= aver[i])
                        {
                            occupancy[i]++; 
                        }
                    }
                }
            }   
        }

        //printf("%d,%d\n", s, n); 
        char s_freqpoint[10] = "";
        itoa(freqpoint,s_freqpoint);
        char s_occu[10] = "";
        itoa(occupancy[i],s_occu);
        char s_total[10] = "";
        itoa(total[i],s_total);

            
        strcat(soccupancy, s_freqpoint);
      
        strcat(soccupancy, ",");
        strcat(soccupancy, s_occu); 
        strcat(soccupancy, " ");
        strcat(soccupancy, s_total); 
        strcat(soccupancy, ";");

        mysql_free_result(res_str);
        //m++;
    } 
    strcat(soccupancy, "\n");
    FILE *fp = fopen("occupancy", "w+");
    if (fp==0) { printf("can't open file\n"); return 0;}
    fseek(fp, 0, SEEK_END);
    fwrite(soccupancy, strlen(soccupancy), 1, fp);
    fclose(fp);
    
    return 0;
}  
*/

int select_time(char time[1000][30], char start_freq[20], char s_step[20], char stop_freq[20], char table_start[1000][20], char table_stop[1000][20], char min_table_start[20], char max_table_stop[20])
{
    long startfreq = atol(start_freq);
    long stopfreq = atol(stop_freq);
    char select_head[400] = "select time,startfreq,stopfreq from Tasktable where(startfreq<'";
    strcat(select_head,start_freq);
    strcat(select_head,"' and '");
    strcat(select_head,start_freq);
    strcat(select_head,"'<stopfreq)");
    strcat(select_head," or (startfreq<'");
    strcat(select_head,stop_freq);
    strcat(select_head,"' and '");
    strcat(select_head,stop_freq);
    strcat(select_head,"'<stopfreq)");
    strcat(select_head," or (startfreq>='");
    strcat(select_head,start_freq);
    strcat(select_head,"' and stopfreq<='");
    strcat(select_head,stop_freq);
    strcat(select_head,"');");
    printf("%s\n", select_head);
    
    long min_start = 0;
    long max_stop  = 0;
    int n = 0;
    long start[1000] = {0};
    long stop[1000] = {0};
    if (mysql_query(conn, select_head))
    {
        fprintf(stderr, "Database: %s\n", mysql_error(conn));
    }
    else
    {
        res_str = mysql_store_result(conn);//保存查询结果  
    }
    if(res_str)
    {
        while(row = mysql_fetch_row(res_str))            //行数
        {   

            if(row==NULL)
            {  
                break; 
            } 
            for(int t=0;t<mysql_num_fields(res_str);t++)  //行中字段的数目
            {   
                if(t==0)
                {
                    strcpy(time[n], row[0]); 
                    printf("%s ", time[n]);
                }
                if(t==1)
                {
                    strcpy(table_start[n], row[1]); 
                    start[n] = atol(table_start[n]);
                    if(n==0)
                    {
                        min_start = start[0];
                    }
                    if(start[n] < min_start)
        	    {
            		min_start = start[n];
        	    }
                }
                if(t==2)
                {
                    strcpy(table_stop[n], row[2]);
                    stop[n] = atol(table_stop[n]); 
                    if(n==0)
                    {
                        max_stop = stop[0];
                    }
                    if(stop[n] > max_stop)
        	    {
            		max_stop = stop[n];
        	    }
                }
            }
            n++;
        }
        itoa(min_start, min_table_start);
        itoa(max_stop, max_table_stop);
        printf("%s %s", min_table_start, max_table_stop);       
        mysql_free_result(res_str);
        
    }
    return n;
}



//计算占用度

int f_occupancy(char start_freq[20], char s_step[20], char stop_freq[20], float dB, float average[10000], long total[10000], long occupancy[10000])
{
    char select_head[10] = "select ";
    long startfreq = atol(start_freq);
    long step = atol(s_step);
    long stopfreq = atol(stop_freq);
    long freqpoint;
    char freq_point[20] = ""; 
    //long count = (stopfreq - startfreq)/step;
    char select_str[1024] = "";
    char s_m[10] = "";   
    char s_occupancy[10000][10];
    memset(s_occupancy, 0, sizeof(s_occupancy));
    long num[1000] = {0};
    char time[1000][30];
    memset(time, 0, sizeof(time));
    //float sum[1000][10000] = {0.0};
    //float average[10000] = {0.0};
    char table_start[1000][20];
    memset(table_start, 0, sizeof(table_start));
    char table_stop[1000][20];
    memset(table_stop, 0, sizeof(table_stop));
    float min_average = 0.0;
   
    char min_table_start[20] = "";
    char max_table_stop[20]="";
    int table_number = select_time(time, start_freq, s_step, stop_freq, table_start, table_stop, min_table_start, max_table_stop);   //查找出表的名字
    long min_start = atol(min_table_start);
    long max_stop  = atol(max_table_stop);
    if(startfreq < min_start)
    {
        startfreq = min_start;
    }
    if(stopfreq > max_stop)
    {
        stopfreq = max_stop;
    }
    long count = (stopfreq - startfreq)/step;
    printf("count: %ld %ld %ld\n", startfreq, stopfreq, count);
 
    float sum[10000] = {0.0};
    long int frame_number[10000] = {0};
    //float average[10000] = {0.0};
    char noise[300000] = "";
    char newstart[20] = "";
    char newstop[20]  = "";
    itoa(startfreq, newstart);
    itoa(stopfreq, newstop);
    strcpy(noise, "threshold ");
    strcat(noise, newstart);
    strcat(noise, " ");
    strcat(noise, newstop);
    strcat(noise, " ");


    for(int i=0; i<count; i++)
    {   
        long point = startfreq + i * step;
        //char spoint[20] = "";
        //itoa(point, spoint);
            
        
        for(int t = table_number-1; t < table_number; t++)
        {
            long tablestart = atol(table_start[t]);
    	    //printf("Database: %s %s %s\n", time[t], table_start[t], table_stop[t]);
    	    long tablestop = atol(table_stop[t]);
    	    long t_count = (tablestop - tablestart)/step;
            int j = 0;
            long start_number = startfreq;
            long stop_number  = stopfreq;
    	    //判断有几个表格    
	    int k = 0;
	    //int s = 0;
    
	    if(t_count%1000==0)
	    {
	        k = t_count/1000;
	    }
	    else
	    {
	        k = t_count/1000+1;
	        //printf("Database: %d\n",k);
	    }
    
	    if(start_number < tablestart)
	    {
                j = (tablestart - start_number)/step;
                //printf("%d\n", j);
	        start_number = tablestart;
	    }
	    if(stop_number > tablestop)
	    {
 	       stop_number = tablestop;
	    }
            freqpoint = start_number + i * step;
            //printf("Database: %ld\n",freqpoint);
            if(freqpoint>=tablestop)
            {
                break;
            }
            itoa(freqpoint,freq_point);
            //int j = (startfreq - min_start)/step;
            int m = 0;
            for(int a=0;a<k;a++)
            {   
                long m_stopfreq = tablestart + 1*1000*step;
        	if(freqpoint>=tablestart&&freqpoint<m_stopfreq)
        	{
                    m = a;
                    //printf("Database: %d\n", m);
        	}
        	//printf("Database: %d\n", m);

            	tablestart = tablestart + 1*1000*step;

        
        	//printf("Database: %ld\n%ld\n%ld\n",tablestart,m_stopfreq,m);
        	itoa(m,s_m);
    	    } 
            strcpy(select_str, select_head);
            strcat(select_str, freq_point);
            strcat(select_str, "f "); 
            strcat(select_str, "from ");
            strcat(select_str, time[t]);
            strcat(select_str, "R");
            strcat(select_str, s_m);
            strcat(select_str, ";");
            //printf("%s\n", select_str);
            if (mysql_query(conn, select_str))
            {
                fprintf(stderr, "Database: %s\n", mysql_error(conn));
            }
            else
            {
                //printf("Database: %d\n",row_count);
                res_str = mysql_store_result(conn);//保存查询结果  
            }
            if(res_str)
            {
                while(row = mysql_fetch_row(res_str))  
                {   
                    frame_number[i+j]++;
                    if(row == NULL)
                    {  
                        break; 
                    } 
                    for(int t=0;t<mysql_num_fields(res_str);t++)
                    {
                        //printf("%d\n", t);
                        float power_value = atof(row[t]);
                        
                        sum[i+j] += power_value;
                    }
                }
            }  
 
        }
        average[i] = (sum[i])/(frame_number[i]) + dB;
        min_average = average[0];
        if(min_average > average[i])
        {
            min_average = average[i];      
        }
       
        //printf("min_average:%f\n", min_average); 
    }
    average[0] = min_average;
    for(int i=0; i<count; i++)
    {
        long point = startfreq + i * step;
        char spoint[20] = "";
        itoa(point, spoint);
        if(i!=0)
        {
            if(fabs(average[i]-average[i-1])>0.4)
	    {
            	average[i] = average[i-1];
            }
        } 
        char s_aver[20] = "";
        bzero(s_aver,20);
        gcvt(average[i], 9, s_aver);
        strcat(noise, spoint);
        strcat(noise, ",");
        strcat(noise, s_aver);
        strcat(noise, ";");
    }

    mysql_free_result(res_str);
    strcat(noise, "\n");
    FILE *fp1 = fopen("threshold", "w+");
    if (fp1==0) { printf("can't open file\n"); return 0;}
    fseek(fp1, 0, SEEK_END);
    fwrite(noise, strlen(noise), 1, fp1);
    fclose(fp1);




    //long total[10000] = {0};
    //long occupancy[10000] = {0};
    char soccupancy[100000] = "";
    strcpy(soccupancy, "occupancy ");
    strcat(soccupancy, newstart);
    strcat(soccupancy, " ");
    strcat(soccupancy, newstop);
    strcat(soccupancy, " ");
    for(int i=0;i<count;i++)
    {   
        
            
	long point = startfreq + i * step;
        char spoint[20] = "";
        itoa(point, spoint);
        for(int t=table_number-2; t<table_number; t++)
        {
            
            long tablestart = atol(table_start[t]);
    	    //printf("Database: %s %s %s\n", time[t], table_start[t], table_stop[t]);
    	    long tablestop = atol(table_stop[t]);
    	    long t_count = (tablestop - tablestart)/step;
	    int j = 0;
            long start_number = startfreq;
            long stop_number  = stopfreq;
    	    //判断有几个表格    
	    int k = 0;
	    //int s = 0;
    
	    if(t_count%1000==0)
	    {
	        k = t_count/1000;
	    }
	    else
	    {
	        k = t_count/1000+1;
	        //printf("Database: %d\n",k);
	    }
    
	    if(start_number < tablestart)
	    {
                j = (tablestart - start_number)/step;
	        start_number = tablestart;
                //printf("Database: %d\n",j);
	    }
	    if(stop_number > tablestop)
	    {
 	       stop_number = tablestop;
            }
            freqpoint = start_number + i * step;
            //printf("Database: %ld\n",freqpoint);
            if(freqpoint>=tablestop)
            {
                break;
            }
            itoa(freqpoint,freq_point);
	    int m = 0;
            for(int a=0;a<k;a++)
            {   
                
                long m_stopfreq = tablestart + 1*1000*step;
        	if(freqpoint>=tablestart&&freqpoint<m_stopfreq)
        	{
                    m = a;
                    //printf("Database: %d\n", m);
        	}
        	//printf("Database: %d\n", m);

            	tablestart = tablestart + 1*1000*step;

        
        	//printf("Database: %ld\n%ld\n%ld\n",tablestart,m_stopfreq,m);
        	itoa(m,s_m);
    	    } 
            strcpy(select_str, select_head);
            strcat(select_str, freq_point);
            strcat(select_str, "f "); 
            strcat(select_str, "from ");
            strcat(select_str, time[t]);
            strcat(select_str, "R");
            strcat(select_str, s_m);
            strcat(select_str, ";");
            //printf("%s\n", select_str);
            if (mysql_query(conn, select_str))
            {
                fprintf(stderr, "Database: %s\n", mysql_error(conn));
            }
            else
            {
                //printf("Database: %d\n",row_count);
                res_str = mysql_store_result(conn);//保存查询结果  
            }
            if(res_str)
            {
                while(row = mysql_fetch_row(res_str))  
                {   
                    total[i+j]++;
                    if(row == NULL)
                    {  
                        break; 
                    } 
                    for(int t=0;t<mysql_num_fields(res_str);t++)
                    {
                        //printf("%d\n", t);
                        float power_value = atof(row[t]);
                        if(power_value >= average[i+j])
                        {
                            occupancy[i+j]++; 
                        }
                    }
                }
            }   
        }
        char s_occu[10] = "";
        itoa(occupancy[i],s_occu);
        char s_total[10] = "";
        itoa(total[i],s_total);
        //printf("occupancy:%ld %ld\n", s_occu, s_total);
            
        strcat(soccupancy, spoint);
      
        strcat(soccupancy, ",");
        strcat(soccupancy, s_occu); 
        strcat(soccupancy, " ");
        strcat(soccupancy, s_total); 
        strcat(soccupancy, ";");
        //printf("occupancy:%s %s %s\n", spoint, s_occu, s_total);
    } 
    mysql_free_result(res_str);
    strcat(soccupancy, "\n");
    FILE *fp = fopen("occupancy", "w+");
    if (fp==0) { printf("can't open file\n"); return 0;}
    fseek(fp, 0, SEEK_END);
    fwrite(soccupancy, strlen(soccupancy), 1, fp);
    fclose(fp);
    //printf("occupancy:%s\n", soccupancy);     
    return 0;

}   

//计算占用度

int f_average(char start_freq[20], char s_step[20], char stop_freq[20], float dB, float average[10000])
{
    char select_head[10] = "select ";
    long startfreq = atol(start_freq);
    long step = atol(s_step);
    long stopfreq = atol(stop_freq);
    long freqpoint;
    char freq_point[20] = ""; 
    //long count = (stopfreq - startfreq)/step;
    char select_str[1024] = "";
    char s_m[10] = "";   
    char s_occupancy[10000][10];
    memset(s_occupancy, 0, sizeof(s_occupancy));
    long num[1000] = {0};
    char time[1000][30];
    memset(time, 0, sizeof(time));
    //float sum[1000][10000] = {0.0};
    //float average[10000] = {0.0};
    char table_start[1000][20];
    memset(table_start, 0, sizeof(table_start));
    char table_stop[1000][20];
    memset(table_stop, 0, sizeof(table_stop));
    float min_average = 0.0;
   
    char min_table_start[20] = "";
    char max_table_stop[20]="";
    int table_number = select_time(time, start_freq, s_step, stop_freq, table_start, table_stop, min_table_start, max_table_stop);   //查找出表的名字
    long min_start = atol(min_table_start);
    long max_stop  = atol(max_table_stop);
    if(startfreq < min_start)
    {
        startfreq = min_start;
    }
    if(stopfreq > max_stop)
    {
        stopfreq = max_stop;
    }
    long count = (stopfreq - startfreq)/step;
    printf("count: %ld %ld %ld\n", startfreq, stopfreq, count);
 
    float sum[10000] = {0.0};
    long int frame_number[10000] = {0};
    //float average[10000] = {0.0};
    char noise[300000] = "";
    char newstart[20] = "";
    char newstop[20]  = "";
    itoa(startfreq, newstart);
    itoa(stopfreq, newstop);
    strcpy(noise, "threshold ");
    strcat(noise, newstart);
    strcat(noise, " ");
    strcat(noise, newstop);
    strcat(noise, " ");


    for(int i=0; i<count; i++)
    {   
        long point = startfreq + i * step;
        //char spoint[20] = "";
        //itoa(point, spoint);
            
        
        for(int t = table_number-2; t < table_number; t++)
        {
            long tablestart = atol(table_start[t]);
    	    //printf("Database: %s %s %s\n", time[t], table_start[t], table_stop[t]);
    	    long tablestop = atol(table_stop[t]);
    	    long t_count = (tablestop - tablestart)/step;
            int j = 0;
            long start_number = startfreq;
            long stop_number  = stopfreq;
    	    //判断有几个表格    
	    int k = 0;
	    //int s = 0;
    
	    if(t_count%1000==0)
	    {
	        k = t_count/1000;
	    }
	    else
	    {
	        k = t_count/1000+1;
	        //printf("Database: %d\n",k);
	    }
    
	    if(start_number < tablestart)
	    {
                j = (tablestart - start_number)/step;
                //printf("%d\n", j);
	        start_number = tablestart;
	    }
	    if(stop_number > tablestop)
	    {
 	       stop_number = tablestop;
	    }
            freqpoint = start_number + i * step;
            //printf("Database: %ld\n",freqpoint);
            if(freqpoint>=tablestop)
            {
                break;
            }
            itoa(freqpoint,freq_point);
            //int j = (startfreq - min_start)/step;
            int m = 0;
            for(int a=0;a<k;a++)
            {   
                long m_stopfreq = tablestart + 1*1000*step;
        	if(freqpoint>=tablestart&&freqpoint<m_stopfreq)
        	{
                    m = a;
                    //printf("Database: %d\n", m);
        	}
        	//printf("Database: %d\n", m);

            	tablestart = tablestart + 1*1000*step;

        
        	//printf("Database: %ld\n%ld\n%ld\n",tablestart,m_stopfreq,m);
        	itoa(m,s_m);
    	    } 
            strcpy(select_str, select_head);
            strcat(select_str, freq_point);
            strcat(select_str, "f "); 
            strcat(select_str, "from ");
            strcat(select_str, time[t]);
            strcat(select_str, "R");
            strcat(select_str, s_m);
            strcat(select_str, ";");
            //printf("%s\n", select_str);
            if (mysql_query(conn, select_str))
            {
                fprintf(stderr, "Database: %s\n", mysql_error(conn));
            }
            else
            {
                //printf("Database: %d\n",row_count);
                res_str = mysql_store_result(conn);//保存查询结果  
            }
            if(res_str)
            {
                while(row = mysql_fetch_row(res_str))  
                {   
                    frame_number[i+j]++;
                    if(row == NULL)
                    {  
                        break; 
                    } 
                    for(int t=0;t<mysql_num_fields(res_str);t++)
                    {
                        //printf("%d\n", t);
                        float power_value = atof(row[t]);
                        
                        sum[i+j] += power_value;
                    }
                }
            }  
 
        }
        average[i] = (sum[i])/(frame_number[i]) + dB;
        min_average = average[0];
        if(min_average > average[i])
        {
            min_average = average[i];      
        }
       
        //printf("min_average:%f\n", min_average); 
    }
    average[0] = min_average;
    for(int i=0; i<count; i++)
    {
        long point = startfreq + i * step;
        char spoint[20] = "";
        itoa(point, spoint);
        if(i!=0)
        {
            if(fabs(average[i]-average[i-1])>0.4)
	    {
            	average[i] = average[i-1];
            }
        } 
        char s_aver[20] = "";
        bzero(s_aver,20);
        gcvt(average[i], 9, s_aver);
        strcat(noise, spoint);
        strcat(noise, ",");
        strcat(noise, s_aver);
        strcat(noise, ";");
    }

    mysql_free_result(res_str);
    strcat(noise, "\n");
    FILE *fp1 = fopen("threshold", "w+");
    if (fp1==0) { printf("can't open file\n"); return 0;}
    fseek(fp1, 0, SEEK_END);
    fwrite(noise, strlen(noise), 1, fp1);
    fclose(fp1);    
    return 0;
} 

int database_size(char data_size[20])
{
    if (mysql_query(conn, "select concat(round(sum(DATA_LENGTH/1024/1024),2),'MB') as data_size from INFORMATION_SCHEMA.TABLES where table_schema='copy';")) {
        fprintf(stderr, "Database: %s\n", mysql_error(conn));
    }
    else{
        res_str = mysql_store_result(conn);//保存查询结果  
    }
    if(res_str){
        row = mysql_fetch_row(res_str);
    }
    strcpy(data_size,row[0]);
    //printf("%s\n", memory_size);
    return 0;
}


int delete() 
{ 
    int res = mysql_query(conn, "drop database spectrum;");
    //printf("Database: %d\n",res);
    if (!res) {  
        printf("Database: Delete database success!\n");  
    } else {  
        fprintf(stderr, "Database: Delete database error %d: %s\n", mysql_errno(conn), mysql_error(conn));
    }
    return res;  
}
