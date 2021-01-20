#ifndef SERVICE_EXECUTION_TYPE_EXCEPTION_HPP
#define SERVICE_EXECUTION_TYPE_EXCEPTION_HPP

#include <exception>

namespace WinServiceLib
{
	class ServiceExecutionTypeException : public std::exception
	{
	public:
		ServiceExecutionTypeException()
			: std::exception("Trying to run service as a console application") {}
	};
}

#endif /* SERVICE_EXECUTION_TYPE_EXCEPTION_HPP */