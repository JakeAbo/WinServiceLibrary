#ifndef BASE_SERVICE_HPP_
#define BASE_SERVICE_HPP_

#include "ServiceExecutionTypeException.hpp"
#include "WinApiLastErrorException.hpp"

#include <Windows.h>
#include <assert.h>

namespace WinServiceLib
{
	/*
	* Base service class - using singletone design to run project as service on the windows OS.
	*/
	class BaseService
	{
	private:
		static BaseService*		_instance;		//The singleton instance
		const char*				_name;			//The name of the service
		SERVICE_STATUS			_status;		//The status of the service
		SERVICE_STATUS_HANDLE	_statusHandle; 	//The service status handle

		/*
		* Method: main
		* Task: Entry point for the service. It registers the handler function for the service and starts the service
		* Arguments: argc, argv - command line arguments for service
		* Return: None
		*/
		static void WINAPI main(unsigned long argc, char** argv)
		{
			assert(_instance);

			// Register the handler function for the service
			_instance->_statusHandle = RegisterServiceCtrlHandler(_instance->_name, handleControl);
			if (_instance->_statusHandle != 0)
			{
				// Start the service.
				_instance->start(argc, argv);
			}
		}

		//The function is called by the SCM whenever a control code is sent to 
		//the service
		/*
		* Method: handleControl
		* Task: Function is called byb the SCM whenever a control code is sent to the service
		* Args: unsigned long control - control code sent
		* Returns: None 
		*/
		static void WINAPI handleControl(unsigned long control)
		{
			switch (control)
			{
			case SERVICE_CONTROL_STOP:			_instance->stop();		break;
			case SERVICE_CONTROL_PAUSE:			_instance->pause();		break;
			case SERVICE_CONTROL_CONTINUE:		_instance->resume();	break;
			case SERVICE_CONTROL_SHUTDOWN:		_instance->shutdown();	break;
			default:													break;
			}
		}

	protected: 
		/*
		* Method: setStatus
		* Task: Set the service status and report the status to the SCM.
		* Args: currentState - the state of the service
		*		exitCode - error code to report
		*		waitHint - estimated time for pending operation, in milliseconds
		* Return: None
		*/
		void setStatus(unsigned long currentState, unsigned long exitCode = NO_ERROR, unsigned long waitHint = 0)
		{
			static unsigned long checkPoint = 1;

			// Fill in the SERVICE_STATUS structure of the service.
			_status.dwCurrentState = currentState;
			_status.dwWin32ExitCode = exitCode;
			_status.dwWaitHint = waitHint;
			_status.dwCheckPoint = ((currentState == SERVICE_RUNNING) || (currentState == SERVICE_STOPPED)) ? 0 : checkPoint++;

			// Report the status of the service to the SCM.
			SetServiceStatus(_statusHandle, &_status);
		}

		/*
		* Method: onStart
		* Task: Pure virtual method - When implemented in a derived class, executes when a Start command is
		*		sent to the service by the SCM or when the operating system starts (for a service that starts automatically).
		*		The function starts the service. It calls the OnStart virtual function in which you can specify the actions
		*		to take when the service starts.
		* Args: command line arguments
		* Return: None.
		*/
		virtual void onStart(unsigned long argc, char** argv) = 0;

		/*
		* Method: onStop
		* Task: virtual method - When implemented in a derived class, executes when a Stop command is
		*		sent to the service by the SCM. The function stops the service. It calls the OnStop virtual function 
		*		in which you can specify the actions to take when the service stops.
		* Args: None
		* Return: None.
		*/
		virtual void onStop() {}

		/*
		* Method: onPause
		* Task: virtual method - When implemented in a derived class, executes when a Pause command is
		*		sent to the service by the SCM. Specifies actions to take when a service pauses.
		*		The function pauses the service if the service supports pause and continue.
		*		It calls the OnPause virtual function in which you can specify the actions to take when the service pauses.
		* Args: None
		* Return: None.
		*/
		virtual void onPause() {}

		/*
		* Method: onResume
		* Task: virtual method - When implemented in a derived class, executes when a Continue command is
		*		sent to the service by the SCM. Specifies actions to take when a service resumes normal functioning after being paused.
		*		The function resumes normal functioning after begin paused if the service supports pause and continue.
		*		It calls the onResume virtual function in which you can specify the actions to take when the service resumes.
		* Args: None
		* Return: None.
		*/
		virtual void onResume() {}

		//When implemented in a derived class, executes when the system is 
		//shutting down. Specifies what should occur immediately prior to the 
		//system shutting down. The function executes when the system is shutting down.
		//It calls the OnShutdown virtual function in which you can specify what 
		//should occur immediately prior to the system shutting down
		
		/*
		* Method: onShutdown
		* Task: virtual method - When implemented in a derived class, executes when the system is shutting down.
		*		Specifies what should occur immediately prior to the system shutting down. 
		*		The function executes when the system is shutting down.
		*		It calls the onShutdown virtual function in which you can specify should occur immediately prior to the system shutting down.
		* Args: None
		* Return: None.
		*/
		virtual void onShutdown() {}

