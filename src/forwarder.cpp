#include <unistd.h>
#include <cstring>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include "global_config.h"
#include "forwarder.h"

using namespace std;

#define BUFFER_SIZE 4096

bool send_all(int fd, const char *buf, size_t len)
{
    size_t total_sent = 0;

    while (total_sent < len)
    {
        ssize_t sent = send(fd, buf + total_sent, len - total_sent, 0);

        if (sent > 0)
        {
            total_sent += sent;
            continue;
        }

        if (sent < 0 && errno == EINTR)
            continue;

        if (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            return false;

        return false;
    }

    return true;
}

void apply_socket_timeout(int fd, int timeout_sec) // Applying the timeout
{
    timeval tv{};
    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;

    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

size_t forward_tcp(int client_fd, const HttpRequest &req)
{
    size_t total_bytes = 0;

    apply_socket_timeout(client_fd, global_config.connection_timeout_sec);

    addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(req.host.c_str(), to_string(req.port).c_str(), &hints, &res) != 0)
    {
        close(client_fd);
        return total_bytes;
    }

    int server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (server_fd < 0)
    {
        freeaddrinfo(res);
        close(client_fd);
        return total_bytes;
    }

    apply_socket_timeout(server_fd, global_config.connection_timeout_sec);

    if (connect(server_fd, res->ai_addr, res->ai_addrlen) < 0)
    {
        freeaddrinfo(res);
        close(server_fd);
        close(client_fd);
        return total_bytes;
    }

    freeaddrinfo(res);

    if (!send_all(server_fd, req.raw_request.c_str(), req.raw_request.size()))
    {
        close(server_fd);
        close(client_fd);
        return total_bytes;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes;

    while ((bytes = recv(server_fd, buffer, BUFFER_SIZE, 0)) > 0)
    {
        if (!send_all(client_fd, buffer, bytes))
            break;

        total_bytes += bytes;
    }

    close(server_fd);
    close(client_fd);
    return total_bytes;
}

size_t tunnel_tcp(int client_fd, const HttpRequest &req)
{
    size_t total_bytes = 0;

    apply_socket_timeout(client_fd, global_config.connection_timeout_sec);

    addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(req.host.c_str(),
                    to_string(req.port).c_str(),
                    &hints,
                    &res) != 0)
    {
        close(client_fd);
        return total_bytes;
    }

    int server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (server_fd < 0)
    {
        freeaddrinfo(res);
        close(client_fd);
        return total_bytes;
    }

    apply_socket_timeout(server_fd, global_config.connection_timeout_sec);

    if (connect(server_fd, res->ai_addr, res->ai_addrlen) < 0)
    {
        freeaddrinfo(res);
        close(server_fd);
        close(client_fd);
        return total_bytes;
    }

    freeaddrinfo(res);

    const char *resp =
        "HTTP/1.0 200 Connection Established\r\n\r\n";

    send_all(client_fd, resp, strlen(resp));

    fd_set fds;
    char buffer[BUFFER_SIZE];

    while (true)
    {
        FD_ZERO(&fds);
        FD_SET(client_fd, &fds);
        FD_SET(server_fd, &fds);

        int max_fd = max(client_fd, server_fd) + 1;

        timeval tv{};
        tv.tv_sec = global_config.connection_timeout_sec;
        tv.tv_usec = 0;

        if (select(max_fd, &fds, NULL, NULL, &tv) <= 0)
            break;

        if (FD_ISSET(client_fd, &fds))
        {
            ssize_t n = recv(client_fd, buffer, sizeof(buffer), 0);
            if (n <= 0)
                break;
            if (!send_all(server_fd, buffer, n))
                break;

            total_bytes += n;
        }

        if (FD_ISSET(server_fd, &fds))
        {
            ssize_t n = recv(server_fd, buffer, sizeof(buffer), 0);
            if (n <= 0)
                break;
            if (!send_all(client_fd, buffer, n))
                break;

            total_bytes += n;
        }
    }

    close(server_fd);
    close(client_fd);
    return total_bytes;
}
