#pragma once

#include <string>

struct SNIRFError {
	std::string message;
	SNIRFError(const std::string& msg) : message(msg) {}
};