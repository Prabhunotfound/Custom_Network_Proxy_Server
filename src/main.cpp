#include <iostream>
#include <csignal>
#include <atomic>
#include "metrics.h"
#include "blocklist.h"
#include "logger.h"
#include "config.h"
#include "global_config.h"
#include "server.h"

atomic<bool> shutting_down(false);

void handle_signal(int) // Shutdown Initiator
{
    shutting_down.store(true);
    stop_server();
    cout << "\n[INFO] Graceful shutdown initiated..." << endl;
}

int main()
{
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    if (!load_config("config/proxy.conf", global_config)) // load config file
        return 1;

    if (!validate(global_config)) // validate config file entries
        return 1;

    if (global_config.enable_blocklist)
    {
        if (!load_blocklist(global_config.blocklist_file)) // loading the blocklist file
            return 1;
    }

    init_logger(global_config.log_file, global_config.log_max_size_bytes); // initialize Log file
    init_metrics(global_config.metrics_file);                              // initialize Metrics file

    cout << "[INFO] Starting Proxy Server on " << global_config.listen_address << ":" << global_config.listen_port << endl;
    log_info("Starting Proxy Server on " + global_config.listen_address + ":" + to_string(global_config.listen_port));

    start_server(global_config.listen_port); // Start the server

    log_info("Proxy Server stopped cleanly");

    close_logger(); // close the log file cleanly after shutdown is initiated

    cout << "[INFO] Proxy Server stopped cleanly" << endl;
    return 0;
}
