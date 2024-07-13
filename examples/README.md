# Example Configuration
This is an example configuration for the ServerManager.

## Contents
- `DBMigrations` directory: This directory contains all Database migrations created by the tool. When a migration is created via the command, head here to update the file. Run the migrations when done.
- `latest_migration.txt` file: This file stores the date & time of the latest applied migration. It does not need to be manually modified.
- `config.json`: This file stores the name of the database instance used, as well as the list of the executables and a list of the files copied by the `copy` command.

It is recommended to place your ServerManager.exe in a directory with a structure similar to this one.