	public:
		/*
		* Method: Constructor
		* Task: Construct BaseService instances
		* Args: name - name of service
		*		canStop - boolean value wether service can stop
		*		canShutdown - boolean value wether service can shutdown
		*		canPauseContinue - boolean value wether service can pause and continue
		* Returns: Instance of BaseService
		*/
		BaseService(const char* name, bool canStop = true, bool canShutdown = true, bool canPauseContinue = false)
			: _name(name), _statusHandle(NULL)
		{
			assert(name);
			assert(name[0]); //Not an empty string

			//Commands accepted by the service
			DWORD dwControlsAccepted = 0;
			if (canStop)
			{
				dwControlsAccepted |= SERVICE_ACCEPT_STOP;
			}

			if (canShutdown)
			{
				dwControlsAccepted |= SERVICE_ACCEPT_SHUTDOWN;
			}

			if (canPauseContinue)
			{
				dwControlsAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;
			}

			_status.dwControlsAccepted = dwControlsAccepted;
			_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;	// The service runs in its own process
			_status.dwCurrentState = SERVICE_START_PENDING;		// The service is starting
			_status.dwWin32ExitCode = NO_ERROR;
			_status.dwServiceSpecificExitCode = 0;
			_status.dwCheckPoint = 0;
			_status.dwWaitHint = 0;
		}

		/*
		* Method: Destructor
		* Task: Destrut BaseService instances
		* Args: None
		* Returns: None
		*/
		virtual ~BaseService(void)
		{}

		/*
		* Method: run
		* Task: Start the service using the SCM and give a entry point (which is the method main)
		* Args: BaseService* service to start
		* Return: None
		*/
		static void run(BaseService* service)
		{
			_instance = service;
			char* serviceName = const_cast<char*>(_instance->getName());

			SERVICE_TABLE_ENTRY serviceTable[] =
			{
				{ serviceName, &BaseService::main },
				{ NULL, NULL }
			};

			// Connects the main thread of a service process to the service control 
			// manager, which causes the thread to be the service control dispatcher 
			// thread for the calling process. This call returns when the service has 
			// stopped. The process should simply terminate when the call returns.
			if (StartServiceCtrlDispatcher(serviceTable) == 0)
			{
				if (GetLastError() == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT)
				{ //This means the program is being run as a console application
					throw ServiceExecutionTypeException();
				}
				else
				{ //Some other error
					throw WinApiLastErrorException("Run service StartServiceCtrlDispatcher failed", GetLastError());
				}
			}
		}

		/* Return name of service */
		const char* getName() 
		{ 
			return _name; 
		}

		/*
		* Method groups: Control service life cycle
		* Task: Control all service operations sent by SCM - start, pause, resume, shutdown, stop
		*/
		void start(unsigned long argc, char** argv)
		{
			try
			{
				// Tell SCM that the service is starting.
				setStatus(SERVICE_START_PENDING);

				// Tell SCM that the service is started.
				setStatus(SERVICE_RUNNING);

				// Perform service-specific initialization.
				onStart(argc, argv);
			}
			catch (DWORD error)
			{
				// Set the service status to be stopped.
				setStatus(SERVICE_STOPPED, error);

				throw std::exception("Service start error");
			}
			catch (...)
			{
				// Set the service status to be stopped.
				setStatus(SERVICE_STOPPED);

				throw std::exception("Service failed to start");
			}
		}
		void stop()
		{
			unsigned long original_state = _status.dwCurrentState;

			try
			{
				// Tell SCM that the service is stopping.
				setStatus(SERVICE_STOP_PENDING);

				// Perform service-specific stop operations.
				onStop();

				// Tell SCM that the service is stopped.
				setStatus(SERVICE_STOPPED);
			}
			catch (DWORD)
			{
				// Set the orginal service status.
				setStatus(original_state);
			}
			catch (...)
			{
				// Set the orginal service status.
				setStatus(original_state);
			}
		}
		void pause()
		{
			try
			{
				// Tell SCM that the service is pausing.
				setStatus(SERVICE_PAUSE_PENDING);

				// Perform service-specific pause operations.
				onPause();

				// Tell SCM that the service is paused.
				setStatus(SERVICE_PAUSED);
			}
			catch (DWORD)
			{
				// Tell SCM that the service is still running.
				setStatus(SERVICE_RUNNING);
			}
			catch (...)
			{
				// Tell SCM that the service is still running.
				setStatus(SERVICE_RUNNING);
			}
		}
		void resume()
		{
			try
			{
				// Tell SCM that the service is resuming.
				setStatus(SERVICE_CONTINUE_PENDING);

				// Perform service-specific continue operations.
				onResume();

				// Tell SCM that the service is running.
				setStatus(SERVICE_RUNNING);
			}
			catch (DWORD)
			{
				// Tell SCM that the service is still paused.
				setStatus(SERVICE_PAUSED);
			}
			catch (...)
			{
				// Tell SCM that the service is still paused.
				setStatus(SERVICE_PAUSED);
			}
		}
		void shutdown()
		{
			try
			{
				// Perform service-specific shutdown operations.
				onShutdown();

				// Tell SCM that the service is stopped.
				setStatus(SERVICE_STOPPED);
			}
			catch (DWORD)
			{
			}
			catch (...)
			{
			}
		}
	};

	BaseService* BaseService::_instance = NULL;
}

#endif /* BASE_SERVICE_HPP_ */