 
 
#include <stdio.h>
#include <liburing.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080	// 监听端口
#define QUEUE_DEPTH 1024	// 请求队列深度
#define BUFFER_SIZE 1024	// 缓冲区大小

#define EVENT_ACCEPT 0
#define EVENT_READ 1
#define EVENT_WRITE 2

//保存连接信息
struct conn_info {
    int fd;
    int event;
};

// 初始化套接字
int init_socketfd(int port) {
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // 创建套接字
	
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));// 清空结构体
	addr.sin_family = AF_INET;// IPv4
	addr.sin_port = htons(port);// 端口号
	addr.sin_addr.s_addr = INADDR_ANY;// 任意地址

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind error\n");
        return -1;
    }

    if (listen(sockfd, 10) < 0) {
        perror("listen error\n");
    }

    return sockfd;
}

/*
以点餐为例：
io_uring_get_sqe(&ring);//相当于服务员取一个菜单
io_uring_prep_accept，io_uring_prep_recv，io_uring_prep_send相当于你在菜单上点菜
struct io_uring_cqe *cqe;	//相当于服务员把菜单给厨师
io_uring_wait_cqe(&ring, &cqe);//相当于服务员等待厨师做菜
*/


//设置accept请求
int set_accept(struct io_uring *ring,int sockfd,struct sockaddr *addr,socklen_t *addrlen,int flag){
	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);// 获取一个请求
	struct conn_info accept_info = {
		.fd = sockfd,
		.event = EVENT_ACCEPT,
	};
	io_uring_prep_accept(sqe, sockfd, (struct sockaddr *)addr, addrlen, flag);// 接受连接
	memcpy(&sqe->user_data, &accept_info, sizeof(accept_info));// 设置用户数据

	return 0;
}

//设置读请求
int set_read(struct io_uring *ring,int sockfd,void *buf,size_t len,int flags){
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);// 获取一个请求
    struct conn_info read_info = {
        .fd = sockfd,
        .event = EVENT_READ,
    };
    io_uring_prep_recv(sqe, sockfd, buf, len, flags);// 读数据
    memcpy(&sqe->user_data, &read_info, sizeof(read_info));// 设置用户数据

    return 0;
}

//设置写请求
int set_write(struct io_uring *ring,int sockfd,void *buf,size_t len,int flags){
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);// 获取一个请求
    struct conn_info write_info = {
        .fd = sockfd,
        .event = EVENT_WRITE,
    };
    io_uring_prep_send(sqe, sockfd, buf, len, flags);// 写数据
    memcpy(&sqe->user_data, &write_info, sizeof(write_info));// 设置用户数据

    return 0;
}

int main() {
	int sockfd = init_socketfd(PORT);// 初始化套接字

	// io_uring初始化
	struct io_uring ring;// io_uring
	struct io_uring_params params;// 参数
	memset(&params, 0, sizeof(params));//初始化参数
	io_uring_queue_init_params(QUEUE_DEPTH, &ring, &params);// 初始化io_uring

	// 接受连接
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	set_accept(&ring,sockfd,(struct sockaddr *)&client_addr,&client_addr_len,0);// 设置accept

	char buffer[BUFFER_SIZE] = {0};// 缓冲区
	while (1) {
	    io_uring_submit(&ring);// 提交请求

		//等待至少一个任务完成
	    struct io_uring_cqe *cqe;// 请求完成
	    io_uring_wait_cqe(&ring, &cqe);// 等待请求完成

		//等待所有任务完成
		struct io_uring_cqe *cqes[128];
		int nready = io_uring_peek_batch_cqe(&ring, cqes, 128);// 批量获取请求完成

		for (int i = 0; i < nready; i++) {
		    struct io_uring_cqe *entries = cqes[i];// 获取请求完成
			struct conn_info result;// 连接信息
			memcpy(&result, &entries->user_data, sizeof(result));// 获取用户数据

			if (result.event == EVENT_ACCEPT) {
			    set_accept(&ring,sockfd,(struct sockaddr *)&client_addr,&client_addr_len,0);// 设置accept
				int connfd = entries->res;
				set_read(&ring,connfd,buffer,BUFFER_SIZE,0);//读取
			}else if (result.event == EVENT_READ) {
			    int ret = entries->res;
			    if (ret == 0) {
			        close(result.fd);// 关闭连接
			    }else if (ret > 0) {
			        set_write(&ring,result.fd,buffer,ret,0);// 写入
			    }
			}else if (result.event == EVENT_WRITE) {
			    int ret = entries->res;
			    if (ret > 0) {
			        set_read(&ring,result.fd,buffer,BUFFER_SIZE,0);// 读取
			    }
			}
		}
		//清空请求完成
		io_uring_cq_advance(&ring, nready);
	}

    return 0;
}