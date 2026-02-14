#include<vector>
#include<iostream>

std::vector<std::string> tokenizer(std::string input) {
    std::vector<std::string> tokens;
    std::string word = "";
    bool quoteOpen = false;
    for(char &c: input) {
        if(c=='"') {
            quoteOpen = !quoteOpen;
            continue;
        }
        if(c == ' ' && !quoteOpen) {
            tokens.push_back(word);
            word.clear();
        } else {
            word.push_back(c);
        }
    }
    if(!word.empty()) {
        tokens.push_back(word);
    }
    return tokens;
}