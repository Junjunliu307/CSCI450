#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <sstream>

#define PORT 24768
#define BUFFER_SIZE 1024

int string_to_int(const std::string &str) {
    std::stringstream ss(str);
    int num;
    ss >> num;
    return num;
}

std::string generate_unique_id() {
    return std::to_string(getpid());
}

int main() {
    int sock;
    struct sockaddr_in server_addr,local_addr;
    socklen_t addr_len = sizeof(local_addr);
    
    char buffer[BUFFER_SIZE];

    std::cout << "Client is up and running." << std::endl;
    std::cout << std::endl;
    std::string unique_id = generate_unique_id();

    while (true) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("Failed to create socket");
            exit(EXIT_FAILURE);
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Failed to connect to the server");
            close(sock);
            exit(EXIT_FAILURE);
        }

        if (getsockname(sock, (struct sockaddr *)&local_addr, &addr_len) == -1) {
            perror("getsockname");
            close(sock);
            exit(EXIT_FAILURE);
        }

        int dynamic_port = ntohs(local_addr.sin_port);

        std::cout << "Enter Department Name: ";
        std::string department;
        std::cin >> department;

        std::stringstream ss;
        ss << unique_id;
        std::string query = ss.str() + ":" + department;
        send(sock, query.c_str(), query.length(), 0);
        std::cout << "Client has sent Department " << department 
                  << " to Main Server using TCP over port " << dynamic_port <<"."<< std::endl;

        memset(buffer, 0, BUFFER_SIZE);
        int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (n < 0) {
            perror("Failed to receive data");
            close(sock);
            continue;
        }
        buffer[n] = '\0';

        if (std::string(buffer) == "Not found") {
            std::cout << department << " not found." << std::endl;
        } else {
            std::cout << "Client has received results from Main Server:"<< std::endl;
            std::cout << department << " is associated with Campus server " 
                      << string_to_int(buffer) << '.' << std::endl;
        }

        std::cout << std::endl;
        std::cout << "-----Start a new query-----" << std::endl;
        close(sock);
    }

    return 0;
}
