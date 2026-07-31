#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::string int_to_hex(int num);
std::string int_to_binary(int num);
bool valid_arg(char* argument, std::string& arg, std::string& value);
int bin_to_hex(int argc, char* argv[]);
int hex_to_asm(int argc, char* argv[]);

/**
 * @brief The main function :) We start executing code here.
 *
 * @param argc The number of command-line arguments provided when running the executable.
 * @param argv Command-line arguments using C-style strings which pertain to certain
 * properties of changable variables the user may  want control over (e.g., input-file, output-file, etc.)
 * @return int The return code given from running the program (e.g., 0 for clear, 1 for error).
 */
int main(int argc, char* argv[]) {
    // Check to see if too many arguments were given
    // if (argc > 6) {
    //     std::cerr << "Error: Too many arguments. Usage: " << argv[0] << " <your argument>\n";
    //     return 1;
    // }
    std::string command = "";
    int j = 0;
    do {
        command.push_back(argv[1][j]);
        j++;
    } while (argv[1][j] != '\0');

    if ("bin-to-hex" == command) {
        bin_to_hex(argc - 2, argv + 2);
    } else if ("" == command) {
        hex_to_asm(argc - 2, argv + 2);
    }

    return 0;
}

/**
 * @brief Takes hexadecimal and translates it into MIPS assembly instructions
 *
 * This function takes hexadecimal input from a .txt file, in the format of the
 * bin-to-hex function and outputs MIPS assembly instructions in a new .txt file.
 *
 * @param argc The number of arguments held inside argv.
 * @param argv The arguments that could be used to modify the input file or output
 * file of this function.
 * @return int The return code given to the main function (e.g., 0 for clear, 1 for error).
 */
int hex_to_asm(int argc, char* argv[]) {
    // Properties
    std::string valid_in_file = "";
    std::string valid_out_file = "";

    // Configures which propertries to change
    for (int i = 0; i < argc; i++) {
        // std::cout << argv[i] << "\n";
        std::string command_value;
        std::string arg_key;
        if (valid_arg(argv[i], arg_key, command_value)) {
            // std::cout << arg_key << ' ' << argc << "\n";
            if (arg_key == "input-file") {
                valid_in_file = command_value;
            } else if (arg_key == "output-file") {
                valid_out_file = command_value;
            }
        }
    }

    // std::cout << valid_in_file << " " << valid_out_file << " " << valid_bpr << " " << valid_interpreter << "\n";

    // Creates variables holding changed values for properties
    std::string hexfile;
    std::string outfile;
    if (valid_in_file != "") {
        hexfile = valid_in_file;
    } else {
        hexfile = "SLUS_custom.txt";
    }  // File to read the hex data from
    if (valid_out_file != "") {
        outfile = valid_out_file;
    } else {
        outfile = "SLUS_205_asm.txt";
    }  // File to write the asm data to

    std::fstream readfile{hexfile, std::ios::ate};  // Start at the end of the file

    // If can't open file then exit
    if (!readfile.is_open()) {
        std::cerr << "Uh oh, " << hexfile << " could not be opened for reading!\n";
        return 1;
    }

    std::streamsize size = readfile.tellg();

    // Move the read cursor to the beginning of the file
    readfile.seekg(0, std::fstream::cur);

    return 0;
}

/**
 * @brief Translates an integer value into its corresponding hexadecimal value
 *
 * @param num The integer to translate into hexadecimal format (0-9, A-F).
 * @return std::string A string containing two hexadecimal characters, which match an integer 0-255.
 */
std::string int_to_hex(int num) {
    std::string result = "";
    int num1 = num / 16;
    int num2 = num % 16;

    // std::cout << num1 << ' ' << num2 << '\n';

    if (num1 < 10) {
        result.push_back(num1 + '0');
    } else {
        switch (num1) {
            case 10: {
                result.push_back('A');
                break;
            }
            case 11: {
                result.push_back('B');
                break;
            }
            case 12: {
                result.push_back('C');
                break;
            }
            case 13: {
                result.push_back('D');
                break;
            }
            case 14: {
                result.push_back('E');
                break;
            }
            case 15: {
                result.push_back('F');
                break;
            }
        }
    }

    if (num2 < 10) {
        result.push_back(num2 + '0');
    } else {
        switch (num2) {
            case 10: {
                result.push_back('A');
                break;
            }
            case 11: {
                result.push_back('B');
                break;
            }
            case 12: {
                result.push_back('C');
                break;
            }
            case 13: {
                result.push_back('D');
                break;
            }
            case 14: {
                result.push_back('E');
                break;
            }
            case 15: {
                result.push_back('F');
                break;
            }
        }
    }

    return result;
}

