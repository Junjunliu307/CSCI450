#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 31319 // Example for Server A, adjust for B/C

struct Room {
    std::string type;
    int buildingID;
    int availability;
    int price;
};

std::map<std::string, std::vector<Room>> departmentData; // Store department -> Room details

void loadData(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    std::getline(file, line); // First line: department names
    std::stringstream ss(line);
    std::string department;
    while (std::getline(ss, department, ',')) {
        departmentData[department] = {};
    }
    // Parse room data
    while (std::getline(file, line)) {
        Room room;
        std::stringstream ss(line);
        std::string value;
        std::getline(ss, room.type, ',');
        ss >> room.buildingID;
        ss.ignore(1, ','); // Skip comma
        ss >> room.availability;
        ss.ignore(1, ',');
        ss >> room.price;
        for (auto& dept : departmentData) {
            dept.second.push_back(room);
        }
    }
    file.close();
}

void startServer() {
    int sockfd;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[1024];
    socklen_t addrLen = sizeof(clientAddr);

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    std::cout << "Server A is up and running using UDP on port " << PORT << std::endl;

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &addrLen);

        std::string request(buffer);
        // Process request here, e.g., "availability S"
        std::string response = "Processed Request: " + request;
        sendto(sockfd, response.c_str(), response.size(), 0, (struct sockaddr*)&clientAddr, addrLen);
    }
    close(sockfd);
}

int main() {
    loadData("dataA.txt"); // Adjust for Server B/C
    startServer();
    return 0;
}
