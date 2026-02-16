#include<vector>
#include<iostream>
#include "../shell_state.hpp"
#include <unistd.h>
#include <sys/wait.h>

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
        std::string dir = str.empty() ? "." : str;
        std::string fullPath = dir + "/" + cmd;

        if (access(fullPath.c_str(), X_OK) == 0) {
            std::cout << cmd << " is " << fullPath << "\n";
            return;
        }
    }
    std::cout << cmd << ": not found\n";
}

void searchPath(std::vector<std::string>& tokens, ShellState &state) {
    std::string cmd = tokens[0];
    std::string foundPath = "";

    std::string path = tokens[0];
    for(std::string &str: state.pathDirs) {
        std::string dir = str.empty() ? "." : str;
        std::string fullPath = dir + "/" + cmd;

        if(access(fullPath.c_str(), X_OK) == 0) {
            foundPath = fullPath;
            break;
        }
    }

    if(foundPath.empty()) {
        std::cout << cmd << ": not found\n";
        return;
    }

    std::vector<char*> args;

    for(std::string &token: tokens) {
        args.push_back(const_cast<char*>(token.c_str()));
    }
    args.push_back(nullptr);

    pid_t pid = fork();

    if(pid == 0) {
        execvp(foundPath.c_str(), args.data());
        std::cout << "execution failed\n";
        exit(1);
    } else if(pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        std::cerr << "fork failed\n";
    }
}