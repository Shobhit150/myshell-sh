#include<vector>
#include<iostream>

std::vector<std::string> tokenizer(std::string input) {
    std::vector<std::string> tokens;
    std::string word = "";
    bool singleQuote = false;
    bool doubleQuote = false;
    bool escape = false;

    for(int i=0;i<input.size();i++) {
        char c = input[i];

        if(escape) {
            word.push_back(c);
            escape = false;
            continue;
        }

        if (c == '\\' && !singleQuote && !doubleQuote) {
            escape = true;
            continue;
        }

        if(doubleQuote && c == '\\') {
            if(i+1 < input.size()) {
                char next = input[i+1];

                if(next == '"' || next == '\\') {
                    word.push_back(next);
                    i++;
                    continue;
                }
            }

            word.push_back('\\');
            continue;
        }

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

    if (escape) {
        word.push_back('\\');
    }
    
    if(!word.empty()) {
        tokens.push_back(word);
    }
    return tokens;
}