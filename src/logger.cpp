#include <fstream>
#include <mutex>
#include <ctime>
#include <sys/stat.h>
#include <cstdio>
#include <iostream>
#include "logger.h"

using namespace std;

static ofstream log_file;
static mutex log_mutex;
static string log_filename;
static size_t max_size = 0;

static string timestamp()
{
    time_t now = time(nullptr);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return buf;
}

static void rotate_if_needed()
{
    struct stat st{};
    if (stat(log_filename.c_str(), &st) == 0 && (size_t)st.st_size >= max_size)
    {
        log_file.close();
        remove((log_filename + ".1").c_str());
        rename(log_filename.c_str(), (log_filename + ".1").c_str());
        log_file.open(log_filename, ios::out | ios::app);
    }
}

void init_logger(const string &filename, size_t max_size_bytes)
{
    log_filename = filename;
    max_size = max_size_bytes;
    log_file.open(filename, ios::out | ios::app);
}

void log_info(const string &msg)
{
    lock_guard<mutex> lock(log_mutex);
    rotate_if_needed();
    log_file << "[" << timestamp() << "] " << msg << endl;
}

void close_logger()
{
    if (log_file.is_open())
    {
        log_file.close();
        cout << "[INFO] Log File Closed" << endl;
    }
}
