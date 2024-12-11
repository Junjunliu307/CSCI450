#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <list>
#include <map>
#include <fstream>
#include <sstream> 
#include <thread>

#define PORT_UDP 34768 
#define PORT_TCP 35768 
#define PORT_CAMPUS_SERVER_A 31768

std::map<std::string, std::string> loginData; 
std::unordered_map<std::string, int> departmentServerMap;

// 加载登录数据
void loadLoginData() {
    std::ifstream file("member.txt");
    std::string line;
    while (std::getline(file, line)) {
        size_t commaPos = line.find(',');
        std::string username = line.substr(0, commaPos);
        std::string password = line.substr(commaPos + 1);
        loginData[username] = password;
    }
    file.close();
}

// 验证登录信息
bool handleLogin(const std::string& username, const std::string& password) {
    if (loginData.find(username) != loginData.end() && loginData[username] == password) {
        return true;
    }
    return false;
}


std::string processUserQuery(int sockfd,std::string requestType,std::string department, std::string dormType,std::string buildingID) {
    if (departmentServerMap.find(department) == departmentServerMap.end()) {
        std::cout << department << " does not show up in Campus servers" << std::endl;
        return "NOTFIND";
    }

    int serverPort = departmentServerMap[department];
    char serverName = 'A'+(PORT_CAMPUS_SERVER_A-serverPort)/1000;

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;

    
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    
    std::string queryMessage;
    if(buildingID.empty()){
        queryMessage = requestType + "," + dormType;
    }else{
        queryMessage = requestType + "," + dormType + ","+buildingID;
    }
    
    sendto(sockfd, queryMessage.c_str(), queryMessage.length(), 0, 
           (const struct sockaddr *)&serverAddr, sizeof(serverAddr));

    std::cout << "The main server forwarded a request of " 
              << requestType 
              << " to Server " << serverName
              << " using UDP over port " << PORT_UDP << "." 
              << std::endl;
    
    char buffer[1024];
    sockaddr_in fromAddr;
    socklen_t fromAddrLen = sizeof(fromAddr);
    
    int recvResult = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, 
                              (struct sockaddr *)&fromAddr, &fromAddrLen);

    std::cout << "The Main server has received result for the request of " 
        << requestType 
        << " from Campus server " << serverName 
        << " using UDP over port " << PORT_UDP << "." 
        << std::endl;

    if (recvResult < 0) {
        std::cerr << "Error receiving response from server" << std::endl;
        return "NOTFIND";
    }
    
    buffer[recvResult] = '\0';  
    std::string receivedData(buffer);
    return receivedData;
}

void handleClient(int clientSock, int udpSockfd) {
    char buffer[1024];
    bool authenticated = false;
    std::string username;
    std::string password;
    std::string departmentName;

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSock, buffer, sizeof(buffer), 0);

        if (bytesReceived <= 0) {
            std::cout << "Client disconnected." << std::endl;
            break;
        }

        std::string request(buffer);

        if (!authenticated) {
            size_t firstComma = request.find(',');
            size_t secondComma = request.find(',', firstComma + 1);

            username = request.substr(0, firstComma);
            password = request.substr(firstComma + 1, secondComma - firstComma - 1);
            departmentName = request.substr(secondComma + 1);

            if (password.empty()) {
                std::cout << "The main server received the guest request for " << username << " using TCP over port " << PORT_TCP << "." << std::endl;
                std::cout << "The main server accepts " << username << " as a guest." << std::endl;
                send(clientSock, "LOGIN_SUCCESS", strlen("LOGIN_SUCCESS"), 0);
                std::cout << "The main server sent the guest response to the client." << std::endl;
                authenticated = true;
            } else {
                std::cout << "The main server received the authentication for " << username << " using TCP over port " << PORT_TCP << "." << std::endl;
                if (handleLogin(username, password)) {
                    std::cout << "The authentication passed." << std::endl;
                    send(clientSock, "LOGIN_SUCCESS", strlen("LOGIN_SUCCESS"), 0);
                    std::cout << "The main server sent the authentication result to the client." << std::endl;
                    authenticated = true;
                } else {
                    std::cout << "The authentication failed." << std::endl;
                    send(clientSock, "LOGIN_FAILED", strlen("LOGIN_FAILED"), 0);
                    std::cout << "The main server sent the authentication result to the client." << std::endl;
                    break;
                }
            }
        } else {
            size_t commaPos = request.find(',');
            std::string requestType, requestParam; 
            if (commaPos != std::string::npos) {
                requestType = request.substr(0, commaPos);               
                requestParam = request.substr(commaPos + 1);           
            }

            std::cout << "Main server has received the query from "
                <<(password.empty() ? "guest" : "member") << " " << username
                << " in " << departmentName
                << " for the request of " << requestType << "." << std::endl;
                
            std::string response;
            if (requestType == "reserve"){
                std::string dormType,buildingID;
                size_t commaPos2 = requestParam.find(',');
                if (commaPos2 != std::string::npos) {
                    dormType = requestParam.substr(0, commaPos2);               
                    buildingID = requestParam.substr(commaPos2 + 1);             
                }
                response = processUserQuery(udpSockfd,requestType,departmentName,dormType,buildingID);
            }else{
                response = processUserQuery(udpSockfd,requestType,departmentName,requestParam,"");
            }

            send(clientSock, response.c_str(), response.size(), 0);
            std::cout << "The Main server has sent back the result for the request of "
                << requestType
                << " to the client " << (password.empty() ? "guest" : "member")
                << " " << username
                << " using TCP over port " << PORT_TCP << "."
                << std::endl;
        }
    }

    close(clientSock); 
}

