#include "Logger.hpp"
#include "windows.h"

Logger * Logger::_instance = NULL;
std::mutex Logger::_mutex;
std::ostream* Logger::_os;

void Logger::clearLine()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	int numColumns = csbi.srWindow.Right - csbi.srWindow.Left;
	*_os << '\r' << std::string(numColumns, ' ') << '\r';
}
