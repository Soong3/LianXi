#include <stdio.h>
#include <rte_eal.h>        // DPDK环境抽象层头文件
#include <rte_ethdev.h>     // DPDK以太网设备操作头文件
#include <arpa/inet.h>      // 网络地址转换相关函数
 
int global_portid = 0;       // 全局变量，指定使用的网卡端口号（默认0号端口）
 
#define NUM_MBUFS  4096      // 内存池中mbuf（内存缓冲区）的数量
#define BURST_SIZE 128       // 单次收发数据包的最大突发数量
 
#define ENABLE_SEND 1
 
#if ENABLE_SEND
 
uint8_t global_smac[RTE_ETHER_ADDR_LEN];
uint8_t global_dmac[RTE_ETHER_ADDR_LEN];
 
uint32_t global_sip;
uint32_t global_dip;
 
uint16_t global_sport;
uint16_t global_dport;
 
#endif
// 默认网卡配置结构体
static const struct rte_eth_conf port_conf_default = {
    .rxmode = { 
        .max_rx_pkt_len = RTE_ETHER_MAX_LEN  // 设置最大接收包长为标准以太网帧长度（1518字节）
    }
};
 
/* 初始化指定网卡端口 */
static int ustack_init_port(struct rte_mempool *mbuf_pool) {
    // 获取系统可用网卡数量
    uint16_t nb_sys_ports = rte_eth_dev_count_avail();
    if (nb_sys_ports == 0) {
        rte_exit(EXIT_FAILURE, "No Supported eth found\n");
    }
    struct rte_eth_dev_info dev_info;
	rte_eth_dev_info_get(global_portid, &dev_info);
    // 配置网卡参数（1个RX队列，0个TX队列）
    const int num_rx_queues = 1;
#if ENABLE_SEND
    const int num_tx_queues = 1;
#else
    const int num_tx_queues = 0;
#endif
    if (rte_eth_dev_configure(global_portid, num_rx_queues, 
                            num_tx_queues, &port_conf_default) < 0) {
        rte_exit(EXIT_FAILURE, "Port configuration failed\n");
    }
    
	
    // 设置接收队列（队列号0，队列长度128，绑定到当前socket的内存池）
    if (rte_eth_rx_queue_setup(global_portid, 0, 128,rte_eth_dev_socket_id(global_portid),NULL, mbuf_pool) < 0) {
        rte_exit(EXIT_FAILURE, "Could not setup RX queue\n");
    }
#if ENABLE_SEND
    struct rte_eth_txconf txq_conf = dev_info.default_txconf;
	txq_conf.offloads = port_conf_default.rxmode.offloads;//
	if(rte_eth_tx_queue_setup(global_portid, 0, 512, rte_eth_dev_socket_id(global_portid), &txq_conf) < 0){
		rte_exit(EXIT_FAILURE, "Could not setup TX queue\n");
	};
#endif
    // 启动网卡设备
    if (rte_eth_dev_start(global_portid) < 0) {
        rte_exit(EXIT_FAILURE, "Could not start port\n");
    }
 
    return 0;
}
 
//打包函数
static int ustack_encode_udp_pkt(uint8_t *msg, uint8_t *data, uint16_t total_len){
    //1 ether header 以太网头
    struct rte_ether_hdr *eth = (struct rte_ether_hdr *)msg;//强行转换，把内存的头部强行转换成以太网头
    //目的地址，源地址，协议类型
    rte_memcpy(eth->d_addr.addr_bytes, global_dmac, RTE_ETHER_ADDR_LEN);
    rte_memcpy(eth->s_addr.addr_bytes, global_smac, RTE_ETHER_ADDR_LEN);
    eth->ether_type = htons(RTE_ETHER_TYPE_IPV4);
    //IP头
    struct rte_ipv4_hdr *ip = (struct rte_ipv4_hdr *)(eth + 1);//msg + sizeof(struct rte_ether_hdr);,以太网头后面接着IP头
    ip->version_ihl = 0x45;//4位版本号
    ip->type_of_service = 0;
    ip->total_length = htons(total_len - sizeof(struct rte_ether_hdr));// 总长度
    ip->packet_id = 0;//16位标识符
    ip->fragment_offset = 0;
    ip->time_to_live = 64 ;//数据包每经过一个网关减一，用来描述一个包的生命时长的。
    ip->next_proto_id = IPPROTO_UDP;
    ip->src_addr = global_sip;
    ip->dst_addr = global_dip;
    //IP头校验,先清零
    ip->hdr_checksum = 0;
    ip->hdr_checksum = rte_ipv4_cksum(ip);
     
   //UDP头
    struct rte_udp_hdr *udp = (struct rte_udp_hdr *)(ip + 1);
    udp->src_port = global_sport;
    udp->dst_port = global_dport;
    uint16_t udplen = total_len - sizeof(struct rte_ether_hdr) - sizeof(struct rte_ipv4_hdr);
    udp->dgram_len = htons(udplen);
 
    rte_memcpy((uint8_t*)(udp + 1), data, udplen);//数据放在后面
    udp->dgram_cksum = 0; 
    udp->dgram_cksum = rte_ipv4_udptcp_cksum(ip, udp);
    
    return 0;
}
 
 
 