void startTCPServer(int udpSockfd) {
    int sockfd, clientSock;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_TCP);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return;
    }

    if (listen(sockfd, 5) < 0) {
        perror("Listen failed");
        close(sockfd);
        return;
    }

    std::cout << "Main server is up and running using TCP on port " << PORT_TCP << "." << std::endl;

    while (true) {
        clientSock = accept(sockfd, (struct sockaddr*)&clientAddr, &addrLen);
        if (clientSock < 0) {
            perror("Accept failed");
            continue;
        }

        std::thread clientThread(handleClient, clientSock, udpSockfd);
        clientThread.detach();
    }

    close(sockfd);
}


void startUDPServer(int &sockfd, sockaddr_in &serverAddr) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_UDP);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr));
}


void receiveDepartmentList(int sockfd) {
    sockaddr_in campusAddr;
    socklen_t addrLen = sizeof(campusAddr);
    char buffer[1024];
    std::list<std::string> serverDepartmentLists(3);  

    for (int i = 0; i < 3; ++i) { 
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&campusAddr, &addrLen);
        buffer[n] = '\0';

        std::string receivedStr(buffer);
        size_t commaPos = receivedStr.find(',');
        std::string deptList = receivedStr.substr(commaPos + 1);
        int serverPortNum = std::stoi(receivedStr.substr(0, commaPos));

        std::list<std::string>::iterator it = serverDepartmentLists.begin();
        std::advance(it, (serverPortNum-PORT_CAMPUS_SERVER_A)/1000);
        *it = deptList;
        
        char serverLabel = 'A' + i;  
        std::cout << "Main server has received the department list from Campus server " 
                  << serverLabel << " using UDP over port " << PORT_UDP << std::endl;
        

        
        size_t pos = 0;
        while ((pos = deptList.find(',')) != std::string::npos) {
            std::string department = deptList.substr(0, pos);
            departmentServerMap[department] = serverPortNum; 
            deptList.erase(0, pos + 1);           
        }
        
        departmentServerMap[deptList] = serverPortNum;
    }
    
    char serverLabel = 'A';
    for (std::list<std::string>::const_iterator it = serverDepartmentLists.begin(); 
         it != serverDepartmentLists.end(); ++it) {
        std::cout << "Server " << serverLabel++ << std::endl;
        std::cout << *it << std::endl;
    }
}


void requestDepartmentList(int sockfd, const sockaddr_in &serverAAddr) {
    std::string request = "REQUEST_DEPARTMENT_LIST";
    sendto(sockfd, request.c_str(), request.length(), 0, (const struct sockaddr *)&serverAAddr, sizeof(serverAAddr));
}


int main() {
    int udpSockfd;
    sockaddr_in serverAddr;
    startUDPServer(udpSockfd, serverAddr);
    std::cout << "Main server is up and running." << std::endl;

    for (int i = 0; i < 3; ++i) {
        
        sockaddr_in serverAAddr;
        memset(&serverAAddr, 0, sizeof(serverAAddr));
        serverAAddr.sin_family = AF_INET;
        serverAAddr.sin_port = htons(PORT_CAMPUS_SERVER_A+i*1000);
        serverAAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        requestDepartmentList(udpSockfd, serverAAddr);
    }

    receiveDepartmentList(udpSockfd);
    
    loadLoginData();
    startTCPServer(udpSockfd);


    return 0;
}
