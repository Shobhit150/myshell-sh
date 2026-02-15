#include <iostream>
#include <string>
#include "services/dataCommand.hpp"
#include "utils/tokenizer.hpp"
#include "shell_state.hpp"
#include <cstdlib>

int main() {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    ShellState state;


    char* envPath = std::getenv("PATH");
    if(envPath) {
        std::string pathStr(envPath);
        std::string temp = "";
        for(int i=0;i<pathStr.size();i++) {
            if(pathStr[i] == ':') {
                state.pathDirs.push_back(temp);
                temp = "";
            } else {
                temp.push_back(pathStr[i]);
            }
        }
        if(!temp.empty()) {
            state.pathDirs.push_back(temp);
        }
    }

    while (true) {
        std::cout << "$ ";
        std::string input;
        std::getline(std::cin, input);
        std::vector<std::string> tokens = tokenizer(input);
        std::string command = tokens[0];
        if (command == "exit") {
            break;
        }else if(command == "echo") {
            handleEcho(tokens);
        } else if(command == "type") {
            handleType(tokens, state);
        } else {
            std::cout << input << ": command not found" << "\n";
        }
    }
}
