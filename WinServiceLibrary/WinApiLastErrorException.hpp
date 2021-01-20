#ifndef WIN_API_LAST_ERROR_EXCEPTION_HPP
#define WIN_API_LAST_ERROR_EXCEPTION_HPP

#include <stdexcept>

namespace WinServiceLib
{
	class WinApiLastErrorException : public std::runtime_error
	{
	public:
		unsigned int lastErrorCode;

		WinApiLastErrorException(const std::exception& ex, unsigned int lastError)
			: std::runtime_error(ex.what()), lastErrorCode(lastError) {}

		WinApiLastErrorException(const std::string& message, unsigned int lastError)
			: std::runtime_error(message), lastErrorCode(lastError) {}
	};
}

#endif /* WIN_API_LAST_ERROR_EXCEPTION_HPP */