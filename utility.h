#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED
#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;
 
// clients_list save all the clients's socket
list<int> clients_list;
 
/**********************   macro defintion **************************/
// server ip
// 服务器ip地址，为测试使用本地机地址，可以更改为其他服务端地址
#define SERVER_IP "127.0.0.1"

// server port 服务器端口号
#define SERVER_PORT 8888
 
//epoll size int epoll_create(int size) 的sizep,为epoll支持的最大句柄数
#define EPOLL_SIZE 5000
 
//message buffer size 缓冲区大小65535
#define BUF_SIZE 0xFFFF
 
#define SERVER_WELCOME "Welcome you join  to the chat room! Your chat ID is: Client #%d"
 
#define SERVER_MESSAGE "ClientID %d say >> %s"
 
// exit
#define EXIT "EXIT"
 
#define CAUTION "There is only one int the char room!"
 
/**********************   some function **************************/
/**
  * @param sockfd: socket descriptor
  * @return 0
**/
//将文件描述符设置为非阻塞方式
int setnonblocking(int sockfd)
{
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)| O_NONBLOCK);
    return 0;
}
 
/**
  * @param epollfd: epoll handle
  * @param fd: socket descriptor
  * @param enable_et : enable_et = true, epoll use ET; otherwise LT
**/
//将文件描述符fd添加到epollfd标示的内核事件表中，并注册EPOLLIN（数据可读事件）和EPOOLET事件（ET工作方式）
//最后将文件描述符设置非阻塞方式
void addfd( int epollfd, int fd, bool enable_et )
{
    struct epoll_event ev;  //epoll_ctl参数event是一个结构体

    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if( enable_et )
        ev.events = EPOLLIN | EPOLLET; 
//EPOLL_CTL_ADD 注册新的fd到epfd)
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
//设置非阻塞方式
    setnonblocking(fd);
    printf("fd added to epoll!\n\n");
}
 
/**
  * @param clientfd: socket descriptor
  * @return : len
**/

//服务端发送广播信息，使所有用户都能收到消息
int sendBroadcastmessage(int clientfd)
{
    // buf[BUF_SIZE] receive new chat message
    // message[BUF_SIZE] save format message
    char buf[BUF_SIZE], message[BUF_SIZE];
    //置字节字符串s的前n个字节为零且包括‘\0’
    bzero(buf, BUF_SIZE); 
    bzero(message, BUF_SIZE);
 
    // receive message
    printf("read from client(clientID = %d)\n", clientfd);
    int len = recv(clientfd, buf, BUF_SIZE, 0);
 
    if(len == 0)  // len = 0 means the client closed connection
    {
        close(clientfd);
        clients_list.remove(clientfd); //server remove the client
        printf("ClientID = %d closed.\n now there are %d client in the char room\n", clientfd, (int)clients_list.size());
 
    }
    else  //broadcast message 
    {
        if(clients_list.size() == 1) { // this means There is only one int the char room
            send(clientfd, CAUTION, strlen(CAUTION), 0);
            return len;
        }
        // format message to broadcast
        sprintf(message, SERVER_MESSAGE, clientfd, buf);
 
        list<int>::iterator it;
        for(it = clients_list.begin(); it != clients_list.end(); ++it) {
           if(*it != clientfd){
                if( send(*it, message, BUF_SIZE, 0) < 0 ) { perror("error"); exit(-1);}
           }
        }
    }
    return len;
}
#endif // UTILITY_H_INCLUDED