int main(int argc, char *argv[]) {
    if (rte_is_zero_ether_addr(&global_dmac)) {
    printf("错误：未设置目标MAC地址\n");
}

    // 初始化DPDK环境抽象层（EAL）
    if (rte_eal_init(argc, argv) < 0) {
        rte_exit(EXIT_FAILURE, "Error with EAL init\n");
    }
 
    // 创建mbuf内存池（名称，元素数量，缓存大小，私有数据大小，每个缓冲区大小，socket ID）
    struct rte_mempool *mbuf_pool = rte_pktmbuf_pool_create(
        "mbuf pool", NUM_MBUFS, 
        0, 0, RTE_MBUF_DEFAULT_BUF_SIZE, 
        rte_socket_id());
    if (mbuf_pool == NULL) {
        rte_exit(EXIT_FAILURE, "Could not create mbuf pool\n");
    }
 
    // 初始化网卡端口
    ustack_init_port(mbuf_pool);
 
    // 主数据包处理循环
    while (1) {
        struct rte_mbuf *mbufs[BURST_SIZE] = {0};  // 接收缓冲区数组
 
        // 从指定端口和队列批量接收数据包（返回实际接收数量）
        uint16_t num_recvd = rte_eth_rx_burst(
            global_portid, 0, mbufs, BURST_SIZE);
        if (num_recvd > BURST_SIZE) {  // DPDK保证不会超过BURST_SIZE，此处为冗余检查
            rte_exit(EXIT_FAILURE, "Error receiving from eth\n");
        }
 
        // 处理每个接收到的数据包
        for (int i = 0; i < num_recvd; i++) {
            // 获取以太网头部（rte_pktmbuf_mtod将mbuf指针转换为数据指针）
            struct rte_ether_hdr *ethhdr = rte_pktmbuf_mtod(
                mbufs[i], struct rte_ether_hdr *);
            
            // 只处理IPv4数据包（注意转换字节序）
            if (ethhdr->ether_type != rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4)) {
                continue;  // 跳过非IPv4包
            }
 
            // 获取IP头部（以太网头之后的位置）
            struct rte_ipv4_hdr *iphdr = rte_pktmbuf_mtod_offset(
                mbufs[i], struct rte_ipv4_hdr *, 
                sizeof(struct rte_ether_hdr));
            
            // 处理UDP协议数据包
            if (iphdr->next_proto_id == IPPROTO_UDP) {
                // 获取UDP头部（IP头之后的位置）
                struct rte_udp_hdr *udphdr = (struct rte_udp_hdr *)(iphdr + 1);
#if ENABLE_SEND
 
                rte_memcpy(global_smac, ethhdr->d_addr.addr_bytes, RTE_ETHER_ADDR_LEN);
                rte_memcpy(global_dmac, ethhdr->s_addr.addr_bytes, RTE_ETHER_ADDR_LEN);
 
                rte_memcpy(&global_sip, &iphdr->dst_addr, sizeof(uint32_t));
                rte_memcpy(&global_dip, &iphdr->src_addr, sizeof(uint32_t));
 
                rte_memcpy(&global_sport, &udphdr->dst_port, sizeof(uint32_t));
                rte_memcpy(&global_dport, &udphdr->src_port, sizeof(uint32_t));
                
                struct in_addr addr;
                addr.s_addr = iphdr->src_addr;
                printf("sip %s:%d -->", inet_ntoa(addr),ntohs(udphdr->src_port));
                
                addr.s_addr = iphdr->dst_addr;
                printf("dip %s:%d -->", inet_ntoa(addr),ntohs(udphdr->dst_port));
 
			    struct rte_mbuf *mbuf = rte_pktmbuf_alloc(mbuf_pool);
                if(!mbuf){
                    rte_exit(EXIT_FAILURE, "rte_pktmbuf_alloc\n");
                }
                uint16_t length = ntohs(udphdr->dgram_len);
                uint16_t total_len = length + sizeof(struct rte_ipv4_hdr) + sizeof(struct rte_ether_hdr);
 
                mbuf->pkt_len = total_len;
                mbuf->data_len = total_len;
 
                uint8_t *msg  = rte_pktmbuf_mtod(mbuf, uint8_t *);
                ustack_encode_udp_pkt(msg, (uint8_t *)(udphdr + 1), total_len);
                
                rte_eth_tx_burst(global_portid, 0, &mbuf, 1);
#endif
                // 打印UDP载荷（UDP头之后的部分）
                printf("UDP Payload: %s\n", (char *)(udphdr + 1));
            }
 
            rte_pktmbuf_free(mbufs[i]);  // 释放mbuf（实际代码可能需要延迟释放）
        }
    }
 
    // 注意：DPDK应用通常不会执行到这里，需要手动终止
    printf("hello dpdk\n");
    return 0;
}
