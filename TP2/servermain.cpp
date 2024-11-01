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
    std::list<std::string> serverDepartmentLists(3);  // 用于存储每个服务器的部门列表

    for (int i = 0; i < 3; ++i) { // 假设三个校园服务器
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&campusAddr, &addrLen);
        buffer[n] = '\0';

        std::string receivedStr(buffer);
        size_t commaPos = receivedStr.find(',');
        std::string deptList = receivedStr.substr(commaPos + 1);
        int serverPortNum = std::stoi(receivedStr.substr(0, commaPos));

        // 将每个服务器的部门列表添加到 serverDepartmentLists 中
        std::list<std::string>::iterator it = serverDepartmentLists.begin();
        std::advance(it, (serverPortNum-PORT_CAMPUS_SERVER_A)/1000);  // 将迭代器移动到第 i 个位置
        *it = deptList;
        // 输出格式化的接收信息
        char serverLabel = 'A' + i;  // 根据 i 值生成服务器标识 A/B/C
        std::cout << "Main server has received the department list from Campus server " 
                  << serverLabel << " using UDP over port " << PORT_MAIN_SERVER << std::endl;
        

        // 手动解析逗号分隔的部门列表，并将其存储到 departmentServerMap 中
        size_t pos = 0;
        while ((pos = deptList.find(',')) != std::string::npos) {
            std::string department = deptList.substr(0, pos);
            departmentServerMap[department] = serverPortNum;  // 将部门名称与对应的服务器编号关联
            deptList.erase(0, pos + 1);           // 移除已处理的部分
        }
        // 处理最后一个部门
        departmentServerMap[deptList] = serverPortNum;
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
    char serverName = 'A'+(PORT_CAMPUS_SERVER_A-serverPort)/1000;
    std::cout << department << " shows up in server " << serverName << std::endl;
    // 发送查询到对应校园服务器...
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;

    // 假设服务器端口号如下（根据 serverID 确定不同端口号）
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // 本地服务器地址

    // 构建查询消息
    std::string queryMessage = department + "," + dormType;

    // 发送查询到对应校园服务器
    sendto(sockfd, queryMessage.c_str(), queryMessage.length(), 0, 
           (const struct sockaddr *)&serverAddr, sizeof(serverAddr));

    std::cout << "The Main Server has sent query for " << department
              << " department and " << dormType << " type dormitory to server "
              << serverName << " using UDP over port " << PORT_MAIN_SERVER << std::endl;
    
    char buffer[1024];
    sockaddr_in fromAddr;
    socklen_t fromAddrLen = sizeof(fromAddr);
    
    int recvResult = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, 
                              (struct sockaddr *)&fromAddr, &fromAddrLen);
    if (recvResult < 0) {
        std::cerr << "Error receiving response from server" << std::endl;
        return;
    }
    
    buffer[recvResult] = '\0';  // 确保缓冲区正确结束
    std::cout << "The Main server has received searching result(s) of "
              << dormType << " type dormitory from Campus server " << serverName << std::endl;
    std::cout << buffer << std::endl;

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

    for (int i = 0; i < 3; ++i) {
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

