//学习资料
//Nginx：配置
https://blog.csdn.net/you_fathe/article/details/127995926?spm=1001.2014.3001.5506

https://blog.csdn.net/2301_76446998/article/details/142689360?spm=1001.2014.3001.5506

//过滤模块
https://blog.csdn.net/you_fathe/article/details/127994783?spm=1001.2014.3001.5506

//handle模块
https://blog.csdn.net/you_fathe/article/details/127994783?spm=1001.2014.3001.5506




//nginx要学习内容
1.conf解析流程
自上而下解析，先文件加载（nginx.conf），然后解析http，再解析server，最后解析location

2.nginx里面基础组件我们可以自己用（ngx_str_t,ngx_list_t等等），也就是线程池，链表什么的

3.nginx惊群现象
一个进程在等待一个事件，如果多个进程同时监听这个事件，当事件发生时，所有进程都会被唤醒，但是只有一个进程能够处理这个事件，其他进程处理完后又会进入等待状态，这就是惊群现象。

4.过滤器模块实现

5.handle模块实现

6.http状态机
将http协议解析成一个个状态，然后根据状态进行调用处理函数


