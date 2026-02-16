#include<vector>
#include<iostream>
#include "../shell_state.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

std::string extratedirect(std::vector<std::string>& tokens) {
    for(size_t i=0;i+1<tokens.size();i++) {
        if(tokens[i] == ">" || tokens[i] == "1>") {
            std::string file = tokens[i+1];
            tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
            return file;            
        }
    }
    return "";
}

void handleEcho(std::vector<std::string>& tokens) {
    std::string redirectedFile = extratedirect(tokens);

    int savedFd = -1;

    if(!redirectedFile.empty()) {
        savedFd = dup(STDOUT_FILENO);
        int fd = open(redirectedFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if(fd < 0) {
            std::cerr << "echo: " << redirectedFile << ": cannot open file\n";
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    for(int i=1;i<tokens.size();i++) {
        std::cout << tokens[i] << " ";
    }
    std::cout << "\n";
    std::cout.flush();

    if(savedFd != -1) {
        dup2(savedFd, STDOUT_FILENO);
        close(savedFd);
    }
}

void handleType(std::vector<std::string>& tokens, ShellState &state) {
    if (tokens.size() < 2) return;
    std::string cmd = tokens[1];
    if(cmd == "echo" || cmd == "exit" || cmd == "type" || cmd == "pwd") {
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
    std::string redirectedFile = extratedirect(tokens);

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
        if(!redirectedFile.empty()) {
            int fd = open(redirectedFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(fd < 0) {
                std::cerr << "echo: " << redirectedFile << ": cannot open file\n";
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        execvp(foundPath.c_str(), args.data());
        std::cout << "execution failed\n";
        _exit(1);
    } else if(pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        std::cerr << "fork failed\n";
    }
}

void handlePwd() {
    const size_t size = 1024;
    char buffer[size];
    if(getcwd(buffer, size) != NULL) {
        std::cout << buffer << "\n";
    } else {
        std::cerr << "Error getting current working directory\n";
    }
}

void handleCd(std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        std::cerr << "cd: missing operand\n";
        return;
    }
    const char* path;

    std::string str = tokens[1];

    if (str == "~") {
        path = getenv("HOME");
        if (!path) {
            std::cerr << "cd: HOME not set\n";
            return;
        }
    } else {
        path = str.c_str();
    }
    if (chdir(path) != 0) {
        std::cout << "cd: " << str << ": No such file or directory\n";
    }
}