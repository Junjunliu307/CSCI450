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
#include <set>

#define PORT_CAMPUS_SERVER_C 33768 

struct RoomInfo {
    int buildingID;
    int availability;
    int price;
};

std::string departmentList; 
std::set<std::string> buildingSet;
std::unordered_map<std::string, std::list<RoomInfo> > dormitoryData; // 用于存储宿舍数据

void loadDataA() {
    std::ifstream file("dataC.txt");
    if (!file.is_open()) {
        std::cerr << "Failed to open dataC.txt" << std::endl;
        return;
    }

    
    std::getline(file, departmentList);

    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string roomType, buildingIDStr, availabilityStr, priceStr;

        
        if (std::getline(ss, roomType, ',') &&
            std::getline(ss, buildingIDStr, ',') &&
            std::getline(ss, availabilityStr, ',') &&
            std::getline(ss, priceStr, ',')) {
            
            int buildingID = std::stoi(buildingIDStr);
            int availability = std::stoi(availabilityStr);
            int price = std::stoi(priceStr);

            
            RoomInfo room = {buildingID, availability, price};
            dormitoryData[roomType].push_back(room);
            buildingSet.insert(buildingIDStr);
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

    
    int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&mainServerAddr, &addrLen);
    buffer[n] = '\0';
    std::string request(buffer);

    if (request == "REQUEST_DEPARTMENT_LIST") {
        
        std::string response = std::to_string(PORT_CAMPUS_SERVER_C)+","+departmentList;
        sendto(sockfd, response.c_str(), response.length(), 0, (const struct sockaddr *)&mainServerAddr, addrLen);
        std::cout << "Server C has sent a department list to Main Server" << std::endl;
    } else {
        size_t commaPos = request.find(',');

        std::string requestType,requestParam;
        if (commaPos != std::string::npos) {
            requestType = request.substr(0, commaPos);
            requestParam = request.substr(commaPos + 1);
        }

        if (requestType == "availability") {
            std::cout << "Server C has received a query of Availability for room type "
                      << requestParam << std::endl;
            
            if (dormitoryData.find(requestParam) != dormitoryData.end()) {
                std::vector<int> buildingIDs;
                int totalAvailability = 0;
                
                std::list<RoomInfo>::iterator it = dormitoryData[requestParam].begin();
                for (; it != dormitoryData[requestParam].end(); ++it) {
                    buildingIDs.push_back(it->buildingID);
                    totalAvailability += it->availability;
                }

                
                std::ostringstream printFormat;
                std::ostringstream responseFormat;
                printFormat << "Server C found totally " << totalAvailability << " available rooms for "
                         << requestParam << " type dormitory in Building: ";
                responseFormat << "C,"<<totalAvailability << ",";

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
                std::cout<< "Room type "<< requestParam <<" does not show up in Server C" << std::endl;
                std::string errorMsg = "Not able to find the room type.";
                sendto(sockfd, errorMsg.c_str(), errorMsg.length(), 0, (const struct sockaddr *)&mainServerAddr, addrLen);
            }
        }else if (requestType == "price"){
            std::cout << "Server C has received a query of Price for room type "
                << requestParam << std::endl;
            if (dormitoryData.find(requestParam) != dormitoryData.end()) {
                std::vector<int> buildingIDs;
                std::vector<int> prices;
                
                std::list<RoomInfo>::iterator it = dormitoryData[requestParam].begin();
                for (; it != dormitoryData[requestParam].end(); ++it) {
                    buildingIDs.push_back(it->buildingID);
                    prices.push_back(it->price);
                }

                
                std::ostringstream printFormat;
                std::ostringstream responseFormat;
                printFormat << "Server C found room type " << requestParam << " with prices:"<<std::endl;
                responseFormat << "Server C found room type " << requestParam << " with prices:"<<std::endl;

                for (size_t i = 0; i < buildingIDs.size(); ++i) {
                    printFormat << "Building ID "<< buildingIDs[i]<<", Price $ "<<prices[i]<<std::endl;
                    responseFormat << "Building ID "<< buildingIDs[i]<<", Price $ "<<prices[i]<<std::endl;
                }
                
                std::string printFormatStr = printFormat.str();
                std::string responseFormatStr = responseFormat.str();
                std::cout << printFormatStr << std::endl;

                sendto(sockfd, responseFormatStr.c_str(), responseFormatStr.length(), 0, (const struct sockaddr *)&mainServerAddr, addrLen);
                std::cout << "Server C has sent the results to Main Server" << std::endl;
                
            }else{
                std::cout<< "Room type "<< requestParam <<" does not show up in Server C" << std::endl;
                std::string errorMsg = "Not able to find the room type.";
                sendto(sockfd, errorMsg.c_str(), errorMsg.length(), 0, (const struct sockaddr *)&mainServerAddr, addrLen);
            }
        }else if (requestType == "reserve"){
            std::string dormType,buildingID;
            size_t commaPos = requestParam.find(',');
            if (commaPos != std::string::npos) {
                dormType = requestParam.substr(0, commaPos);               
                buildingID = requestParam.substr(commaPos + 1);             
            }
            std::cout << "Server C has received a query of Reserve for room type "
                << dormType <<" at Building ID "<<buildingID<< std::endl;

            if (dormitoryData.find(dormType) == dormitoryData.end()){
                std::cout<< "Room type "<< dormType <<" does not show up in Server C" << std::endl;
                std::string errorMsg = "Reservation failed: Not able to find the room type.";
                sendto(sockfd, errorMsg.c_str(), errorMsg.length(), 0, (const struct sockaddr *)&mainServerAddr, addrLen);
            }
            else if (buildingSet.count(buildingID) == 0){
                std::string errorMsg = "Reservation failed: BuildingID "+buildingID+" does not exist.";
                sendto(sockfd, errorMsg.c_str(), errorMsg.length(), 0, (const struct sockaddr *)&mainServerAddr, addrLen);
            }else{
                std::string response;
                for (auto& room : dormitoryData[dormType]) {
                    if (std::to_string(room.buildingID) == buildingID && room.availability > 0){
                        room.availability -= 1;
                        response = "Reservation is successful for Campus C Building ID " + buildingID +"!";
                        break;
                    }
                }
                if(response.empty()){
                    response = "Reservation failed: Building ID " + buildingID+" room type " + dormType + " is not available.";
                }
                sendto(sockfd, response.c_str(), response.length(), 0, (const struct sockaddr *)&mainServerAddr, addrLen);
            }
        }
        std::cout<< "Server C has sent the results to Main Server"<<std::endl;

    }
}


int main() {
    int sockfd;
    sockaddr_in serverAddr;
    initializeCampusServer(sockfd, serverAddr);
    loadDataA();
    while (true) {
        handleRequest(sockfd);
    }
    close(sockfd);
    return 0;
}

