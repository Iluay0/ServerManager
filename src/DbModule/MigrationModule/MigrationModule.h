#pragma once

#include "../../InputModule.h"

class MigrationModule : public InputModule
{
public:
	MigrationModule();
	~MigrationModule();

	static std::string getLastMigration();
	static std::string getMigrationDate(std::string szFilePath);
	static int getPendingMigrations();
protected:
	void bindCommands() override;
private:
	void make(std::deque<std::string> args);
	void run(std::deque<std::string> args);
	void list(std::deque<std::string> args);
private:
	static void setLastMigration(std::string migrationDate);
};