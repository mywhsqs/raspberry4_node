#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>

#define BUFLEN 255

int
main (int argc, char **argv) 
{  
    struct sockaddr_in peeraddr,ia;  
    int sockfd; 
    char recmsg[BUFLEN + 1]; 
    unsigned int socklen, n; 
    struct ip_mreq mreq; 

    /* ���� socket ����UDPͨѶ */ 
    sockfd = socket (AF_INET, SOCK_DGRAM, 0); 
    if (sockfd < 0)
    {          
        printf ("socket creating err in udptalk\n");          
        exit (1);        
    } 
    /* ����Ҫ�����鲥�ĵ�ַ */ 
    bzero(&mreq, sizeof (struct ip_mreq)); 
    
    inet_pton(AF_INET,"224.0.1.2",&ia.sin_addr);
    /* �������ַ */ 
    bcopy (&ia.sin_addr.s_addr, &mreq.imr_multiaddr.s_addr, sizeof (struct in_addr)); 
    /* ���÷����鲥��Ϣ��Դ�����ĵ�ַ��Ϣ */ 
    mreq.imr_interface.s_addr = htonl (INADDR_ANY);  

    /* �ѱ��������鲥��ַ��������������Ϊ�鲥��Ա��ֻ�м���������յ��鲥��Ϣ */ 
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,sizeof (struct ip_mreq)) == -1)
    {     
        perror ("setsockopt");      
        exit (-1);   
    }

    socklen = sizeof (struct sockaddr_in); 
    memset (&peeraddr, 0, socklen); 
    peeraddr.sin_family = AF_INET;
    peeraddr.sin_port = htons (7838);
    inet_pton(AF_INET, "224.0.1.2", &peeraddr.sin_addr); 

    /* ���Լ��Ķ˿ں�IP��Ϣ��socket�� */ 
    if (bind(sockfd, (struct sockaddr *) &peeraddr,sizeof (struct sockaddr_in)) == -1)
    {      
        printf ("Bind error\n");      
        exit (0);    
    }
  
    /* ѭ�����������������鲥��Ϣ */ 
    for (;;)
    {     
        bzero (recmsg, BUFLEN + 1);     
        n = recvfrom (sockfd, recmsg, BUFLEN, 0, (struct sockaddr *) &peeraddr, &socklen);
        if (n < 0)
        {      
            printf ("recvfrom err in udptalk!\n");      
            exit (4);    
        }
        else{      
        /* �ɹ����յ����ݱ� */ 
            recmsg[n] = 0;      
            printf ("peer:%s", recmsg);    
        }
    
    }

}