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
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

// 自定义 proto 文件的命名空间
using first::Login::LoginService;
using first::Login::IMRegistRequest;
using first::Login::IMRegistResponse;
using first::Login::IMLoginRequest;
using first::Login::IMLoginResponse;

class ImLoginClient{
public:
    ImLoginClient(std::shared_ptr<Channel> channel)
    :stub_(LoginService::NewStub(channel)) 
    {}

    void Regist(const std::string &username, const std::string &password) {
        IMRegistRequest request;
        request.set_username(username);
        request.set_password(password);
        
        IMRegistResponse response;
        ClientContext context;
        std::cout <<  "-> Regist req" << std::endl;
        // 调用 rpc 接口
        Status status = stub_->Regist(&context, request, &response);
        if(status.ok()) {
            std::cout  << ", user_id:" << response.msg() << std::endl;
        } 
        else {
            std::cout  << "Regist failed: " << response.msg()<< std::endl;
        }
    }

     void Login(const std::string &username, const std::string &password) {
        IMLoginRequest request;
        request.set_username(username);
        request.set_password(password);
        
        IMLoginResponse response;
        ClientContext context;
        std::cout <<  "-> Login req" << std::endl;
        // 调用 rpc 接口
        Status status = stub_->Login(&context, request, &response);
        if(status.ok()) {
            std::cout << " login ok" << std::endl;
        } 
        else {
            std::cout  << "Login failed: " << response.msg()<< std::endl;
        }
    }

private:
    std::unique_ptr<LoginService::Stub> stub_;   // 存根，客户端代理
};

int main()  {
    // 服务器的地址
    std::string server_addr = "localhost:50051";
    
    // 创建请求通道 
    ImLoginClient im_login_client(
        grpc::CreateChannel(server_addr, grpc::InsecureChannelCredentials())
    );

    // 测试
    std::string username = "Jim Hacker";
    std::string password = "123456";
    im_login_client.Regist(username, password);
    im_login_client.Login(username, password);

    return 0;
}
