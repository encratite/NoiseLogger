#pragma once

#include <string>

#include <Fall/Console.hpp>

#define DEBUG_MARK Fall::log(std::string(__FILE__) + " (line " + std::to_string(__LINE__) + ")");