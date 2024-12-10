#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 35319 // Main server TCP port

std::string encrypt(const std::string& input) {
    std::string result;
    for (char c : input) {
        if (std::isalpha(c)) {
            result += (c + 3 > 'z' ? c - 23 : c + 3); // Example shift
        } else {
            result += c;
        }
    }
    return result;
}

void startClient() {
    int sockfd;
    struct sockaddr_in serverAddr;
    char buffer[1024];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    std::cout << "Client is up and running." << std::endl;

    std::string username, password;
    std::cout << "Enter username: ";
    std::cin >> username;
    std::cout << "Enter password: ";
    std::cin >> password;

    std::string encryptedPassword = encrypt(password);
    std::string loginRequest = username + "," + encryptedPassword;
    send(sockfd, loginRequest.c_str(), loginRequest.size(), 0);

    memset(buffer, 0, sizeof(buffer));
    recv(sockfd, buffer, sizeof(buffer), 0);
    std::cout << "Server Response: " << buffer << std::endl;

    close(sockfd);
}

int main() {
    startClient();
    return 0;
}
