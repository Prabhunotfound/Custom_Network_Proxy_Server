#include <arpa/inet.h>
#include <atomic>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "server.h"
#include "thread_pool.h"
#include "task.h"
#include "global_config.h"

using namespace std;

static atomic<bool> running{true};
static int server_fd = -1;

void start_server(int port)
{
    ThreadPool pool(global_config.thread_pool_size); // initialize an object of ThreadPool

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        return;
    }

    if (listen(server_fd, SOMAXCONN) < 0)
    {
        perror("listen");
        close(server_fd);
        return;
    }

    while (running) // start the main server loop
    {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (sockaddr *)&client_addr, &client_len);

        if (client_fd < 0)
        {
            if (!running)
                break;

            if (errno == EINTR)
                continue;

            perror("accept");
            continue;
        }

        char ipbuf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ipbuf, sizeof(ipbuf));

        Task task;
        task.client_fd = client_fd;
        task.client_ip = ipbuf;
        task.client_port = ntohs(client_addr.sin_port);

        pool.enqueue(task); // add the Task to the ThreadPool Object pool
    }

    if (server_fd >= 0)
    {
        close(server_fd);
        server_fd = -1;
    }

    cout << "[INFO] Server stopped accepting connections" << endl;
}

void stop_server()
{
    running = false;

    if (server_fd >= 0)
    {
        close(server_fd);
        server_fd = -1;
    }
}
