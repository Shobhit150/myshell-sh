// terminal.hpp
#pragma once
#include <string>
#include <vector>
#include <algorithm>

std::string readLineRaw(const std::string& prompt);
void buildTree();
void recordUsage(const std::string &cmd);