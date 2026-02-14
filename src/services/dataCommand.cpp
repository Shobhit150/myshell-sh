#include<vector>
#include<iostream>

void handleEcho(std::vector<std::string> tokens) {
    for(int i=1;i<tokens.size();i++) {
        std::cout << tokens[i] << " ";
    }
    std::cout << "\n";
}

void handleType(std::vector<std::string> tokens) {
    if(tokens[1] == "echo") {
        std::cout << "echo is a shell builtin\n";
    } else if(tokens[1] == "exit") {
        std::cout << "exit is a shell builtin\n";
    } else if(tokens[1] == "type") {
        std::cout << "type is a shell builtin\n";
    } else {
        std::cout << tokens[1] << ": not found";
    }
}