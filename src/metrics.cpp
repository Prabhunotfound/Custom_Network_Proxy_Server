#include <fstream>
#include <mutex>
#include <unordered_map>
#include <ctime>
#include "metrics.h"

using namespace std;

static mutex m;
static string metrics_file;
static time_t start_time;
static size_t total_requests = 0;
static size_t blocked_requests = 0;
static size_t allowed_requests = 0;
static size_t bytes_transferred = 0;
static unordered_map<string, size_t> host_counts;
static string top_host;
static size_t top_host_count = 0;

static void flush()
{
    time_t now = time(nullptr);
    double elapsed_minutes = difftime(now, start_time) / 60.0;

    double rpm = 0.0;
    if (elapsed_minutes > 0)
        rpm = total_requests / elapsed_minutes;

    ofstream out(metrics_file, ios::out | ios::trunc);

    out << "Total Requests : " << total_requests << "\n";
    out << "Blocked Requests : " << blocked_requests << "\n";
    out << "Allowed Requests : " << allowed_requests << "\n";
    out << "Bytes transferred : " << bytes_transferred << "\n";

    if (!top_host.empty())
        out << "Top Requested Host : " << top_host << " - " << top_host_count << "\n";
    else
        out << "Top Requested Host : None\n";

    out << "Requests Per Minute : " << rpm << "\n";
}

void init_metrics(const string &filename)
{
    lock_guard<mutex> lock(m);

    metrics_file = filename;
    start_time = time(nullptr);

    total_requests = 0;
    blocked_requests = 0;
    allowed_requests = 0;
    bytes_transferred = 0;

    host_counts.clear();
    top_host.clear();
    top_host_count = 0;

    flush();
}

void metrics_record_request(const string &host)
{
    lock_guard<mutex> lock(m);

    total_requests++;

    size_t count = ++host_counts[host];
    if (count > top_host_count)
    {
        top_host = host;
        top_host_count = count;
    }

    flush();
}

void metrics_record_blocked()
{
    lock_guard<mutex> lock(m);
    blocked_requests++;
    flush();
}

void metrics_record_allowed(size_t bytes)
{
    lock_guard<mutex> lock(m);
    allowed_requests++;
    bytes_transferred += bytes;
    flush();
}
