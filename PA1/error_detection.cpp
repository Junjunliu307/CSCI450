#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <vector>

using namespace std;

// Define the CRC divisor as a binary string (Example: CRC-16 polynomial)
const string crc_divisor = "10000010000010001";  // This is equivalent to x^16 + x^10 + x^4 + 1

// Function to perform XOR on two binary strings
string xorOperation(const string& dividend, const string& divisor) {
    string result = "";
    for (size_t i = 1; i < divisor.length(); ++i) {
        if (dividend[i] == divisor[i]) {
            result += '0';
        } else {
            result += '1';
        }
    }
    return result;
}

// Function to calculate CRC using bitwise division
string calculateCRC(const string& data, const string& divisor) {
    // Append zeros to the data based on the divisor's length
    string extendedData = data + string(divisor.length() - 1, '0');
    
    size_t pos = divisor.length();
    string dividend = extendedData.substr(0, divisor.length());
    
    while (pos < extendedData.length()) {
        if (dividend[0] == '1') {
            dividend = xorOperation(dividend, divisor) + extendedData[pos];
        } else {
            dividend = dividend.substr(1) + extendedData[pos];
        }
        pos++;
    }
    
    // Handle the remaining bits
    while (dividend.length() >= divisor.length()) {
        if (dividend[0] == '1') {
            dividend = xorOperation(dividend, divisor);
        } else {
            dividend = dividend.substr(1);
        }
    }
    
    return dividend;
}

// Function to convert binary string to an integer
unsigned long binaryStringToInt(const string& str) {
    unsigned int value = 0;
    for (size_t i = 0; i < str.length(); ++i) {
        value = (value << 1) + (str[i] - '0'); // Convert binary string to integer bit by bit
    }
    return value;
}

// Function to calculate 16-bit checksum
string calculateChecksum(const string& data) {
    unsigned long sum_value = 0;
    
    // Process data in 16-bit blocks
    for (size_t i = 0; i < data.length(); i += 16) {
        string block = data.substr(i, 16);  // Extract 16-bit block
        unsigned long value = binaryStringToInt(block);  // Convert to integer
        sum_value += value;
        
        // Handle overflow (carry over)
        sum_value = (sum_value >> 16) + (sum_value & 0xFFFF);
    }
    
    // Take one's complement
    unsigned long checksum = ~sum_value & 0xFFFF;
    return bitset<16>(checksum).to_string();  // Return 16-bit binary string
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
    string crc = calculateCRC(data, crc_divisor);
    string checksum = calculateChecksum(data);
    string codeword_crc = data + crc;
    string codeword_checksum = data + checksum;

    // Introduce errors
    string received_crc = introduceErrors(codeword_crc, errorBits);
    string received_checksum = introduceErrors(codeword_checksum, errorBits);

    // Recalculate CRC and checksum for the received data
    string recalculated_crc = calculateCRC(received_crc.substr(0, data.length()), crc_divisor);
    string recalculated_checksum = calculateChecksum(received_checksum.substr(0, data.length()));

    // Print results for CRC
    cout << "CRC-16" << endl;
    cout << "CRC: " << crc << "; \t\t\tResult: " 
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
        cout << "=================================" << endl;
        cout << "Data:" << data << endl;
        cout << "Error:" << errorBits << endl;
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
