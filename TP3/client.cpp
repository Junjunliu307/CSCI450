#include <iostream>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 35768 // 主服务器的 TCP 端口

// 加密函数
std::string encrypt(const std::string& input) {
    std::string encrypted;
    for (char c : input) {
        if (std::isalpha(c)) {
            if (std::islower(c)) {
                encrypted += (c - 'a' + 3) % 26 + 'a';
            } else {
                encrypted += (c - 'A' + 3) % 26 + 'A';
            }
        } else if (std::isdigit(c)) {
            encrypted += (c - '0' + 3) % 10 + '0';
        } else {
            encrypted += c;
        }
    }
    return encrypted;
}

void startClient() {
    int sockfd;
    struct sockaddr_in serverAddr;
    char buffer[1024];

    // 创建 TCP Socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Socket creation failed!" << std::endl;
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection to server failed!" << std::endl;
        close(sockfd);
        return;
    }
    std::cout << "Client is up and running." << std::endl;

    // 用户输入
    std::string username, password, departmentName;
    std::cout << "Please enter username: ";
    std::cin >> username;

    std::cout << "Please enter password (“Enter” to skip for guests): ";
    std::cin.ignore(); // 清理缓冲
    std::getline(std::cin, password);

    std::cout << "Please enter department name: ";
    std::getline(std::cin, departmentName);

    // 加密用户名和密码
    std::string encryptedUsername = encrypt(username);
    std::string encryptedPassword = password.empty() ? "" : encrypt(password);

    // 构造请求
    std::string request = encryptedUsername + "," + encryptedPassword + "," + departmentName;
    send(sockfd, request.c_str(), request.size(), 0);
    std::cout << username <<" sent an authentication request to the main server."<<std::endl;

    // 接收服务器响应
    memset(buffer, 0, sizeof(buffer));
    recv(sockfd, buffer, sizeof(buffer), 0);

    std::string response(buffer);
    if (response == "LOGIN_SUCCESS") {
        std::cout << "Welcome " << (password.empty() ? "guest" : "member") << " " << username
                  << " from " << departmentName << "!" << std::endl;

        // 持续发送请求
        while (true) {
            std::string roomType;
            std::string actionType;
            std::cout << "Please enter the room type S/D/T:" << std::endl;
            std::getline(std::cin, roomType);
            std::cout << "Please enter request action (availability, price, reserve):" << std::endl;
            std::getline(std::cin, actionType);

            if (actionType == "availability"){
                std::string request = actionType+","+roomType;
                send(sockfd, request.c_str(), request.size(), 0);
                std::cout << username << " sent a request of " << actionType
                    << " for type " << roomType << " to the main server."
                    << std::endl;

                memset(buffer, 0, sizeof(buffer));
                recv(sockfd, buffer, sizeof(buffer), 0);
                std::cout << "The client received the response from the main server "
                    << "using TCP over port " << SERVER_PORT << "."
                    << std::endl;

                std::string receivedData(buffer);
                std::string server, roomNum, detail;

                size_t firstComma = receivedData.find(',');
                // 找到第二个逗号的位置，从第一个逗号后开始查找
                size_t secondComma = receivedData.find(',', firstComma + 1);

                if (firstComma != std::string::npos && secondComma != std::string::npos) {
                    // 提取逗号前后的字符串
                    server = receivedData.substr(0, firstComma);      
                    roomNum = receivedData.substr(firstComma + 1,secondComma - firstComma - 1);
                    detail = receivedData.substr(secondComma+1); 
                    if (roomNum== "0"){
                        std::cout << "The requested room is not available." << std::endl;
                    }else{
                        std::cout << "Campus " << server << " found " << roomNum
                            << " available rooms in " << roomType
                            << " type dormitory. Their Building IDs are: "<<detail<< std::endl;
                    }
                } else {
                    std::cout << "Not able to find the room type." << std::endl;
                }

            }else if (actionType == "price" && !password.empty()){
                std::string request = actionType+","+roomType;
                send(sockfd, request.c_str(), request.size(), 0);
                std::cout << username << " sent a request of " << actionType
                    << " for type " << roomType << " to the main server."
                    << std::endl;

                memset(buffer, 0, sizeof(buffer));
                recv(sockfd, buffer, sizeof(buffer), 0);
                std::cout << "The client received the response from the main server "
                    << "using TCP over port " << SERVER_PORT << "."
                    << std::endl;

                std::string receivedData(buffer);
                if(receivedData != "NOTFIND"){
                    std::cout << receivedData << std::endl;
                }else{
                    std::cout << "Not able to find the room type." << std::endl;
                }
            }else if (actionType == "reserve" && !password.empty()){
                std::string buildingID;
                std::cout << "Please enter Building ID for reservation:" << std::endl;
                std::getline(std::cin, buildingID);
                std::string request = actionType+","+roomType+","+buildingID;

                send(sockfd, request.c_str(), request.size(), 0);
                std::cout << username << " sent a request of " << actionType
                    << " for type " << roomType <<" and Building ID "<<buildingID<< " to the main server."
                    << std::endl;
                
                memset(buffer, 0, sizeof(buffer));
                recv(sockfd, buffer, sizeof(buffer), 0);
                std::cout << "The client received the response from the main server "
                    << "using TCP over port " << SERVER_PORT << "."
                    << std::endl;

                std::string receivedData(buffer);
                std::cout << receivedData << std::endl;
            }
        std::cout << "-----Start a new request-----" << std::endl;
        }
    } else {
        std::cout << "Failed login. Invalid username or password." << std::endl;
    }

    close(sockfd);
}


int main() {
    startClient();
    return 0;
}
