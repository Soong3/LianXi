#ifndef __REACTOR_H__
#define __REACTOR_H__

#define BUF_SIZE 1024   //缓冲区大小
typedef int (*RCALLBACK)(int fd);//回调函数类型
typedef int (*msg_handler)(char *msg,int len,char *response); //消息处理函数类型
//定义一个结构体，用于存储连接信息
struct coon 
{
    int fd;
    char rbuffer[BUF_SIZE];
    int rlength;

    char wbuffer[BUF_SIZE];
    int wlength;

    RCALLBACK send_callback;

    union 
    {
        RCALLBACK accept_callback;
        RCALLBACK recv_callback;
    }r_action;
    
};

int reactor_start(unsigned short port,msg_handler handler);

int kvs_response(struct coon *c);
int kvs_request(struct coon *c);

#endif