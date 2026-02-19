// main.cpp
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include <string>
#include "services/dataCommand.hpp"
#include "utils/tokenizer.hpp"
#include "shell_state.hpp"
#include <cstdlib>
#include "utils/terminal.hpp"
#include "core/init.hpp"

int main() {
    buildTree();

    ShellState state;
    initialization(state);

    while (true) {
        std::string input = readLineRaw("$ ");
        std::vector<std::string> tokens = tokenizer(input);
        if (tokens.empty()) continue;

        auto redirect = extractRedirect(tokens);

        SavedFDs saved = applyRedirect(redirect);

        std::string command = tokens[0];
        if (command == "exit") {
            recordUsage(command);
            restoreFDs(saved);
            break;
        }else if(command == "echo") {
            handleEcho(tokens);
            recordUsage(command);
        } else if(command == "type") {
            handleType(tokens, state);
        } else if(command == "pwd") {
            handlePwd();
        } else if(command == "cd") {
            handleCd(tokens);
            
        } else {
            searchPath(tokens, state);
        }

        restoreFDs(saved);
    }
}
