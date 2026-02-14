#include <iostream>
#include <string>

int main(){
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    while (true){
        std::cout << "$ ";
        std::string command;
        std::getline(std::cin, command);
        std::cout << command << ": command not found" << "\n";
    }
}