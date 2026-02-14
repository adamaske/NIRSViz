#pragma once

#include <string>

struct SNIRFError {
	enum Code {
		ERROR_NONE = 0,
		FILE_NOT_FOUND = 1,
		INVALID_FORMAT = 2,
		MISSING_METADATA = 3,
		CHANNEL_MISMATCH = 4,
		UNSUPPORTED_VERSION = 5,
		DATA_CORRUPTION = 6,
		UNKNOWN = 7
	};

	Code code = ERROR_NONE;
	std::string message;

	SNIRFError(Code _code, std::string msg)
		: code(_code), message(msg) {
	}
};
