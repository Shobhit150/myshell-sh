#include <iostream>
#include <string>
#include "services/dataHandling.hpp"

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // TODO: Uncomment the code below to pass the first stage
    while (true) {
        std::cout << "$ ";
        std::string input;
        std::getline(std::cin, input);
        std::vector<std::string> tokens = tokenizer(input);
        std::string command = tokens[0];
        if (command == "exit") {
            break;
        }else if(command == "echo") {
            for(int i=1;i<tokens.size();i++) {
                std::cout << tokens[i] << " ";
            }
            std::cout << "\n";
        } else {
            std::cout << input << ": command not found" << "\n";
        }
    }
}
