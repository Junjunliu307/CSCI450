#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT_MAIN_SERVER 33000  // 示例端口号，需根据学号更改

std::unordered_map<std::string, int> departmentServerMap;

// 初始化主服务器
void initializeMainServer(int &sockfd, sockaddr_in &serverAddr) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_MAIN_SERVER);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr));
    std::cout << "Main server is up and running." << std::endl;
}

// 接收部门列表
void receiveDepartmentList(int sockfd) {
    sockaddr_in campusAddr;
    socklen_t addrLen = sizeof(campusAddr);
    char buffer[1024];

    for (int i = 0; i < 3; ++i) { // 假设三个校园服务器
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&campusAddr, &addrLen);
        buffer[n] = '\0';
        std::string deptList(buffer);
        std::cout << "Received department list: " << deptList << std::endl;
        // 处理收到的部门列表，将部门分配到服务器
    }
}

// 处理用户查询
void processUserQuery(int sockfd) {
    std::string department, dormType;
    std::cout << "Enter department name: ";
    std::cin >> department;
    std::cout << "Enter dormitory type (S/D/T): ";
    std::cin >> dormType;

    int serverID = departmentServerMap[department];
    // 发送查询到对应校园服务器...
}

int main() {
    int sockfd;
    sockaddr_in serverAddr;
    initializeMainServer(sockfd, serverAddr);
    receiveDepartmentList(sockfd);
    while (true) {
        processUserQuery(sockfd);
    }
    close(sockfd);
    return 0;
}
