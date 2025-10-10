#include <stdio.h>
#include <rte_eal.h>        // DPDK环境抽象层头文件
#include <rte_ethdev.h>     // DPDK以太网设备操作头文件
#include <arpa/inet.h>      // 网络地址转换相关函数

int global_portid = 0; // 全局端口ID
#define NUM_MBUFS 4096  // 环形缓冲区中数据包的最大数量
#define BURST_SIZE 128  // 每次从网卡接收或发送数据包的最大数量
// 默认网卡配置结构体
static const struct rte_eth_conf port_conf_default = {
    .rxmode = { 
        .max_rx_pkt_len = RTE_ETHER_MAX_LEN  // 设置最大接收包长为标准以太网帧长度（1518字节）
    }
};

static int ustack_init_port(struct rte_mempool *mbuf_pool){
    //获取系统可用网卡数量
    int nb_ports = rte_eth_dev_count_avail();
    if(nb_ports == 0)
    {
        rte_exit(EXIT_FAILURE, "No Ethernet ports found\n");
    }

    //配置网卡参数
    const int nb_rx_queues = 1;//接收队列数量
    const int nb_tx_queues = 0;//发送队列数量
    if(rte_eth_dev_configure(global_portid, nb_rx_queues, nb_tx_queues, &port_conf_default) < 0){
        rte_exit(EXIT_FAILURE, "Cannot configure device\n");
    }

    //为网卡分配接收队列
    if(rte_eth_rx_queue_setup(global_portid, 0, 128, rte_eth_dev_socket_id(global_portid), NULL, mbuf_pool) < 0){//0为队列ID，128为队列大小
        rte_exit(EXIT_FAILURE, "rte_eth_rx_queue_setup:err\n");
    }

    //启动网卡
    if(rte_eth_dev_start(global_portid) < 0){
        rte_exit(EXIT_FAILURE, "rte_eth_dev_start:err\n");
    }

    return 0;
}
int main(int argc, char *argv[])
{
    // 初始化DPDK环境
    if(rte_eal_init(argc, argv) < 0)
    {
        rte_exit(EXIT_FAILURE, "Cannot init EAL\n");
    }

    //创建内存池
    struct rte_mempool *mbuf_pool = rte_pktmbuf_pool_create("mbuf pool",NUM_MBUFS,0,0,RTE_MBUF_DEFAULT_BUF_SIZE,rte_socket_id());
    if(mbuf_pool == NULL){
        rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");
    }

    //初始化网卡端口
    ustack_init_port(mbuf_pool);

    //数据包处理
    while(1)
    {
        //接收数据包
        struct rte_mbuf *bufs[BURST_SIZE] = {0};
        uint16_t num_recv =  rte_eth_rx_burst(global_portid,0,bufs,BURST_SIZE);
        if(num_recv > BURST_SIZE){
            rte_exit(EXIT_FAILURE, "rte_eth_rx_burst:err\n");
        }

        //处理数据包
        for(uint16_t i = 0; i < num_recv; i++){
            //获取头部
            struct rte_ether_hdr *eth_hdr = rte_pktmbuf_mtod(bufs[i], struct rte_ether_hdr *);

            //判断协议类型,只处理IPV4
            if(eth_hdr->ether_type != rte_be_to_cpu_16(RTE_ETHER_TYPE_IPV4)){
                continue; 
            }

            //获取IPV4头部
            struct rte_ipv4_hdr *ipv4_hdr = rte_pktmbuf_mtod_offset(bufs[i], struct rte_ipv4_hdr *, sizeof(struct rte_ether_hdr));

            //处理UDP数据包
            if(ipv4_hdr->next_proto_id == IPPROTO_UDP){
                //获取UDP头部后面内容
                struct rte_udp_hdr *udp_hdr = (struct rte_udp_hdr*)(ipv4_hdr + 1);

                //打印内容
                printf("UPD DATA:%s\n",(char *)(udp_hdr + 1));
            }

            //释放数据包
            rte_pktmbuf_free(bufs[i]);
        }
    }
    return 0;
}