# ServerManager
A Fly For Fun manager for server executables & database migrations.

## Main Features
- Executable management:
  - Allows starting/stopping/restarting executable files based on configuration.
  - Allows copying files based on configuration.
- Database management:
  - Allows creation and gestion of database migrations.

## Using this tool
Refer to the [example configuration documentation](examples/) for details on how to set up this tool.

## Modules and commands list
- Main Module:
	- `exit`: Exits the program, closing all executables.
	- `help`|`help <module_name>`: Prints the help message.
- Process Module:
	- `copy`: Copies server processes to target location(s).
	- `restart`: Restarts all server processes.
	- `start`: Starts all server processes.
	- `stop`: Closes all server processes.
- Database Module:
	- Instance Module:
		- `set_name`: Sets the name of the database instance. db:instance:set_name <ComputerName>\<InstanceName>.
	- Migration Module:
		- `list`: Lists all database migrations and their status.
		- `make`: Creates a migration file. db:migration:make migration_name
		- `run`: Runs all database migrations.

The Main and Process modules do not require their aliases to be used. Commands can be used directly via their name.  
The modules under Database require their aliases to be used. Commands will require the alias(es), separated by a colon (:).

## Dependencies
This project depends on the [nlohmann/json](https://github.com/nlohmann/json) and [color.hpp](https://github.com/hugorplobo/colors.hpp) libraries.