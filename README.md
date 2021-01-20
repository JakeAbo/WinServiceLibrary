# WinServiceLibrary
WinServiceLibrary is C-Style library written with 2 main modules:
1) BaseService - Base class to inherits from. the class make your project run the code when its on service mode on OS.
2) ServiceManager - Standalone module which interacts the SCM (Service control manager) using WINAPI to register, unregister, start, stop, pause, resume, shutdown services from code.

----
*Author*:      [Jack Abulafia](https://www.linkedin.com/in/jackabu)

## Dependencies
The library requires no dependencies.

Windows OS.

## Building 
The library is header-only. 

## How to use
1) Register the service of your project on OS.
2) Inherit from the BaseService and implement onStart(), onStop(), onPause(), onContinue(), onShutdown().

## Getting Started
Inherits from BaseService to imeplement your own service for your own project.
You want to put the code when service is start on onStart() method and write down the code when stop on onStop() method.
```cpp
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
```

Than you should register your service using the ServiceManager
```cpp
/* Return execution path of exe */
std::string getExecutionPath()
{
	char buffer[MAX_PATH];
	WinServiceLib::ServiceManager::serviceGetPath(buffer, MAX_PATH);
	return buffer;
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
```


## License 
This project is open source and freely available.

  Copyright (c) 2020, Jack Abulafia 
  All rights reserved.
