#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT_CAMPUS_SERVER_A 30300  // 根据学号最后三位修改

std::unordered_map<std::string, int> dormitoryDataA;

void loadDataA() {
    std::ifstream file("dataA.txt");
    std::string department;
    while (std::getline(file, department, ',')) {
        dormitoryDataA[department] = 0; // 初始化数据
    }
    file.close();
}

void initializeCampusServer(int &sockfd, sockaddr_in &serverAddr) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_CAMPUS_SERVER_A);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr));
    std::cout << "Server A is up and running using UDP on port " << PORT_CAMPUS_SERVER_A << std::endl;
}

void handleQuery(int sockfd) {
    sockaddr_in mainServerAddr;
    socklen_t addrLen = sizeof(mainServerAddr);
    char buffer[1024];

    int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&mainServerAddr, &addrLen);
    buffer[n] = '\0';
    std::string query(buffer);
    std::cout << "Server A received request for " << query << std::endl;

    std::string response = dormitoryDataA.find(query) != dormitoryDataA.end() ?
                           "Available rooms in " + query :
                           "No data available for department " + query;

    sendto(sockfd, response.c_str(), response.length(), 0, (const struct sockaddr *)&mainServerAddr, addrLen);
    std::cout << "Server A has sent the results to Main Server" << std::endl;
}

int main() {
    int sockfd;
    sockaddr_in serverAddr;
    initializeCampusServer(sockfd, serverAddr);
    loadDataA();
    while (true) {
        handleQuery(sockfd);
    }
    close(sockfd);
    return 0;
}
