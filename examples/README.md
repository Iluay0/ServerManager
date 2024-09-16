# Example Configuration
This is an example configuration for the ServerManager.

## Contents
- `DBMigrations` directory: This directory contains all Database migrations created by the tool. When a migration is created via the command, head here to update the file. Run the migrations when done.
- `latest_migration.txt` file: This file stores the date & time of the latest applied migration. It does not need to be manually modified.
- `config.json`: This file stores the name of the database instance used, as well as the list of the executables and a list of the files copied by the `copy` command. It also stores various configuration options for ServerManager.

## Start Check Method
By default, ServerManager waits until a window is open to minimize it and open the next process.  
It is possible to modify this for a more reliable method, via file creation.  
ServerManager will create a `.boot` file upon attempting to start a process, which will then be deleted when the process is done starting.  
In order to apply this, update the `startCheckMethod` value of `config.json` to `"file"`, and call this function in all of your server processes after `InitInstance()` has successfully performed:
```cpp
void OnAfterInitInstance()
{
	char buf[MAX_PATH];
	GetModuleFileNameA(nullptr, buf, MAX_PATH);
	std::string szFullName = PathFindFileName(buf);
	size_t nLastIndex = szFullName.find_last_of(".");
	std::string szRawName = szFullName.substr(0, nLastIndex);

	std::string bootFileName = std::format("{}.exe.boot", szRawName);
	if (std::filesystem::exists(bootFileName))
		std::filesystem::remove(bootFileName);
}
```

## Additional Information
It is recommended to place your ServerManager.exe in a directory with a structure similar to this one.