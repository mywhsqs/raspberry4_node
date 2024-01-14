#ifndef  DATABASE_H
#define  DATABASE_H

#include <stdio.h>
#include <stdlib.h>  
#include <string.h> 
#include <time.h> 
#include <iconv.h>
#include <mysql/mysql.h>
#include <math.h>


#define MAX_LINE 10240
MYSQL *conn; // mysql 连接
MYSQL_RES *res_str; // mysql 记录集
MYSQL_ROW row; // 字符串数组，mysql 记录行

extern void itoa(long i,char* string);

extern int init_database(); 

extern int Check_table(char* table_name);

extern int create_task_table();

extern int insert_task_table(char table_name[20],char serial[50],char startfreq[20],char step[20],char stopfreq[20]); 

extern int select_task_table(char input_serial[50],char start_freq[20],char s_step[20],char stop_freq[20]);

extern int create_spectrum_table(char table_name[20],char start_freq[20],char s_step[20],char stop_freq[20]);

extern int insert_spectrum_table(char table_name[20], char table_time[20], char temp[10], char hum[10], char height[10], char light[10], char start_freq[20],char s_step[20], char stop_freq[20], char num[2]);

extern int select_time(char time[1000][30], char start_freq[20], char s_step[20], char stop_freq[20], char table_start[1000][20], char table_stop[1000][20], char min_table_start[20], char max_table_stop[20]);

extern int f_occupancy(char start_freq[20], char s_step[20], char stop_freq[20], float dB, float average[10000], long total[10000], long occupancy[10000]);

extern int f_average(char start_freq[20], char s_step[20], char stop_freq[20], float dB, float average[10000]);

extern int database_size(char data_size[20]);
 
extern int delete();


#endif



