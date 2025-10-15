
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <poll.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/time.h>
#include "reactor.h"



static msg_handler kvs_handler; //消息处理函数数组

int kvs_request(struct coon *c){//请求处理函数
    c->wlength = kvs_handler(c->rbuffer,c->rlength,c->wbuffer);
    //printf("kvs_request:%d\n",c->wlength);
    return c->wlength > 0 ? 0 : -1;  // 必须返回0/-1，不能返回长度
}

int kvs_response(struct coon *c){
    
}

#define MAX_PORT 20    //最大端口数
#define MAX_CONNECT 1024  //最大连接数
#define TIME_SUB_MS(tv1, tv2)  ((tv1.tv_sec - tv2.tv_sec) * 1000 + (tv1.tv_usec - tv2.tv_usec) / 1000)  //计算时间差，单位为毫秒
struct timeval begin;//开始时间
int epfd = 0; //epoll文件描述符

int accept_cb(int fd);  //接受连接的回调函数
int recv_cb(int fd);   //读取数据的回调函数
int send_cb(int fd);  //写入数据的回调函数


struct coon coon_list[MAX_CONNECT] = {0};  //定义一个连接列表

//初始化epoll
int set_event(int fd,int event,int flag)  //设置事件
{
    if(flag){
        struct epoll_event ev;
        ev.events = event;
        ev.data.fd = fd;
        epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);//添加事件
    }else{
        struct epoll_event ev;
        ev.events = event;
        ev.data.fd = fd;
        epoll_ctl(epfd,EPOLL_CTL_MOD,fd,&ev);//删除事件
    }
}

//注册事件
int event_register(int fd,int event)  
{
    if(fd < 0) return -1;

    coon_list[fd].fd = fd;
    coon_list[fd].r_action.recv_callback = recv_cb;
    coon_list[fd].send_callback = send_cb;
    //coon_list[fd].r_action.accept_callback = accept_cb;

    memset(coon_list[fd].rbuffer,0,BUF_SIZE);
    coon_list[fd].rlength = 0;

    memset(coon_list[fd].wbuffer,0,BUF_SIZE);
    coon_list[fd].wlength = 0;

    set_event(fd,event,1);//设置事件
}

int accept_cb(int fd){
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(fd,(struct sockaddr*)&client_addr,&client_len);
    if(client_fd < 0){
        perror("accept");
        return -1;
    }
    event_register(client_fd,EPOLLIN);//注册事件

    if ((client_fd % 1000) == 0) {	
		
		struct timeval current;		
		gettimeofday(&current, NULL);	
		
		int time_used = TIME_SUB_MS(current, begin);		
		memcpy(&begin, &current, sizeof(struct timeval));	
		
		printf("accept finshed: %d, time_used: %d\n", client_fd, time_used);	
 
	}

    return 0;
}

int recv_cb(int fd){
    memset(coon_list[fd].rbuffer,0,BUF_SIZE);
    int n = recv(fd,coon_list[fd].rbuffer,BUF_SIZE,0);
    if(n == 0){
        printf("client close\n");
        close(fd);
        return 0;
    }else if(n < 0){    //出错
        perror("recv error");
        close(fd);
        return -1;      
    }

    //printf("recv data: %s\n",coon_list[fd].rbuffer);

    coon_list[fd].rlength = n;

#if 0
    coon_list[fd].wlength = coon_list[fd].rlength;
    memcpy(coon_list[fd].wbuffer,coon_list[fd].rbuffer,coon_list[fd].wlength);
#else
    kvs_request(&coon_list[fd]);//触发业务逻辑
#endif
    set_event(fd,EPOLLOUT,0);//注册写事件
    return n;
}

int send_cb(int fd){
    kvs_response(&coon_list[fd]);
    int n = 0;
    if(coon_list[fd].wlength != 0){
        int n = send(fd,coon_list[fd].wbuffer,coon_list[fd].wlength,0);
    }

    if(n < 0){
        perror("send error");
        close(fd);
        return -1;
    }

    //printf("send data: %s\n",coon_list[fd].wbuffer);

    set_event(fd,EPOLLIN,0);//注册读事件
    return n;
}

int init_server(unsigned short port){
    
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0){
        perror("socket error\n");
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr)) == -1){
        perror("bind error\n");
    }

    listen(sockfd,10);

    return sockfd;
}

int reactor_start(unsigned short port,msg_handler handler){
    kvs_handler = handler;//注册回调函数
    //unsigned short port = 8080;
    epfd = epoll_create(1);
    for(int i = 0;i < MAX_PORT;i++){
        int sockfd = init_server(port + i);
        coon_list[sockfd].fd = sockfd;
        coon_list[sockfd].r_action.accept_callback = accept_cb;//注册回调函数
        set_event(sockfd,EPOLLIN,1);
    }

    gettimeofday(&begin,NULL);

    while(1){
        struct epoll_event events[MAX_PORT] = {0};
        int n = epoll_wait(epfd,events,MAX_PORT,1000);
        for(int i = 0;i < n;i++){
            int fd = events[i].data.fd;
            if(events[i].events & EPOLLIN){
                coon_list[fd].r_action.recv_callback(fd);
            }
            if(events[i].events & EPOLLOUT){
                coon_list[fd].send_callback(fd);
            }
        }
    }
    return 0;
}