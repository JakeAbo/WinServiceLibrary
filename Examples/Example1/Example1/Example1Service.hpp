#ifndef EXAMPLE_1_SERVICE_HPP_
#define EXAMPLE_1_SERVICE_HPP_

#include "BaseService.hpp"

class ExampleService : public WinServiceLib::BaseService
{
private:
	std::string		_execution_path;

	virtual void onStart(unsigned long argc, char** argv) override
	{
		std::cout << "Just started" << std::endl;
	}
	virtual void onStop() override
	{
		std::cout << "Just stopped" << std::endl;
	}

public:
	static const char* NAME;
	static const char* DISPLAY_NAME;
	static const char* DESCRIPTION;
	static const char* DEPENDENCIES;
	static const char* ACCOUNT;
	static const char* PASSWORD;
	static const unsigned long START_TYPE;

	explicit ExampleService(const std::string& execution_path)
		: WinServiceLib::BaseService(NAME, true, false, false),
		_execution_path(execution_path) 
	{}
	
	~ExampleService() 
	{}
};

const char*			ExampleService::NAME = "ExampleService";
const char*			ExampleService::DISPLAY_NAME = "Example Service";
const char*			ExampleService::DESCRIPTION = "Example Service description";
const char*			ExampleService::DEPENDENCIES = NULL;
const char*			ExampleService::ACCOUNT = NULL;
const char*			ExampleService::PASSWORD = NULL;
const unsigned long ExampleService::START_TYPE = SERVICE_AUTO_START;

#endif /* EXAMPLE_1_SERVICE_HPP_ */