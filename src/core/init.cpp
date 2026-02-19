//init.cpp
#include <iostream>
#include "../shell_state.hpp"
#include "../utils/terminal.hpp"

void initialization(ShellState& state) {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
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
}