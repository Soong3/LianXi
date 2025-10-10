#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
 
#define BUFFER_SIZE 1024
 
#define CONNECT_SIZE 1024
 
typedef int (*CALLBACK)(int fd);
 
typedef struct conn {
    int fd;
    
    char rbuffer[BUFFER_SIZE];
    int rlen;
    
    char wbuffer[BUFFER_SIZE];
    int wlen;
    
    CALLBACK send_callback;
    
    union {
        CALLBACK recv_callback;
        CALLBACK accept_callback;
    } r_actor;
} conn;
 
int epfd = 0;
static conn conn_list[CONNECT_SIZE] = {0};
 
void set_event(int fd, int event) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = event;
    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
}
 
int send_cb(int fd) {
    conn* c = &conn_list[fd];
 
    int len = send(c->fd, c->wbuffer, c->wlen, 0);
    set_event(c->fd, EPOLLIN | EPOLLET);
    return len;
}
 
int recv_cb(int fd) {
    conn* c = &conn_list[fd];
    memset(c->rbuffer, 0, BUFFER_SIZE);
    c->rlen = recv(c->fd, c->rbuffer, BUFFER_SIZE, 0);
    if(c->rlen <= 0) {
        close(c->fd);
        epoll_ctl(epfd, EPOLL_CTL_DEL, c->fd, NULL);
        return -1;
    }
 
    memcpy(c->wbuffer, c->rbuffer, c->rlen);
    c->wlen = c->rlen;
    set_event(c->fd, EPOLLOUT | EPOLLET);
}
 
int accept_cb(int fd) {
    conn* c = &conn_list[fd];
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    int connfd = accept(c->fd, (struct sockaddr*)&client_addr, &len);
    if(connfd < 0) {
        printf("accept failed\n");
        return -1;
    }
 
    conn_list[connfd].fd = connfd;
    conn_list[connfd].r_actor.recv_callback = recv_cb;
    conn_list[connfd].send_callback = send_cb;
    
    struct epoll_event ev;
    ev.data.fd = connfd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
}
 
int main() {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = INADDR_ANY;
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
 
    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr_in)) < 0) {
        printf("bind failed\n");
        close(listenfd);
        return -1;
    }
 
    listen(listenfd, 10);
    printf("listen listenfd: %d\n", listenfd);
 
    conn_list[listenfd].fd = listenfd;
    conn_list[listenfd].r_actor.recv_callback = accept_cb;
 
    epfd = epoll_create(1);
    struct epoll_event ev;
    ev.data.fd = listenfd;
    ev.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
 
    while(1) {
        struct epoll_event events[1024];
        int nready = epoll_wait(epfd, events, 1024, -1);
        for(int i = 0; i < nready; ++i) {
            if(events[i].events & EPOLLIN) {
                conn_list[events[i].data.fd].r_actor.recv_callback(events[i].data.fd);
            }
 
            if(events[i].events & EPOLLOUT) {
                conn_list[events[i].data.fd].send_callback(events[i].data.fd);
            }
        }
    }
}