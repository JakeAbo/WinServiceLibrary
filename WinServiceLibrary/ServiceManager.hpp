#ifndef SERVICE_MANAGER_HPP_
#define SERVICE_MANAGER_HPP_

#include <Windows.h>
#include <exception>

namespace WinServiceLib
{
	/*
	* This module manages the Service class
	* It allows interaction with the SCM including installation of the Service
	*/
	class ServiceManager
	{
	private:
		enum class Action
		{
			START,
			STOP,
			UNINSTALL
		};

		/* Keep checking service is still stop pending every 1 second until it stop. */
		static const unsigned long STATE_CHANGE_WAIT = 1000;

		/*
		* Method: controlService
		* Task: Controls an installed service - start, stop or uninstall
		* Args: service_name - name of Service
		*		service_action - start, stop or uninstall
		*		service_access - access to open service
		*		manager_access - access to open SCM
		* Return: None
		*/
		static void controlService(const char* service_name, Action service_action, unsigned long service_access, unsigned long manager_access)
		{
			SC_HANDLE services_manager = NULL;
			SC_HANDLE service_handle = NULL;

			try
			{
				services_manager = serviceOpenManager(manager_access);
				service_handle = serviceOpen(services_manager, service_name, service_access);

				switch (service_action)
				{
				case Action::START:
					serviceStart(service_handle);
					break;
				case Action::STOP:
					serviceStop(service_handle);
					break;
				case Action::UNINSTALL:
					serviceStop(service_handle);
					serviceDelete(service_handle);
					break;
				default:
					throw std::exception("Unknown control requested");
				}
			}
			catch (const std::exception& ex)
			{
				std::cout << ex.what() << std::endl;
				serviceCleanupHandles(service_handle, services_manager);
				throw;
			}

			serviceCleanupHandles(service_handle, services_manager);
		}

		/*
		* Method: serviceOpenManager
		* Task: Open SCM - Service control manager
		* Args: manager_access - The access required when using the manager
		* Returns: A handle to the manager with the requested access
		*/
		static SC_HANDLE serviceOpenManager(unsigned long manager_access)
		{
			SC_HANDLE services_manager;

			// Open the local default service control manager database
			if ((services_manager = OpenSCManager(NULL, NULL, manager_access)) == NULL)
			{
				throw std::exception("OpenSCManager failed", GetLastError());
			}

			return services_manager;
		}

		/*
		* Method: serviceSetDescription
		* Task: Sets the service description as displayed in the services manager
		* Args: service_handle - A handle to the service whos description we want to set
		*		service_description - The description to set for the service
		* Returns: None
		*/
		static void serviceSetDescription(SC_HANDLE service_handle, const char* service_description)
		{
			SERVICE_DESCRIPTION description = { const_cast<char*>(service_description) };

			//Important, to execute this, we must request CHANGE_CONFIG access when creating the server
			if (ChangeServiceConfig2(service_handle, SERVICE_CONFIG_DESCRIPTION, &description) == 0)
			{
				throw std::exception("ChangeServiceConfig2 failed", GetLastError());
			}
		}

		/*
		* Method: serviceCreate
		* Task: Creates a new service
		* Args: services_manager - The services manager
		*		service_executable_path - The service's executable path
		*		service_access - The type of access required from the returned handle
		*		service_name - The name of the service
		*		service_display_name - The display name of the service
		*		service_dependencies - The dependencies of the service
		*		service_account - Under which user should the service run
		*		service_password - The password for the given account
		*		service_start_type - How should the service start
		* Returns: A handle to the created service with the requested access
		*/
		static SC_HANDLE serviceCreate(SC_HANDLE services_manager, const char* service_executable_path, unsigned long service_access, const char* service_name,
			const char* service_display_name, const char* service_dependencies, unsigned long service_start_type, const char* service_account, const char* service_password)
		{
			SC_HANDLE service_handle;

			// Install the service into SCM by calling CreateService
			if ((service_handle = CreateService(
				services_manager,		// SCManager database
				service_name,					// Name of service
				service_display_name,			// Name to display
				service_access,					// Service access
				SERVICE_WIN32_OWN_PROCESS,		// Service type
				service_start_type,				// Service start type
				SERVICE_ERROR_NORMAL,			// Error control type
				service_executable_path,		// Service's binary
				NULL,                           // No load ordering group
				NULL,                           // No tag identifier
				service_dependencies,			// Dependencies
				service_account,				// Service running account
				service_password				// Password of the account
			)) == NULL)
			{
				throw std::exception("CreateService failed", GetLastError());
			}

			return service_handle;
		}

		/*
		* Method: serviceOpen
		* Task: Opens an already installed service
		* Args: services_manager - The services manager
		*		service_name - The name of the service to open
		*		service_access - The type of access required from the returned handle
		* Returns: A handle to the opened service with the requested access
		*/
		static SC_HANDLE serviceOpen(SC_HANDLE services_manager, const char* service_name, unsigned long service_access)
		{
			SC_HANDLE service_handle;

			if ((service_handle = OpenService(services_manager, service_name, service_access)) == NULL)
			{
				throw std::exception("OpenService failed", GetLastError());
			}

			return service_handle;
		}

		/*
		* Method: serviceStart
		* Task: Starts an already installed service using it's handle
		* Args: service_handle - A service to the handle.
		* Returns: None
		* 
		* Notice: The handle must have SERVICE_START access.
		*/
		static void serviceStart(SC_HANDLE service_handle)
		{
			if (StartService(service_handle, 0, NULL) == 0)
			{
				throw std::exception("StartService failed", GetLastError());
			}
		}

