#ifndef _FILE_EXPLORER_H
#define _FILE_EXPLORER_H

#include <string>
#include <stack>
#include <vector>

enum class FILETYPE;

enum class MenuOperations
{
	NONE = 0,
	LOAD,
	ADD,
	SAVE
};

class FileExplorer
{
public:
	FileExplorer();
	~FileExplorer();

	void Draw();		// Draws the modal pop-up
	std::vector<std::string> GetPath(std::string prevPath); //Get path previous to actual folder
	void SetPath(const char* newPath);						// Sets path and pathStack variables
	void OpenFileExplorer(MenuOperations operation, FILETYPE typeToFilter = FILETYPE(sizeof(FILETYPE)), 
		const char* startPath = "", const char* windowTitle = "File Explorer", const char* fileName = ""); // Sets all customizable variables and opens the File Explorer

private:
	void Reset();		// Resets the file explorer
	void ResetPath();	// Resets only path related variables
	std::vector<std::string> GetPath(); //Get path for direction bar

	bool Open();		// Returns true on accepted, false on cancelled or running
	void DrawPath();	// Draws the path directories bar
	void FilterByFileType(const char* file);	// Draws a selectable for the file given if has the extension selected to filter

public:
	bool openFileExplorer = false;
	char title[100] = "File Explorer";
	char filename[1024] = "";
	std::string path = "";
	std::stack<std::string> pathStack;

	FILETYPE extensionToFilter;
	MenuOperations currentOperation = MenuOperations::NONE;
};

#endif