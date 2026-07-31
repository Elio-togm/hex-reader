#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>

std::string int_to_hex(int num);
std::string int_to_binary(int num);

int main()
{
    
    std::string hexfile = "SLUS_205.91";     // File to read the hex data from
    std::string outfile = "SLUS_custom.txt"; // File to write the hex data to
    int bytes_per_row = 32;
    bool binary_file = false;

    // Access point to read file as binary, starting at the end of the file
    std::fstream readfile{hexfile, std::ios::in | std::ios::binary | std::ios::ate}; 
    // Check for open error
    if (!readfile.is_open())
    {
        std::cerr << "Failed to open the file: " << hexfile << ".\n";
        return 1;
    }

    std::streamsize insize = readfile.tellg(); // Tells the total length of the file we're reading
    readfile.seekg(0); // Moves the read file pointer to begin reading the file
    

    // Create a buffer the same size as the file
    std::vector<char> buffer(insize);

    // Read the bulk stream into the vector memory block
    if (readfile.read(buffer.data(), insize)) {
        std::cout << "Successfully loaded " << insize << " bytes into memory.\n";
    }
    readfile.close();

    // Access point to write to file, which will be in hex (0-9, A-F)
    std::ofstream writefile{outfile, std::ios::out};
    // Check for open error
    if (!writefile)
    {
        std::cerr << "Uh oh, " << outfile << " could not be opened for writing!\n";
        return 1;
    }

    for (int i = 1; i <= insize; i++) {
        if (i % bytes_per_row == 0) {
            writefile << "\n";
        }
        if (binary_file) {
            writefile << int_to_binary(int(uint8_t(buffer[i]))) << ' ';
        } else {
            writefile << int_to_hex(int(uint8_t(buffer[i]))) << ' ';
        }
    }
    writefile.close();


    if (binary_file) {
        std::cout << "Binary Writing Completed Successfully!\n";    
    } else {
        std::cout << "Hex Writing Completed Successfully!\n";
    }

    return 0;
}

std::string int_to_hex(int num) {
    std::string result = "";
    int num1 = num / 16;
    int num2 = num % 16;

    // std::cout << num1 << ' ' << num2 << '\n'; 

    if (num1 < 10) {
        result.push_back(num1 + '0');
    }
    else {
        switch (num1) {
            case 10: {result.push_back('A'); break;}
            case 11: {result.push_back('B'); break;}
            case 12: {result.push_back('C'); break;}
            case 13: {result.push_back('D'); break;}
            case 14: {result.push_back('E'); break;}
            case 15: {result.push_back('F'); break;}
        }
    }

    if (num2 < 10) {
        result.push_back(num2 + '0');
    }
    else {
        switch (num2) {
            case 10: {result.push_back('A'); break;}
            case 11: {result.push_back('B'); break;}
            case 12: {result.push_back('C'); break;}
            case 13: {result.push_back('D'); break;}
            case 14: {result.push_back('E'); break;}
            case 15: {result.push_back('F'); break;}
        }
    }

    return result;
}

std::string int_to_binary(int num) {
    std::string result = "";
    int num1 = num % 2;
    int num2 = (num / 2) % 2;
    int num3 = (num / 4) % 2;
    int num4 = (num / 8) % 2;
    int num5 = (num / 16) % 2;
    int num6 = (num / 32) % 2;
    int num7 = (num / 64) % 2;
    int num8 = (num / 128) % 2;

    // std::cout << num1 << ' ' << num2 << ' ' << num3 << ' ' << num4 << ' ' << num5;
    // std::cout << ' ' << num6 << ' ' << num7 << ' ' << num8 << '\n'; 

    result.push_back(num8 + '0');
    result.push_back(num7 + '0');
    result.push_back(num6 + '0');
    result.push_back(num5 + '0');
    result.push_back(num4 + '0');
    result.push_back(num3 + '0');
    result.push_back(num2 + '0');
    result.push_back(num1 + '0');

    return result;
}

