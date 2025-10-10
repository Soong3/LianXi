#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <sys/epoll.h>    // epoll的核心头文件
void *thread_func(void *arg) {
    int socktfd = *(int *)arg;//获取套接字描述符
    char buffer[1024];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int n = read(socktfd, buffer, sizeof(buffer));
        if (n == 0) {
            printf("client closed\n");
            break;
        }
        printf("recv: %s\n", buffer);

        send(socktfd, buffer, n, 0);
        printf("send: %s\n", buffer);
    }
}
int main() {
    int socktfd = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;//IPv4
    server_addr.sin_port = htons(8080);//port
    server_addr.sin_addr.s_addr = INADDR_ANY;//IP address

    if(bind(socktfd,(struct sockaddr*)&server_addr,sizeof(server_addr)) == -1){
        printf("bind error\n");
    }

    listen(socktfd,5);

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
#if 0
    while(1){
        int client_socket = accept(socktfd,(struct sockaddr*)&client_addr,&client_addr_len);
        if(client_socket == -1){
            printf("accept error\n");
        }

        char buffer[1024] = {0};
        recv(client_socket,buffer,1024,0);
        printf("Message from client: %s\n",buffer);

        char buffer1[1024] = {0};
        scanf("%s",buffer1);
        send(client_socket,buffer1,strlen(buffer1),0);
        close(client_socket);
    }
#if 0
    while(1){
        int client_socket = accept(socktfd,(struct sockaddr*)&client_addr,&client_addr_len);
        if(client_socket == -1){
            printf("accept error\n");
        }

        pthread_t tid;
        pthread_create(&tid,NULL,thread_func,&client_socket);
    }
#endif

#if 0
    fd_set rfds;//定义文件描述符集合
    FD_ZERO(&rfds);//清空集合
    FD_SET(socktfd,&rfds);//将socktfd加入集合
    int maxfd = socktfd;//记录最大的文件描述符
    while(1){
        select(maxfd+1,&rfds,NULL,NULL,NULL);//阻塞等待
        if(FD_ISSET(socktfd,&rfds)){//判断socktfd是否在集合中
            int client_socket = accept(socktfd,(struct sockaddr*)&client_addr,&client_addr_len);
            if(client_socket == -1){
                printf("accept error\n");
            }
            FD_SET(client_socket,&rfds);//将client_socket加入集合
            if(client_socket > maxfd){
                maxfd = client_socket;//更新最大文件描述符
            }
        }
        //rev
        int i = 0;
        for ( i = socktfd + 1; i <= maxfd; i++)
        {
            if(FD_ISSET(i,&rfds)){
                char buf[1024] = {0};
                recv(i,buf,sizeof(buf),0);  
                printf("recv:%s\n",buf);

                int count = send(i, buf, sizeof(buf), 0);
			    printf("SEND: %d\n",count);
            }
        }
    }
#endif
#else
    int epfd = epoll_create(1);//创建epoll实例
    struct epoll_event ev;//事件
    ev.events = EPOLLIN;//读事件
    ev.data.fd = socktfd;//文件描述符

    epoll_ctl(epfd,EPOLL_CTL_ADD,socktfd,&ev);//将socktfd加入epoll实例

    while(1){
        struct epoll_event events[1024] = {0};//epoll_wait返回的数组
        int count = epoll_wait(epfd,events,1024,-1);//等待事件发生
        for (int i = 0; i < count; i++)
        {
            int connfd = events[i].data.fd;
            if(connfd == socktfd){
                int client_socket = accept(socktfd,(struct sockaddr*)&client_addr,&client_addr_len);
                if(client_socket < 0){
                    printf("accept error\n");
                }
                ev.events = EPOLLIN;//读事件
                ev.data.fd = client_socket;//文件描述符
                epoll_ctl(epfd,EPOLL_CTL_ADD,client_socket,&ev);//将client_socket加入epoll实例
            }
            else if(events[i].events & EPOLLIN){
                char buf[1024] = {0};
                int count = recv(connfd, buf, sizeof(buf), 0);
                printf("RECV: %s\n",buf);
                if(count == 0){
                    close(connfd);
                }
                else if(count < 0){
                    close(connfd);
                }
            }
        }
    }
#endif
    printf("exit\n");
    return 0;
}

