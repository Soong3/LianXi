#include <iostream>
#include <string>

// grpc 头文件
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

// 自定义 proto 文件生成的.h
#include "first.Login.pb.h"
#include "first.Login.grpc.pb.h"

// 1、命名空间
// grcp 命名空间
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

// 自定义 proto 文件的命名空间
using first::Login::LoginService;
using first::Login::IMRegistRequest;
using first::Login::IMRegistResponse;
using first::Login::IMLoginRequest;
using first::Login::IMLoginResponse;

class IMLoginServiceImpl: public LoginService::Service{
    //注册
     virtual ::grpc::Status Regist(::grpc::ServerContext* context, const ::first::Login::IMRegistRequest* request, ::first::Login::IMRegistResponse* response) override{
        std::cout << "Regist user_name:" << request->username() <<std::endl;
        response->set_code(5);
        response->set_msg("注册成功");
        return Status::OK;
    }
    

    //登录
    virtual ::grpc::Status Login(::grpc::ServerContext* context, const ::first::Login::IMLoginRequest* request, ::first::Login::IMLoginResponse* response) override{
        std::cout << "Login user_name:" << request->username() <<std::endl;
        response->set_code(5);
        response->set_msg("登录成功");
        return Status::OK;
    }
    
};

void RunService(){
    std::string server_addr("0.0.0.0:50051");

    // 创建一个服务类
    IMLoginServiceImpl service;

    // 创建工厂类
    ServerBuilder builder;

    // 监听端口地址
    builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());
    // 心跳探活
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, 5000);
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 10000);
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
    // 多线程：动态调整 epoll 线程数量
    builder.SetSyncServerOption(ServerBuilder::MIN_POLLERS, 4);
    builder.SetSyncServerOption(ServerBuilder::MAX_POLLERS, 8);
    // 注册服务
    builder.RegisterService(&service);
   
    // 创建并启动 rpc 服务器
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_addr << std::endl;
    
    // 进入服务事件循环
    server->Wait();
}

int main(int argc,const char** argv){
    RunService();
    return 0;
}