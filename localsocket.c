#include <sys/types.h>  
#include <sys/socket.h>  
#include <stdio.h>  
#include <sys/un.h>  
#include <unistd.h>  
#include <stdlib.h>  
      
int main (int argc, char *argv[])  
{  
    struct    sockaddr_un address;  
    int       sockfd;  
    int       len;  
    int       i, bytes;  
    int       result;  
    char      ch_recv;  
    char      ch_send         = 'N';
    char      local_name[20]  = "SDR";
    //char      command         = 'N';
 
    if(argc != 3)
    {
        printf("LocalSocket: Please input parameters !\n");
        return 1;
    }
    if(argv[1] != NULL)
    {
        strcpy(local_name, argv[1]);
    }
    else
    { 
        printf("LocalSocket: Please look at the help !\n");
        return 1;
    }

    if(argv[2] != NULL)
    {
        ch_send = argv[2][0];////It is the char in char array
    }
    else
    { 
        printf("LocalSocket: Please look at the help !\n");
        return 1;
    }
     

    /*创建socket,AF_UNIX通信协议,SOCK_STREAM数据方式*/  
    if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {  
        perror ("LocalSocket: socket");  
        exit (EXIT_FAILURE);  
    }  
      
    address.sun_family = AF_UNIX;  
    strcpy (address.sun_path, local_name);  
    len = sizeof (address);  
      
    /*向服务器发送连接请求*/  
    result = connect (sockfd, (struct sockaddr *)&address, len);  
    if (result == -1) 
    {  
        printf("LocalSocket: ensure the server is up\n");  
        perror("LocalSocket: connect");  
        exit(EXIT_FAILURE);  
    }  
      
    //ch_send = 'Q'; 
    if((bytes = write(sockfd, &ch_send, 1)) == -1) //发消息给服务器
    { 
        perror ("LocalSocket: write");  
        exit (EXIT_FAILURE);  
    }  
   
    sleep (2); 
      
    if ((bytes = read(sockfd, &ch_recv, 1)) == -1) //接收消息
    { 
        perror("LocalSocket: read");  
        exit(EXIT_FAILURE);  
    }  
      
    printf("LocalSocket: receive from server data is %c\n", ch_recv);  
 
    close(sockfd);
    unlink(local_name);  
      
    return (0);  
}  
