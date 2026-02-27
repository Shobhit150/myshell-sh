#include <vector>
#include <iostream>
#include "../shell_state.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "dataCommand.hpp"

void runCommandInChild(std::vector<char*>& args, ShellState& state) {
    std::string cmd = args[0];

    if (cmd == "echo") {
        std::vector<std::string> t;
        for (char* a : args) if (a) t.push_back(a);
        handleEcho(t);
        _exit(0);
    }

    if (cmd == "type") {
        std::vector<std::string> t;
        for (char* a : args) if (a) t.push_back(a);
        handleType(t, state);
        _exit(0);
    }

    if (cmd == "pwd") {
        handlePwd();
        _exit(0);
    }

    if (cmd == "cd") {
        std::vector<std::string> t;
        for (char* a : args) if (a) t.push_back(a);
        handleCd(t);
        _exit(0);
    }

    if (cmd == "exit") {
        _exit(0);
    }

    execvp(args[0], args.data());
    perror("execvp");
    _exit(1);
}

RedirectInfo extractRedirect(std::vector<std::string> &tokens) {
    RedirectInfo info;
    for (size_t i = 0; i + 1 < tokens.size();)
    {
        if (tokens[i] == ">" || tokens[i] == "1>")
        {
            info.stdoutFile = tokens[i + 1];
            tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
        }
        else if (tokens[i] == ">>" || tokens[i] == "1>>")
        {
            info.stdoutFile = tokens[i + 1];
            info.stdoutAppend = true;
            tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
        }
        else if (tokens[i] == "2>")
        {
            info.stderrFile = tokens[i + 1];
            tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
        }
        else if (tokens[i] == "2>>")
        {
            info.stderrFile = tokens[i + 1];
            info.stderrAppend = true;
            tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
        }
        else
        {
            i++;
        }
    }
    return info;
}

SavedFDs applyRedirect(RedirectInfo &r) {
    SavedFDs s;
    if (!r.stdoutFile.empty())
    {
        s.out = dup(STDOUT_FILENO);
        int flags = O_WRONLY | O_CREAT |
                    (r.stdoutAppend ? O_APPEND : O_TRUNC);
        int fd = open(r.stdoutFile.c_str(), flags, 0644);
        if (fd < 0)
        {
            perror("open");
            return s;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    if (!r.stderrFile.empty())
    {
        s.err = dup(STDERR_FILENO);
        int flags = O_WRONLY | O_CREAT |
                    (r.stderrAppend ? O_APPEND : O_TRUNC);
        int fd = open(r.stderrFile.c_str(), flags, 0644);
        if (fd < 0)
        {
            perror("open");
            return s;
        }
        dup2(fd, STDERR_FILENO);
        close(fd);
    }
    return s;
}

void restoreFDs(const SavedFDs &s) {
    if (s.out != -1)
    {
        dup2(s.out, STDOUT_FILENO);
        close(s.out);
    }

    if (s.err != -1)
    {
        dup2(s.err, STDERR_FILENO);
        close(s.err);
    }
}

void handleEcho(std::vector<std::string> &tokens) {
    for (size_t i = 1; i < tokens.size(); ++i)
    {
        if (i > 1)
            std::cout << " ";
        std::cout << tokens[i];
    }
    std::cout << "\n";
}

void handleType(std::vector<std::string> &tokens, ShellState &state) {
    if (tokens.size() < 2)
        return;
    std::string cmd = tokens[1];
    if (cmd == "echo" || cmd == "exit" || cmd == "type" || cmd == "pwd")
    {
        std::cout << cmd << " is a shell builtin\n";
        return;
    }

    for (std::string &str : state.pathDirs)
    {
        std::string dir = str.empty() ? "." : str;
        std::string fullPath = dir + "/" + cmd;

        if (access(fullPath.c_str(), X_OK) == 0)
        {
            std::cout << cmd << " is " << fullPath << "\n";
            return;
        }
    }
    std::cout << cmd << ": not found\n";
}

void searchPath(std::vector<std::string> &tokens, ShellState &state) {

    std::string cmd = tokens[0];
    std::string foundPath = "";

    for (std::string &str : state.pathDirs)
    {
        std::string dir = str.empty() ? "." : str;
        std::string fullPath = dir + "/" + cmd;

        if (access(fullPath.c_str(), X_OK) == 0)
        {
            foundPath = fullPath;
            break;
        }
    }

    if (foundPath.empty())
    {
        std::cerr << cmd << ": not found\n";
        return;
    }

    std::vector<char *> args;

    for (std::string &token : tokens)
    {
        args.push_back(const_cast<char *>(token.c_str()));
    }
    args.push_back(nullptr);

    pid_t pid = fork();

    if (pid == 0)
    {
        execvp(foundPath.c_str(), args.data());
        perror("execvp");
        _exit(1);
    }
    else if (pid > 0)
    {
        int status;
        waitpid(pid, &status, 0);
    }
    else
    {
        perror("fork");
    }
}

void handlePwd() {
    const size_t size = 1024;
    char buffer[size];
    if (getcwd(buffer, size) != NULL)
    {
        std::cout << buffer << "\n";
    }
    else
    {
        std::cerr << "Error getting current working directory\n";
    }
}

void handleCd(std::vector<std::string> &tokens) {
    if (tokens.size() < 2)
    {
        std::cerr << "cd: missing operand\n";
        return;
    }
    const char *path;

    std::string str = tokens[1];

    if (str == "~")
    {
        path = getenv("HOME");
        if (!path)
        {
            std::cerr << "cd: HOME not set\n";
            return;
        }
    }
    else
    {
        path = str.c_str();
    }
    if (chdir(path) != 0)
    {
        std::cerr << "cd: " << str << ": No such file or directory\n";
    }
}

void handlePipe(std::vector<std::string>& tokens, int i, ShellState& state) {
    std::string cmd1 = tokens[0];
    std::string cmd2 = tokens[i + 1];
    std::vector<char *> args1;
    std::vector<char *> args2;
    for (int j = 0; j < i; j++)
    {
        args1.push_back(const_cast<char *>(tokens[j].c_str()));
    }
    for (int j = i + 1; j < tokens.size(); j++)
    {
        args2.push_back(const_cast<char *>(tokens[j].c_str()));
    }

    args1.push_back(nullptr);
    args2.push_back(nullptr);

    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("pipe");
        return;
    }

    pid_t pid1 = fork();

    if (pid1 == 0)
    {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        runCommandInChild(args1, state);
        perror("execvp cmd1");
        _exit(1);
    }
    pid_t pid2 = fork();
    if (pid2 == 0)
    {
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        runCommandInChild(args2, state);
        perror("execvp cmd2");
        _exit(1);
    }
    close(fd[0]);
    close(fd[1]);

    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);
}