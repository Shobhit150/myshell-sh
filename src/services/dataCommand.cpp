#include<vector>
#include<iostream>
#include "../shell_state.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "dataCommand.hpp"

RedirectInfo extractRedirect(std::vector<std::string>& tokens) {
    RedirectInfo info;
    for(size_t i = 0; i + 1 < tokens.size(); ) {
        if(tokens[i] == ">" || tokens[i] == "1>") {
            info.stdoutFile = tokens[i+1];
            tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
        } else if(tokens[i] == ">>" || tokens[i] == "1>>") {
            info.stdoutFile = tokens[i+1];
            info.stdoutAppend = true;
            tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
        } else if(tokens[i] == "2>") {
            info.stderrFile = tokens[i+1];
            tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
        } else if (tokens[i] == "2>>") {
            info.stderrFile = tokens[i + 1];
            info.stderrAppend = true;
            tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
        } else {
            i++;
        }
    }
    return info;
}


SavedFDs applyRedirect(RedirectInfo &r) {
    SavedFDs s;
    if(!r.stdoutFile.empty()) {
        s.out = dup(STDOUT_FILENO);
        int flags = O_WRONLY | O_CREAT |
            (r.stdoutAppend ? O_APPEND : O_TRUNC);
        int fd = open(r.stdoutFile.c_str(), flags, 0644);
        if (fd < 0) {
            perror("open");
            return s;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    if(!r.stderrFile.empty()) {
        s.err = dup(STDERR_FILENO);
        int flags = O_WRONLY | O_CREAT |
            (r.stderrAppend ? O_APPEND : O_TRUNC);
        int fd = open(r.stderrFile.c_str(), flags, 0644);
        if (fd < 0) {
            perror("open");
            return s;
        }
        dup2(fd, STDERR_FILENO);
        close(fd);
    }
    return s;
}

void restoreFDs(const SavedFDs& s) {
    if (s.out != -1) {
        dup2(s.out, STDOUT_FILENO);
        close(s.out);
    }

    if (s.err != -1) {
        dup2(s.err, STDERR_FILENO);
        close(s.err);
    }
}

void handleEcho(std::vector<std::string>& tokens) {
    for (size_t i = 1; i < tokens.size(); ++i) {
        if (i > 1) std::cout << " ";
        std::cout << tokens[i];
    }
    std::cout << "\n";
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

    std::string cmd = tokens[0];
    std::string foundPath = "";

    for(std::string &str: state.pathDirs) {
        std::string dir = str.empty() ? "." : str;
        std::string fullPath = dir + "/" + cmd;

        if(access(fullPath.c_str(), X_OK) == 0) {
            foundPath = fullPath;
            break;
        }
    }

    if(foundPath.empty()) {
        std::cerr << cmd << ": not found\n";
        return;
    }

    std::vector<char*> args;

    for(std::string &token: tokens) {
        args.push_back(const_cast<char*>(token.c_str()));
    }
    args.push_back(nullptr);

    pid_t pid = fork();

    if (pid == 0) {
        execvp(foundPath.c_str(), args.data());
        perror("execvp");
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork");
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
        std::cerr << "cd: " << str << ": No such file or directory\n";
    }
}