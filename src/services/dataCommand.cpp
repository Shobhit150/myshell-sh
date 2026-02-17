#include<vector>
#include<iostream>
#include "../shell_state.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct RedirectInfo {
    std::string stdoutFile;
    std::string stderrFile;
    bool stdoutAppend = false;
    bool stderrAppend = false;
};

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

void handleEcho(std::vector<std::string>& tokens) {
    auto redirect = extractRedirect(tokens);

    int savedStdout = -1;
    int savedStderr = -1;

    if(!redirect.stdoutFile.empty()) {
        savedStdout = dup(STDOUT_FILENO);
        int flags = O_WRONLY | O_CREAT |
            (redirect.stdoutAppend ? O_APPEND : O_TRUNC);
        int fd = open(redirect.stdoutFile.c_str(), flags, 0644);
        if (fd < 0) {
                perror("open");
                return;
            }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    if(!redirect.stderrFile.empty()) {
        savedStderr = dup(STDERR_FILENO);
        int flags = O_WRONLY | O_CREAT |
            (redirect.stderrAppend ? O_APPEND : O_TRUNC);
        int fd = open(redirect.stderrFile.c_str(), flags, 0644);
        if (fd < 0) {
            perror("open");
            return;
        }
        dup2(fd, STDERR_FILENO);
        close(fd);
    }


    for (size_t i = 1; i < tokens.size(); ++i) {
        if (i > 1) std::cout << " ";
        std::cout << tokens[i];
    }
    std::cout << "\n";
    std::cout.flush();

    if(savedStdout != -1) {
        dup2(savedStdout, STDOUT_FILENO);
        close(savedStdout);
    }
    if(savedStderr != -1) {
        dup2(savedStderr, STDERR_FILENO);
        close(savedStderr);
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
    auto redirect = extractRedirect(tokens);
    int savedStdout = -1;
    int savedStderr = -1;

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
        std::cerr << cmd << ": not found\n";
        return;
    }

    std::vector<char*> args;

    for(std::string &token: tokens) {
        args.push_back(const_cast<char*>(token.c_str()));
    }
    args.push_back(nullptr);

    pid_t pid = fork();

    if(pid == 0) {
        if(!redirect.stdoutFile.empty()) {
            savedStdout = dup(STDOUT_FILENO);
            int flags = O_WRONLY | O_CREAT |
                (redirect.stdoutAppend ? O_APPEND : O_TRUNC);
            int fd = open(redirect.stdoutFile.c_str(), flags, 0644);
            if (fd < 0) {
                perror("open");
                return;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        if(!redirect.stderrFile.empty()) {
            savedStderr = dup(STDERR_FILENO);
            int flags = O_WRONLY | O_CREAT |
                (redirect.stderrAppend ? O_APPEND : O_TRUNC);
            int fd = open(redirect.stderrFile.c_str(), flags, 0644);
            if (fd < 0) {
                perror("open");
                return;
            }

            dup2(fd, STDERR_FILENO);
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
        std::cerr << "cd: " << str << ": No such file or directory\n";
    }
}