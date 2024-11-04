#include <iostream>
#include <string>
#include <list>
#include <unordered_map>
#include <fstream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <cstring>
#include <vector>

#define PORT_CAMPUS_SERVER_C 32768  // 根据学号最后三位修改

struct RoomInfo {
    int buildingID;
    int availability;
    int price;
};

std::string departmentList; // 用于存储第一行的部门列表
std::unordered_map<std::string, std::list<RoomInfo> > dormitoryDataA; // 用于存储宿舍数据

void loadDataC() {
    std::ifstream file("dataC.txt");
    if (!file.is_open()) {
        std::cerr << "Failed to open dataC.txt" << std::endl;
        return;
    }

    // 读取第一行并存储在 departmentList 中
    std::getline(file, departmentList);

    // 读取后续行并存储在 dormitoryDataA 中
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string roomType, buildingIDStr, availabilityStr, priceStr;

        // 按逗号分隔每个字段
        if (std::getline(ss, roomType, ',') &&
            std::getline(ss, buildingIDStr, ',') &&
            std::getline(ss, availabilityStr, ',') &&
            std::getline(ss, priceStr, ',')) {
            
            int buildingID = std::stoi(buildingIDStr);
            int availability = std::stoi(availabilityStr);
            int price = std::stoi(priceStr);

            // 将每个房间的详细信息存储在 dormitoryDataA 的 vector 中
            RoomInfo room = {buildingID, availability, price};
            dormitoryDataA[roomType].push_back(room);
        }
    }

    file.close();
}

void initializeCampusServer(int &sockfd, sockaddr_in &serverAddr) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_CAMPUS_SERVER_C);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr));
    std::cout << "Server C is up and running using UDP on port " << PORT_CAMPUS_SERVER_C << std::endl;
}

void handleRequest(int sockfd) {
    sockaddr_in mainServerAddr;
    socklen_t addrLen = sizeof(mainServerAddr);
    char buffer[1024];

    // 接收主服务器的请求
    int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&mainServerAddr, &addrLen);
    buffer[n] = '\0';
    std::string request(buffer);

    if (request == "REQUEST_DEPARTMENT_LIST") {
        // 如果收到请求部门列表的请求
        std::string response = std::to_string(PORT_CAMPUS_SERVER_C)+","+departmentList;
        sendto(sockfd, response.c_str(), response.length(), 0, (const struct sockaddr *)&mainServerAddr, addrLen);
        std::cout << "Server C has sent a department list to Main Server" << std::endl;
    } else {
        std::istringstream ss(request);
        std::string department, dormType;
        if (std::getline(ss, department, ',') && std::getline(ss, dormType)) {
            std::cout << "Server C has received a request for department " << department
                      << " dormitory type " << dormType << std::endl;

            // 检查请求的部门和宿舍类型是否存在
            if (dormitoryDataA.find(dormType) != dormitoryDataA.end()) {
                std::vector<int> buildingIDs;
                int totalAvailability = 0;

                // 遍历数据结构以查找指定类型的宿舍信息
                std::list<RoomInfo>::iterator it = dormitoryDataA[dormType].begin();
                for (; it != dormitoryDataA[dormType].end(); ++it) {
                    buildingIDs.push_back(it->buildingID);
                    totalAvailability += it->availability;
                }

                // 构建并发送响应消息
                std::ostringstream printFormat;
                std::ostringstream responseFormat;
                printFormat << "Server C found totally " << totalAvailability << " available rooms for "
                         << dormType << " type dormitory in Building: ";
                responseFormat << "There are totally " << totalAvailability << " available rooms for "
                         << dormType << " type dormitory in Building: ";

                for (size_t i = 0; i < buildingIDs.size(); ++i) {
                    printFormat << buildingIDs[i];
                    responseFormat << buildingIDs[i];
                    if (i < buildingIDs.size() - 1) {
                        printFormat << ", ";
                        responseFormat << ", ";
                    }
                }
                
                std::string printFormatStr = printFormat.str();
                std::string responseFormatStr = responseFormat.str();
                std::cout << printFormatStr << std::endl;

                sendto(sockfd, responseFormatStr.c_str(), responseFormatStr.length(), 0, (const struct sockaddr *)&mainServerAddr, addrLen);
                std::cout << "Server C has sent the results to Main Server" << std::endl;
            } else {
                std::string errorMsg = "Department or dormitory type not found";
                sendto(sockfd, errorMsg.c_str(), errorMsg.length(), 0, (const struct sockaddr *)&mainServerAddr, addrLen);
            }
        }
    }
}


int main() {
    int sockfd;
    sockaddr_in serverAddr;
    initializeCampusServer(sockfd, serverAddr);
    loadDataC();
    while (true) {
        handleRequest(sockfd);
    }
    close(sockfd);
    return 0;
}
