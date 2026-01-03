#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <string>
#include <chrono>
#include "http_parser.h"
#include "global_config.h"

using namespace std;

#define BUFFER_SIZE 4096
#define MAX_HEADER_SIZE 8192

bool parse_http_request(int client_fd, HttpRequest &req)
{
    char buffer[BUFFER_SIZE];
    string data;

    auto start = chrono::steady_clock::now();

    while (true)
    {
        auto now = chrono::steady_clock::now();

        if (chrono::duration_cast<chrono::seconds>(now - start).count() >= global_config.connection_timeout_sec)
        {
            return false;
        }

        ssize_t bytes = recv(client_fd, buffer, BUFFER_SIZE, 0);

        if (bytes > 0)
        {
            data.append(buffer, bytes);
            if (data.size() > MAX_HEADER_SIZE)
                return false;

            // EARLY malformed request-line detection
            size_t line_end = data.find("\r\n");
            if (line_end != string::npos)
            {
                string request_line = data.substr(0, line_end);

                size_t m1 = request_line.find(' ');
                size_t m2 = request_line.find(' ', m1 + 1);

                if (m1 == string::npos || m2 == string::npos)
                    return false;
            }

            // Stop reading once full headers are received
            if (data.find("\r\n\r\n") != string::npos)
                break;

            continue;
        }

        if (bytes == 0)
            return false;

        if (errno == EINTR)
            continue;

        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return false;

        return false;
    }

    req.raw_request = data;

    size_t line_end = data.find("\r\n");
    if (line_end == string::npos)
        return false;

    string request_line = data.substr(0, line_end);
    size_t m1 = request_line.find(' ');
    size_t m2 = request_line.find(' ', m1 + 1);
    if (m1 == string::npos || m2 == string::npos)
        return false;

    req.method = request_line.substr(0, m1);
    string uri = request_line.substr(m1 + 1, m2 - m1 - 1);

    req.port = 80;

    if (req.method == "CONNECT")
    {
        size_t colon = uri.find(':');
        if (colon == string::npos)
            return false;

        req.host = uri.substr(0, colon);
        req.port = stoi(uri.substr(colon + 1));
        return true;
    }

    if (uri.find("http://") == 0)
    {
        string rest = uri.substr(7);
        size_t slash = rest.find('/');
        req.host = (slash == string::npos) ? rest : rest.substr(0, slash);
        req.path = (slash == string::npos) ? "/" : rest.substr(slash);
    }
    else
    {
        // Absolute path
        req.path = uri;

        size_t host_pos = data.find("\r\nHost:");
        if (host_pos == string::npos)
            return false;

        size_t start_h = host_pos + 7;
        size_t end_h = data.find("\r\n", start_h);

        string host_port = data.substr(start_h, end_h - start_h); // get the host name
        while (!host_port.empty() && host_port[0] == ' ')         // erase any spaces if present
            host_port.erase(0, 1);

        size_t colon = host_port.find(':');
        if (colon != string::npos)
        {
            req.host = host_port.substr(0, colon);
            req.port = stoi(host_port.substr(colon + 1)); // get the port after the colon
        }
        else
        {
            req.host = host_port;
        }
    }

    string new_line = req.method + " " + req.path + " HTTP/1.0"; // turning the request into http/1.0
    req.raw_request.replace(0, line_end, new_line);

    return true;
}
