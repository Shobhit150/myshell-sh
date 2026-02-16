#include<vector>
#include<iostream>

std::vector<std::string> tokenizer(std::string input) {
    std::vector<std::string> tokens;
    std::string word = "";
    bool singleQuote = false;
    bool doubleQuote = false;
    for(char &c: input) {
        if(c=='"' && !singleQuote) {
            doubleQuote = !doubleQuote;
            continue;
        }
        if(c=='\'' && !doubleQuote) {
            singleQuote = !singleQuote;
            continue;
        }
        if(c == ' ' && !doubleQuote && !singleQuote) {
            if (!word.empty()) {
                tokens.push_back(word);
                word.clear();
            }
        } else {
            word.push_back(c);
        }
    }
    if(!word.empty()) {
        tokens.push_back(word);
    }
    return tokens;
}