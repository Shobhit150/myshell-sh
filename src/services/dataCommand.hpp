#pragma once
#include<vector>
#include<iostream>
#include "../shell_state.hpp"

void handleEcho(std::vector<std::string> &tokens);
void handleType(std::vector<std::string> &tokens, ShellState &state);