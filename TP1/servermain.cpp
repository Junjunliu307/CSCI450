#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h> 
#include <fstream>
#include <vector>
#include <set>
#include <string>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <map>

#define PORT 24768  
#define BUFFER_SIZE 1024
// int client_counter = 0;
// std::map<std::string, int> client_map;

struct Campus {
    std::string campusID;
    std::vector<std::string> departments;
};

std::vector<Campus> campus_data;

void load_data(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open list.txt" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string line;
    while (std::getline(file, line)) {
        Campus campus;
        campus.campusID = line;
        // std::cout << campus.campusID << std::endl;

        if (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string department;
            while (std::getline(ss, department, ';')) {
                campus.departments.push_back(department);
            }
            campus_data.push_back(campus);
        }
    }
    file.close();
    std::cout << "Main server has read the department list from list.txt" << std::endl;

    std::cout << "Total number of Campus Servers: " << campus_data.size() << std::endl;

    for (size_t i = 0; i < campus_data.size(); ++i) {
        const Campus &campus = campus_data[i];
        std::set<std::string> unique_departments(campus.departments.begin(), campus.departments.end());
        std::cout << "Campus Server " << std::stoi(campus.campusID)
                  << " contains " << unique_departments.size() 
                  << " distinct departments" << std::endl;
    }
    std::cout << std::endl;
}

std::string search_department(const std::string &department) {
    for (size_t i = 0; i < campus_data.size(); ++i) {
        const Campus &campus = campus_data[i];
        for (size_t j = 0; j < campus.departments.size(); ++j) {
            if (campus.departments[j] == department) {
                return campus.campusID;
            }
        }
    }
    return "Not found";
}

void handle_client(int client_sock) {
    // send(client_sock, &client_id, sizeof(client_id), 0);

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    char buffer[BUFFER_SIZE];
    int n = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (n < 0) {
        perror("Failed to receive data");
        exit(EXIT_FAILURE);
    }
    buffer[n] = '\0';

    std::string query(buffer);
    size_t pos = query.find(":");
    int received_unique_id = std::stoi(query.substr(0, pos));
    std::string department = query.substr(pos + 1);
    
    int client_id = received_unique_id;

    // int client_id;
    // std::map<std::string, int>::iterator it = client_map.find(received_unique_id);
    // if (it == client_map.end()) {
    //     std::cout << received_unique_id << " Not Found" << std::endl;
    //     client_id = client_counter++;
    //     client_map[received_unique_id] = client_id;
    // } else {
    //     std::cout << received_unique_id << " Found" << std::endl;
    //     client_id = it->second;
    // }

    // std::cout << "Current contents of client_map:" << std::endl;
    // for (std::map<std::string, int>::iterator it = client_map.begin(); it != client_map.end(); ++it) {
    //     std::cout << "Unique ID: \"" << it->first << "\", Client ID: " << it->second << std::endl;
    // }

    std::cout << "Main server has received the request on Department " 
              << department << " from client "<<client_id <<" using TCP." << std::endl;

    std::string result = search_department(department);

    send(client_sock, result.c_str(), result.length(), 0);
    if(result == "Not found"){
        std::cout << department << " does not show up in Campus server ";
        std::cout << "< ";
        for (size_t i = 0; i < campus_data.size(); ++i) {
            std::cout << std::stoi(campus_data[i].campusID);
            if (i < campus_data.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << " >" << std::endl;
        std::cout << "The Main Server has sent “Department Name: Not found” "
            << "to client "<< client_id << " using TCP over port " 
            << PORT << std::endl;

    }else{
        std::cout << department << " shows up in Campus server " 
            << std::stoi(result) << std::endl;
        std::cout << "Main Server has sent searching result to client " 
            << client_id << " using TCP over port " 
            << PORT << std::endl;
    }
    std::cout << std::endl;
    close(client_sock);
}

int main() {
    std::cout << "Main server is up and running."<< std::endl;
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    load_data("list.txt");

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    listen(server_sock, 3);

    while (true) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Failed to accept connection");
            exit(EXIT_FAILURE);
        }

        if (fork() == 0) {
            close(server_sock);
            handle_client(client_sock);
            exit(0);
        }
        close(client_sock);
    }

    close(server_sock);
    return 0;
}
