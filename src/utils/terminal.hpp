// terminal.hpp
#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include "../shell_state.hpp"
std::string readLineRaw(const std::string& prompt);
void buildTree(ShellState &state);
void recordUsage(const std::string &cmd);