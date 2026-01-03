#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <cstddef>

using namespace std;

void init_logger(const string &filename, size_t max_size_bytes);

void log_info(const string &msg);

void close_logger();

#endif
