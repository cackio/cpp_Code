#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <iostream>

#define PORT 8000
#define MAXSIZE 1024

int main()
{
    int listenfd,clifd;
    char rec_buffer[MAXSIZE]= {0},send_buffer[MAXSIZE],str[INET_ADDRSTRLEN];

    struct sockaddr_in cli_addr= {0};
    struct sockaddr_in ser_addr = {0};
    socklen_t cli_len;

    if((listenfd=socket(AF_INET,SOCK_STREAM,0))<0)
    {
        std::cout<<"creating sockfd failed"<<std::endl;
        return -1;
    }

    inet_pton(AF_INET, "172.19.89.229", &ser_addr.sin_addr.s_addr);
    //ser_addr.sin_addr.s_addr = inet_addr("172.19.89.229");       //inet_addr不能处理IPv6
    ser_addr.sin_port = htons(PORT);
    ser_addr.sin_family = AF_INET;

    if((bind(listenfd,(struct sockaddr*)&ser_addr,sizeof(ser_addr)))<0)
    {
        std::cout<<"bind failed"<<std::endl;
        close(listenfd);
        return -1;
    }
    if((listen(listenfd,10))<0)
    {
        std::cout<<"listen failed"<<std::endl;
        return -1;
    }

    fd_set rset,allset;          //fd_set就是fd集合,需要监测的描述符置为1
    int maxfd,lafd;
    maxfd = listenfd;
    FD_ZERO(&allset);           //FD_ZERO：将指定集合里面所有的描述符全部置为0
    FD_SET(listenfd,&allset);        //FD_SET：用于在文件描述符集合中增加一个新的文件描述符
    
    while(1)
    {
        rset = allset;
        lafd = select(maxfd+1,&rset,NULL,NULL,NULL);
        if(lafd==0)
        {
            continue;
        }
        //检测是否有连接请求
        if(FD_ISSET(listenfd,&rset))                //FD_ISSET：用来检测指定的某个描述符是否有数据到来。
        {
            cli_len = sizeof(cli_addr);
            clifd = accept(listenfd,(struct sockaddr*)&cli_addr,&cli_len);
            printf("connect with %s by %d\n",inet_ntop(AF_INET,&cli_addr.sin_addr,str,sizeof(str)),ntohs(cli_addr.sin_port)); //inet_ntop()将IPv4或IPv6Internet网络地址转换为Internet标准格式的字符串。
            //添加fd                                                                                                //inet_pton()则相反,将标准文本表示形式的IPv4或IPv6Internet网络地址转换为数字二进制形式。
            FD_SET(clifd,&allset);
            if(clifd>maxfd)
            maxfd = clifd;
            if(lafd==1)
            continue;
        }
        //轮询检测有哪些fd传入数据
        int n = 0;
        for(int i = listenfd+1;i<maxfd+1;++i)
        {
            if(FD_ISSET(i,&rset))
            {
                n=recv(i,rec_buffer,MAXSIZE,0);
                if(n==0)
                {
                    close(i);
                    FD_CLR(i,&allset);
                }
                else if(n==-1)
                {

                }
                else{
                    std::cout<<"客户端传来信息，信息为："<<rec_buffer<<std::endl;
                    std::copy(rec_buffer,rec_buffer+strlen(rec_buffer)+1,send_buffer);
                    send(i,send_buffer,strlen(send_buffer),0);
                    std::cout<<"响应数据已发送\n"<<std::endl;
                }
            }
        }
    }
    close(listenfd);
    return 0;
}