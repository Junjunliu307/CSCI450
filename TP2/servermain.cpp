#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <list>

#define PORT_MAIN_SERVER 33768  // 示例端口号，需根据学号更改
#define PORT_CAMPUS_SERVER_A 30768

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
    std::list<std::string> serverDepartmentLists;  // 用于存储每个服务器的部门列表

    for (int i = 0; i < 1; ++i) { // 假设三个校园服务器
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&campusAddr, &addrLen);
        buffer[n] = '\0';
        std::string deptList(buffer);
        // 将每个服务器的部门列表添加到 serverDepartmentLists 中
        serverDepartmentLists.push_back(deptList);
        // 输出格式化的接收信息
        char serverLabel = 'A' + i;  // 根据 i 值生成服务器标识 A/B/C
        std::cout << "Main server has received the department list from Campus server " 
                  << serverLabel << " using UDP over port " << PORT_MAIN_SERVER << std::endl;
        

        // 手动解析逗号分隔的部门列表，并将其存储到 departmentServerMap 中
        size_t pos = 0;
        while ((pos = deptList.find(',')) != std::string::npos) {
            std::string department = deptList.substr(0, pos);
            departmentServerMap[department] = PORT_CAMPUS_SERVER_A + i * 1000;  // 将部门名称与对应的服务器编号关联
            deptList.erase(0, pos + 1);           // 移除已处理的部分
        }
        // 处理最后一个部门
        departmentServerMap[deptList] = PORT_CAMPUS_SERVER_A + i * 1000;
    }
    // 按格式输出每个服务器的部门列表
    char serverLabel = 'A';
    for (std::list<std::string>::const_iterator it = serverDepartmentLists.begin(); 
         it != serverDepartmentLists.end(); ++it) {
        std::cout << "Server " << serverLabel++ << std::endl;
        std::cout << *it << std::endl;
    }
}

// 处理用户查询
void processUserQuery(int sockfd) {
    std::string department, dormType;
    std::cout << "Enter department name: ";
    std::cin >> department;
    if (departmentServerMap.find(department) == departmentServerMap.end()) {
        std::cout << department << " does not show up in Campus servers" << std::endl;
        return;
    }
    std::cout << "Enter dormitory type (S/D/T): ";
    std::cin >> dormType;
    if (dormType != "S" && dormType != "D" && dormType != "T") {
        std::cout << dormType << " does not show up in Campus servers" << std::endl;
        return;
    }

    int serverPort = departmentServerMap[department];
    // 发送查询到对应校园服务器...
    // 设置服务器地址，根据 serverID 确定对应的服务器端口和地址
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;

    // 假设服务器端口号如下（根据 serverID 确定不同端口号）
    // int serverPort = PORT_CAMPUS_SERVER_A + serverID * 1000;  // 假设 serverA = 30300, serverB = 30400, serverC = 30500
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // 本地服务器地址

    // 构建查询消息
    std::string queryMessage = department + "," + dormType;

    // 发送查询到对应校园服务器
    sendto(sockfd, queryMessage.c_str(), queryMessage.length(), 0, 
           (const struct sockaddr *)&serverAddr, sizeof(serverAddr));


    std::cout << "-----Start a new query-----" << std::endl;
}

void requestDepartmentList(int sockfd, const sockaddr_in &serverAAddr) {
    std::string request = "REQUEST_DEPARTMENT_LIST";
    sendto(sockfd, request.c_str(), request.length(), 0, (const struct sockaddr *)&serverAAddr, sizeof(serverAAddr));
}

int main() {
    int sockfd;
    sockaddr_in serverAddr;
    initializeMainServer(sockfd, serverAddr);

    for (int i = 0; i < 1; ++i) {
        // 构造 serverA 的地址信息
        sockaddr_in serverAAddr;
        memset(&serverAAddr, 0, sizeof(serverAAddr));
        serverAAddr.sin_family = AF_INET;
        serverAAddr.sin_port = htons(PORT_CAMPUS_SERVER_A+i*1000); // serverA 的端口号，根据实际端口调整
        serverAAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        // 发送请求以获取 serverA 的部门列表
        requestDepartmentList(sockfd, serverAAddr);
    }

    // 接收部门列表
    receiveDepartmentList(sockfd);
    std::cout << std::endl;

    while (true) {
        processUserQuery(sockfd);
        std::cout << std::endl;
    }
    close(sockfd);
    return 0;
}

