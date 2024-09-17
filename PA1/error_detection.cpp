#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <vector>

using namespace std;

// CRC-16 generator polynomial: x^16 + x^10 + x^4 + 1 -> 10000010000010001
const int CRC_POLY = 0x11021; 
const int CRC_BITS = 16;

// Function to convert binary string to an integer
unsigned int binaryStringToInt(const string& str) {
    unsigned int value = 0;
    for (size_t i = 0; i < str.length(); ++i) {
        value = (value << 1) + (str[i] - '0'); // Convert binary string to integer bit by bit
    }
    return value;
}


// Function to calculate CRC-16
string calculateCRC(const string& data) {
    unsigned int codeword = binaryStringToInt(data); // Convert binary string to integer
    codeword <<= CRC_BITS; // Append 16 zero bits to the data

    for (int i = data.length(); i > 0; i--) {
        if (codeword & (1 << (i + CRC_BITS - 1))) {
            codeword ^= (CRC_POLY << (i - 1));
        }
    }

    return bitset<CRC_BITS>(codeword).to_string(); // Return the 16-bit CRC
}

// Function to calculate 16-bit checksum
string calculateChecksum(const string& data) {
    unsigned int sum = 0;
    for (size_t i = 0; i < data.length(); i += 16) {
        string block = data.substr(i, 16);
        unsigned int value = binaryStringToInt(block);
        sum += value;
        sum = (sum >> 16) + (sum & 0xFFFF); // Add overflow
    }
    sum = ~sum & 0xFFFF; // 1's complement of the sum
    return bitset<16>(sum).to_string();
}

// Function to introduce errors using XOR
string introduceErrors(const string& data, const string& errorBits) {
    string result = data;
    for (size_t i = 0; i < data.length(); ++i) {
        if (errorBits[i] == '1') {
            result[i] = (result[i] == '0') ? '1' : '0'; // Flip the bit
        }
    }
    return result;
}

// Function to print the result (Pass/Not Pass)
void printResult(const string& data, const string& errorBits) {
    // Append CRC and checksum
    string crc = calculateCRC(data);
    string checksum = calculateChecksum(data);
    string codeword_crc = data + crc;
    string codeword_checksum = data + checksum;

    // Introduce errors
    string received_crc = introduceErrors(codeword_crc, errorBits);
    string received_checksum = introduceErrors(codeword_checksum, errorBits);

    // Recalculate CRC and checksum for the received data
    string recalculated_crc = calculateCRC(received_crc.substr(0, data.length()));
    string recalculated_checksum = calculateChecksum(received_checksum.substr(0, data.length()));

    // Print results for CRC
    cout << "CRC-16" << endl;
    cout << "CRC: " << crc << "; \t\tResult: " 
         << ((recalculated_crc == received_crc.substr(data.length())) ? "Pass" : "Not Pass") 
         << endl;

    // Print results for checksum
    cout << "Checksum" << endl;
    cout << "Checksum: " << checksum << "; \tResult: " 
         << ((recalculated_checksum == received_checksum.substr(data.length())) ? "Pass" : "Not Pass") 
         << endl;
}

// Function to read data from file and process each line
void processFile(const string& filename) {
    ifstream infile(filename);
    if (!infile) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string data, errorBits;
    while (infile >> data >> errorBits) {
        cout << "==========================================================================================" << endl;
        cout << "Data: " << data << endl;
        cout << "Error: " << errorBits << endl;
        printResult(data, errorBits);
    }
    infile.close();
}

// Main function
int main() {
    // File containing the data and error bits
    string filename = "./data.txt"; // Adjust file path if necessary
    processFile(filename);

    return 0;
}
