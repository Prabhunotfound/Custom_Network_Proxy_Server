#ifndef CONFIG_H
#define CONFIG_H

#include <string>

using namespace std;

struct Config
{
    string listen_address = "";
    string blocklist_file = "";
    string log_file = "";
    string metrics_file = "";
    bool enable_blocklist = true;
    bool enable_https_tunnel = true;
    bool log_enabled = true;
    int connection_timeout_sec;
    int listen_port;
    int thread_pool_size;
    size_t log_max_size_bytes;
};

bool load_config(const string &filename, Config &config);

bool validate(Config &config);

#endif