		/*
		* Method: serviceStop
		* Task: Stops an already installed service using it's handle
		* Args: service_handle - A service to the handle.
		* Returns: None
		* 
		* Notice: The handle must have SERVICE_STOP and SERVICE_QUERY_STATUS access.
		*/
		static void serviceStop(SC_HANDLE service_handle)
		{
			SERVICE_STATUS service_status = {};

			if (ControlService(service_handle, SERVICE_CONTROL_STOP, &service_status) == 0 && GetLastError() != ERROR_SERVICE_NOT_ACTIVE)
			{
				throw std::exception("ControlService failed", GetLastError());
			}

			//Query service status and wait until it's state is not longer stop pending
			while (QueryServiceStatus(service_handle, &service_status) && service_status.dwCurrentState == SERVICE_STOP_PENDING)
			{
				Sleep(STATE_CHANGE_WAIT);
			}

			if (service_status.dwCurrentState != SERVICE_STOPPED)
			{
				throw std::exception("Service state is not SERVICE_STOPPED");
			}
		}

		/*
		* Method: serviceDelete
		* Task: Starts an already installed service using it's handle
		* Args: service_handle - A service to the handle.
		* Returns: None
		*
		* Notice: The handle must have SERVICE_STOP, SERVICE_QUERY_STATUS and DELETE access.
		*/
		static void serviceDelete(SC_HANDLE service_handle)
		{
			if (DeleteService(service_handle) == FALSE)
			{
				throw std::exception("DeleteService failed", GetLastError());
			}
		}

		/*
		* Method: serviceCleanupHandles
		* Task: Cleanup service and service control manager handles
		* Args: service_handle - service handle to free
		*		service_control_manager - scm handle to free
		* Return: None
		*/
		static void serviceCleanupHandles(SC_HANDLE service_handle, SC_HANDLE service_control_manager)
		{
			serviceCleanupHandle(service_handle);
			serviceCleanupHandle(service_control_manager);
		}

		/*
		* Method: serviceCleanupHandle
		* Task: Cleans up service handle safely
		* Args: handle - The handle to cleanup
		* Return: None
		*/
		static void serviceCleanupHandle(SC_HANDLE handle)
		{
			if (handle)
			{
				CloseServiceHandle(handle);
			}
		}

	public:
		/* Static class - deleted constructor & destructor */
		ServiceManager() = delete;
		~ServiceManager() = delete;
		
		/*
		* Method: serviceGetPath
		* Task: Get service executable path
		* Args:	buffer - The buffer to copy the path into
		*		buffer_size - The size of the buffer
		* Returns: None
		*/
		static void serviceGetPath(char* buffer, size_t buffer_length)
		{
			//Get service executable path
			if (GetModuleFileName(NULL, buffer, buffer_length) == 0)
			{
				throw std::exception("GetModuleFileName failed", GetLastError());
			}
		}

		/*
		* Method: installService
		* Task: Installs the Service with the SCM
		* Args:	service_path - The path to exe
		*		service_name - The name of the service
		*		service_display_name - The display name of the service
		*		service_dependencies - The dependencies of the service
		*		service_account - Under which user should the service run
		*		service_password - The password for the given account
		*		service_start_type - How should the service start
		* Returns: None
		*/
		static void installService(const char* service_path, const char* service_name, const char* service_display_name, const char* service_dependencies,
			const char* service_account, const char* service_password, const char* service_description, unsigned long service_start_type)
		{
			unsigned long manager_access = SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE;
			unsigned long service_access = SERVICE_CHANGE_CONFIG;

			SC_HANDLE services_manager = NULL;
			SC_HANDLE service_handle = NULL;

			try
			{
				services_manager = serviceOpenManager(manager_access);

				service_handle = serviceCreate(services_manager, service_path,
					service_access, service_name, service_display_name, service_dependencies,
					service_start_type, service_account, service_password);

				serviceSetDescription(service_handle, service_description);
			}
			catch (const std::exception& ex)
			{
				serviceCleanupHandles(service_handle, services_manager);
				throw std::exception(ex.what(), GetLastError());
			}

			serviceCleanupHandles(service_handle, services_manager);
		}

		/*
		* Method: uninstallService
		* Task: Uninstalls the Service from the SCM
		* Args: service_name - The name of the service to uninstall
		* Returns: None
		*/
		static void uninstallService(const char* service_name)
		{
			unsigned long manager_access = SC_MANAGER_CONNECT;
			unsigned long service_access = SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE;

			controlService(service_name, Action::UNINSTALL, service_access, manager_access);
		}

		/*
		* Method: startService
		* Task: Starts an installed service using the SCM.
		*		The SCM then issues a Start command, which results in a call to the OnStart method in the service.
		* 
		* Args:	service_name - The name of the service to start
		* Returns: None
		*/
		static void startService(const char* service_name)
		{
			unsigned long manager_access = SC_MANAGER_CONNECT;
			unsigned long service_access = SERVICE_START;


			controlService(service_name, Action::START, service_access, manager_access);
		}

		/*
		* Method: stopService
		* Task: Stop a running service not from inside the owning process.
		*		Finds the service using the SCM and issues a stop command.
		* 
		* Args: service_name - The name of the service to stop/
		* Returns: None.
		*/
		static void stopService(const char* service_name)
		{
			unsigned long manager_access = SC_MANAGER_CONNECT;
			unsigned long service_access = SERVICE_STOP | SERVICE_QUERY_STATUS;


			controlService(service_name, Action::STOP, service_access, manager_access);
		}
	};
}

#endif /* SERVICE_MANAGER_HPP_ */