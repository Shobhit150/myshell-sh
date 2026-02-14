#include <iostream>
#include <string>
#include "services/dataCommand.hpp"
#include "utils/tokenizer.hpp"
#include "shell_state.hpp"

int main() {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    ShellState state;

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
            handleType(tokens);
        } else if(command.substr(0,4) == "PATH") {
            handlePath(tokens);
        } else {
            std::cout << input << ": command not found" << "\n";
        }
    }
}
