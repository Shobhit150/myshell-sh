#include "terminal.hpp"
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <algorithm>

class Node {
public:
    Node* arr[26]{};
    bool endsWith = false;
    int freq = 0;
    int bestFreq = 0;
};

static inline bool normalize(char &c) {
    if (c >= 'A' && c <= 'Z')
        c = c - 'A' + 'a'; 

    if (c < 'a' || c > 'z')
        return false;

    return true;
}

class Trie {
public:
    Node* root = new Node();
    void insert(std::string &s, int score = 1) {
        Node* temp = root;
        for(char &c: s) {
            if (!normalize(c)) return;
            int idx = c - 'a';
            if(temp->arr[idx] == nullptr) {
                temp->arr[idx] = new Node();
            }
            temp = temp->arr[idx];
            temp->bestFreq = std::max(temp->bestFreq, score);
        }
        temp->endsWith = true;  
        temp->freq += score;
    }

    Node* exist(Node* node, std::string &s) {
        Node* temp = node;
        for(int i=0;i<s.size();i++) {
            if (!normalize(s[i])) return nullptr;
            int idx = s[i] - 'a';
            if(temp->arr[idx] == nullptr) {
                return nullptr;
            }
            temp = temp->arr[idx];
        }
        return temp;
    }

    bool autocomplete(std::string &s) {
        Node* path = exist(root, s);
        if(path == nullptr) return false;
        std::string input = s;
        while(path->endsWith != true) {
            int bestIdx = -1;
            int best = -1;
            for(int i=0;i<26;i++) {
                if(path->arr[i] != nullptr) {
                    // input += char('a' + i);
                    // path = path->arr[i];
                    if(path->arr[i]->bestFreq > best) {
                        best = path->arr[i]->bestFreq;
                        bestIdx = i;
                    }
                }
            }
            if (bestIdx == -1) return false;
            input += char('a' + bestIdx);
            path = path->arr[bestIdx];
        }
        s = input + " ";
        return true;
    }
};

static termios orig_termios;
Trie t;

static std::vector<std::pair<std::string,int>> BUILDINS = {
    {"echo", 5},
    {"exit", 3}
};

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);

    termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void buildTree() {
    for(auto& [word, priority] : BUILDINS) {
        t.insert(word, priority);
    }
}

static bool autocomplete(std::string &input) {
    // for (const auto& cmd: BUILDINS) { // O(n) n = no. string
    //     if(cmd.substr(0, input.size()) == input) {
    //         input = cmd + " ";
    //         return true;
    //     }
    // }
    // return false;

    // Trie t;
    // for(int i=0;i<BUILDINS.size();i++) {
    //     auto [word, priority] = BUILDINS[i];
    //     t.insert(word, priority);
    // }
    return t.autocomplete(input);
}

std::string readLineRaw(const std::string &prompt) {
    std::cout << prompt << std::flush;

    enableRawMode();

    std::string input;
    char c;

    while(true) {
        if(read(STDIN_FILENO, &c, 1) != 1) {
            break;
        }
        if(c == '\n' || c == '\r') {
            std::cout << "\n";
            break;
        } else if (c == '\t') {
            if(autocomplete(input)) {
                std::cout << "\r" << prompt << input << std::flush;
            }
        } else if (c == 127) {
            if(!input.empty()) {
                input.pop_back();
                std::cout << "\b \b" << std::flush;
            }
        } else {
            input.push_back(c);
            std::cout << c << std::flush;
        }
    }
    disableRawMode();
    return input;
}