/**
 * @brief Translates an integer value into its corresponding binary value
 *
 * @param num The integer to translate into binary format (0-1).
 * @return std::string A string containing eight binary characters, which match an integer 0-255.
 */
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

/**
 * @brief Checks to see if a given command-line argument is valid
 *
 * @param argument A C-style string containing the argument to check the validity of.
 * @param arg Set to the key that corresponds to a certain property for the bin-to-hex
 * function. Used to check against known keys for determining if argument is valid.
 * @param value Set to the value that a given property should be changed to. Used by
 * caller function.
 * @return true The given argument was valid, proceed with changing propertie(s) for bin-to-hex.
 * @return false The given argument was NOT valid, DO NOT change propertie(s) for bin-to-hex.
 */
bool valid_arg(char* argument, std::string& arg, std::string& value) {
    int i = 2;
    do {
        arg.push_back(argument[i]);
        i++;
    } while (argument[i] != '=' && argument[i] != '\0');

    std::vector<std::string> possible_args{"input-file", "output-file", "bytes-per-row", "binary-output",
                                           "format-output", ""};

    // Tries to find the current argument in the list of possible_args
    if (std::find(possible_args.begin(), possible_args.end(), arg) !=
                                           possible_args.end()) {
        i++;
        do {
            value.push_back(argument[i]);
            i++;
        } while (argument[i] != '\0');
        return 1;
    } 

    return 0;
}

/**
 * @brief Translates pre-compiled binary (normally unreadable unless using a hex editor) into hexadecimal.
 *
 * @param argc The number of C-style strings provided by argv.
 * @param argv Contains different properties that may be changed within the function
 * (e.g., input-file, output-file, bytes-per-row, binary-output).
 * @return int The return code given to the main function (e.g., 0 for clear, 1 for error).
 */
int bin_to_hex(int argc, char* argv[]) {
    std::string valid_in_file = "";
    std::string valid_out_file = "";
    int valid_bpr = 0;
    bool valid_interpreter = false;
    bool valid_format = false;

    for (int i = 0; i < argc; i++) {
        // std::cout << argv[i] << "\n";
        std::string command_value;
        std::string arg_key;
        if (valid_arg(argv[i], arg_key, command_value)) {
            // std::cout << arg_key << ' ' << argc << "\n";

            if (arg_key == "input-file") {
                valid_in_file = command_value;
            } else if (arg_key == "output-file") {
                valid_out_file = command_value;
            } else if (arg_key == "binary-output") {
                valid_interpreter = std::stoi(command_value);  // Should be 1 for true, 0 for false
            } else if (arg_key == "bytes-per-row") {
                valid_bpr = std::stoi(command_value);
            } else if (arg_key == "format-output") {  // Should be just a key tag without a value
                valid_format = true;
            }
        }
    }

    // std::cout << valid_in_file << " " << valid_out_file << " " << valid_bpr << " " << valid_interpreter << "\n";

    std::string hexfile;
    std::string outfile;
    int bytes_per_row;
    bool binary_file;
    bool format_output;
    if (valid_in_file != "") {
        hexfile = valid_in_file;
    } else {
        hexfile = "SLUS_205.91";
    }  // File to read the hex data from
    if (valid_out_file != "") {
        outfile = valid_out_file;
    } else {
        outfile = "SLUS_custom.txt";
    }  // File to write the hex data to
    if (valid_bpr != 0) {
        bytes_per_row = valid_bpr;
    } else {
        bytes_per_row = 32;
    }
    if (!valid_interpreter) {
        binary_file = false;
    } else {
        binary_file = true;
    }
    if (!valid_format) {
        format_output = false;
    } else {
        format_output = true;
    }

    // Access point to read file as binary, starting at the end of the file
    std::fstream readfile{hexfile, std::ios::in | std::ios::binary | std::ios::ate};
    // Check for open error
    if (!readfile.is_open()) {
        std::cerr << "Failed to open the file: " << hexfile << ".\n";
        return 1;
    }

    std::streamsize insize = readfile.tellg();  // Tells the total length of the file we're reading
    readfile.seekg(0);                          // Moves the read file pointer to begin reading the file

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
    if (!writefile) {
        std::cerr << "Uh oh, " << outfile << " could not be opened for writing!\n";
        return 1;
    }

    for (int i = 0; i <= insize; i++) {
        if (i % bytes_per_row == 0 && i != 0 && format_output) {
            writefile << "\n";
        }
        if (binary_file) {
            writefile << int_to_binary(int(uint8_t(buffer[i])));
        } else {
            writefile << int_to_hex(int(uint8_t(buffer[i])));
        }
        if (format_output) {
            writefile << ' ';
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
