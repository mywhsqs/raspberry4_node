
//// Normal
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

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

//// Semun
#include <sys/stat.h>      
#include <sys/sem.h> 

//// Mac
#include <sys/ioctl.h> 
#include <netinet/in.h> 
#include <net/if.h> 

//// Math
#include <math.h>

//time
#include <sys/timeb.h>
#include <time.h>

#include "cJSON.h"
int main()
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

        fread(init_version, sizeof(char), 150, fp);
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


            //system("reboot");
            
        }
                    
        sleep(1);
        system("make main");
        system("make spectrum0");



       /*
        char command_update[150] = "";
        strcat(command_update, "curl --compressed http://10.0.18.217:8080/chattochat/update/Linux/chattochat/main.c >main.c");
        system(command_update);
        
        bzero(command_update, 150);
        strcat(command_update, "curl http://10.0.18.217:8080/chattochat/update/Linux/chattochat/spectrum.c >spectrum.c");
        system(command_update);

        bzero(command_update, 150);
        strcat(command_update, "curl http://10.0.18.217:8080/chattochat/update/Linux/chattochat/spectrum.c >spectrum1.c");
        system(command_update);
       */
    }
    else
    {
        printf("The version is the latest!\n"); 
      
    }

    return 0;
}



