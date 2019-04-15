#include "IdGenerator.hpp"

std::mutex IdGenerator::_mutex;
IdGenerator * IdGenerator::_instance = NULL;
