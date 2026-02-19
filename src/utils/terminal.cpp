// terminal.cpp
#include "terminal.hpp"
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <algorithm>

class Node {
public:
    Node* arr[128]{};
    bool endsWith = false;
    int freq = 0;
    int bestFreq = 0;
};

class Trie {
public:
    Node* root = new Node();
    void insert(const std::string &s, int score = 1) {
        Node* temp = root;

        for (unsigned char c : s) {
            if (!temp->arr[c])
                temp->arr[c] = new Node();

            temp = temp->arr[c];
            temp->bestFreq = std::max(temp->bestFreq, score);
        }

        temp->endsWith = true;
        temp->freq += score;
    }

    Node* exist(Node* node, const std::string &s) {
        Node* temp = node;

        for (unsigned char c : s) {
            if (!temp->arr[c]) return nullptr;
            temp = temp->arr[c];
        }

        return temp;
    }

    void recordUsage(const std::string &s) {
        Node* temp = root;
        std::vector<Node*> path;

        for (unsigned char c : s) {
            if (!temp->arr[c]) return;
            temp = temp->arr[c];
            path.push_back(temp);
        }
        if (!temp->endsWith) return;
        temp->freq++;
        int f = temp->freq;
        for(auto &node: path) {
            node->bestFreq = std::max(node->bestFreq, f);
        }
    }

    bool autocomplete(std::string &s) {
        Node* path = exist(root, s);

        if(path == nullptr) return false;
        if (path->endsWith) {
            s += " ";
            return true;
        }
        std::string input = s;
        while(path->endsWith != true) {
            int bestIdx = -1;
            int best = -1;
            for (int i = 0; i < 128; i++) {
                if (path->arr[i] &&
                    path->arr[i]->bestFreq > best) {
                    best = path->arr[i]->bestFreq;
                    bestIdx = i;
                }
            }
            if (bestIdx == -1) return false;
            input += char(bestIdx);
            path = path->arr[bestIdx];
        }
        s = input + " ";
        return true;
    }
};

Trie t;
static termios orig_termios;

static std::vector<std::pair<std::string,int>> BUILTINS = {
    {"echo", 1},
    {"exit", 1}
};

void recordUsage(const std::string &cmd) {
    t.recordUsage(cmd);
}

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
    for(auto &[word, priority]: BUILTINS) {
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
