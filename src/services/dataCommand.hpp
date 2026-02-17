#pragma once
#include<vector>
#include<iostream>
#include "../shell_state.hpp"

struct RedirectInfo {
    std::string stdoutFile;
    std::string stderrFile;
    bool stdoutAppend = false;
    bool stderrAppend = false;
};

struct SavedFDs {
    int out = -1;
    int err = -1;
};

void handleEcho(std::vector<std::string> &tokens);
void handleType(std::vector<std::string> &tokens, ShellState &state);
void searchPath(std::vector<std::string>& tokens, ShellState &state);
void handlePwd();
void handleCd(std::vector<std::string>& tokens);
SavedFDs applyRedirect(RedirectInfo &r);
void restoreFDs(const SavedFDs& s);
RedirectInfo extractRedirect(std::vector<std::string>& tokens);