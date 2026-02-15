#include<vector>
#include<iostream>
#include "../shell_state.hpp"
#include <unistd.h>

void handleEcho(std::vector<std::string>& tokens) {
    for(int i=1;i<tokens.size();i++) {
        std::cout << tokens[i] << " ";
    }
    std::cout << "\n";
}

void handleType(std::vector<std::string>& tokens, ShellState &state) {
    if (tokens.size() < 2) return;
    std::string cmd = tokens[1];
    if(cmd == "echo" || cmd == "exit" || cmd == "type") {
        std::cout << cmd << " is a shell builtin\n";
        return;
    }

    for(std::string &str: state.pathDirs) {
        std::string fullPath = str + "/" + cmd;

        if (access(fullPath.c_str(), X_OK) == 0) {
            std::cout << cmd << " is " << fullPath << "\n";
            return;
        }
    }
    std::cout << cmd << ": not found\n";
}