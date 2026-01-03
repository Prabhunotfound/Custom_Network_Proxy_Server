#include <fstream>
#include <iostream>
#include <algorithm>
#include "config.h"

using namespace std;

static string trim(const string &s)
{
    size_t start = s.find_first_not_of(" \t");
    size_t end = s.find_last_not_of(" \t");
    if (start == string::npos)
        return "";
    return s.substr(start, end - start + 1);
}

static bool to_bool(const string &v)
{
    return v == "true" || v == "1" || v == "yes";
}

bool load_config(const string &filename, Config &config)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "[ERROR] Cannot open config file: " << filename << endl;
        return false;
    }

    string line;
    while (getline(file, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        size_t eq = line.find('=');
        if (eq == string::npos)
            continue;

        string key = trim(line.substr(0, eq));
        string value = trim(line.substr(eq + 1));

        if (key == "listen_address")
            config.listen_address = value;
        else if (key == "listen_port")
            config.listen_port = stoi(value);
        else if (key == "thread_pool_size")
            config.thread_pool_size = stoi(value);
        else if (key == "blocklist_file")
            config.blocklist_file = value;
        else if (key == "log_file")
            config.log_file = value;
        else if (key == "enable_blocklist")
            config.enable_blocklist = to_bool(value);
        else if (key == "enable_https_tunnel")
            config.enable_https_tunnel = to_bool(value);
        else if (key == "log_enabled")
            config.log_enabled = to_bool(value);
        else if (key == "log_max_size_bytes")
            config.log_max_size_bytes = stoul(value);
        else if (key == "metrics_file")
            config.metrics_file = value;
        else if (key == "connection_timeout_sec")
            config.connection_timeout_sec = stoi(value);
    }

    return true;
}

bool validate(Config &config)
{
    if (config.listen_address.empty())
        config.listen_address = "0.0.0.0";

    if (config.thread_pool_size <= 0)
        config.thread_pool_size = 4;

    if (config.log_max_size_bytes == 0)
        config.log_max_size_bytes = 64 * 1024; // 64 Kb

    if (config.metrics_file.empty())
        config.metrics_file = "config/metrics.txt";

    if (config.listen_port <= 0 || config.listen_port > 65535)
    {
        cerr << "[CONFIG ERROR] Invalid listen_port: " << config.listen_port << endl;
        return false;
    }

    if (config.log_file.empty())
    {
        config.log_file = "config/logs/proxy.log";
    }

    if (config.blocklist_file.empty())
    {
        config.blocklist_file = "config/blocked_sites.txt";
    }

    return true;
}
