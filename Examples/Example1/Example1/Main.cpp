#include <iostream>
#include <string>

#include "Example1Service.hpp"

#include "ServiceManager.hpp"

std::string getExecutionPath()
{
	//Get module execution path
	char buffer[MAX_PATH];
	if (GetModuleFileName(NULL, buffer, sizeof(buffer)) == 0)
	{

		throw std::exception("Get module file name failed"/*,GetLastError()*/);
	}
	return buffer;
}

std::string getExecutionDirectory()
{
	std::string splitter = "\\";
	std::string str = getExecutionPath();
	size_t last_index = str.find_last_of(splitter);
	if (last_index == std::string::npos)
	{
		return std::string("");
	}
	else
	{
		size_t offset = splitter.length();
		return str.substr(0, last_index + offset);
	}
}

/* Run as service - will only run if is on service mode and on console mode! */
void runAsService()
{
	ExampleService service(getExecutionDirectory());
	WinServiceLib::BaseService::run((WinServiceLib::BaseService*)&service);
}

/* Register service using ServiceManager Module */
void registerService()
{
	WinServiceLib::ServiceManager::installService(getExecutionPath().c_str(),
		ExampleService::NAME, ExampleService::DISPLAY_NAME,
		ExampleService::DEPENDENCIES, ExampleService::ACCOUNT, ExampleService::PASSWORD,
		ExampleService::DESCRIPTION, ExampleService::START_TYPE);
}

/* Unregister service using ServiceManager Module */
void unregisterService()
{
	WinServiceLib::ServiceManager::uninstallService(ExampleService::NAME);
}

int main()
{
	/* this code will run when project run as service - after registered using the ServiceManager Module */
	runAsService();
	return 0;
}