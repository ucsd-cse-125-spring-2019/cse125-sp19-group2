#include "Logger.hpp"

Logger * Logger::_instance = NULL;
std::mutex Logger::_mutex;
std::ostream* Logger::_os;
