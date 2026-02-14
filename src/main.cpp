#include <iostream>
#include <string>

int main(){
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::cout << "$ ";
    std::string command;
    std::getline(std::cin, command);
}
