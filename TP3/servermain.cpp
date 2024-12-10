#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 34319 // Main server UDP port

std::map<std::string, std::string> loginData; // Store username -> encrypted password
std::map<std::string, int> campusPorts = {{"A", 31319}, {"B", 32319}, {"C", 33319}};

void loadLoginData() {
    std::ifstream file("login.txt");
    std::string line;
    while (std::getline(file, line)) {
        size_t commaPos = line.find(',');
        std::string username = line.substr(0, commaPos);
        std::string password = line.substr(commaPos + 1);
        loginData[username] = password;
    }
    file.close();
}

void handleLogin(int clientSock, const std::string& username, const std::string& password) {
    std::string encryptedPassword = loginData[username];
    if (encryptedPassword == password) {
        std::string successMsg = "Login successful for " + username;
        send(clientSock, successMsg.c_str(), successMsg.size(), 0);
    } else {
        std::string failMsg = "Invalid username/password.";
        send(clientSock, failMsg.c_str(), failMsg.size(), 0);
    }
}

void startServer() {
    int sockfd;
    struct sockaddr_in serverAddr;
    char buffer[1024];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    std::cout << "Main server is up and running using UDP on port " << PORT << std::endl;

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        // Process incoming requests here
    }
    close(sockfd);
}

int main() {
    loadLoginData();
    startServer();
    return 0;
}
