#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstring>
#include "client_handler.h"
#include "http_parser.h"
#include "forwarder.h"
#include "blocklist.h"
#include "logger.h"
#include "global_config.h"
#include "metrics.h"

using namespace std;

void handle_client(const Task &task)
{
    HttpRequest req;
    size_t bytes = 0;

    apply_socket_timeout(task.client_fd, global_config.connection_timeout_sec);

    if (!parse_http_request(task.client_fd, req))
    {
        // The request parser fails hence we send response 400 BAD REQUEST

        string body = "Bad Request: unable to parse HTTP request.\n";

        string response = "HTTP/1.0 400 Bad Request\r\n"
                          "Content-Type: text/plain\r\n"
                          "Content-Length: " +
                          to_string(body.size()) + "\r\n"
                                                   "Connection: close\r\n"
                                                   "\r\n" +
                          body;

        send_all(task.client_fd, response.c_str(), response.size());

        metrics_record_blocked();

        log_info(task.client_ip + ":" + to_string(task.client_port) +
                 " | \"INVALID REQUEST\""
                 " | -"
                 " | FAILED | 400 | bytes=0");

        close(task.client_fd);
        return;
    }

    metrics_record_request(req.host);

    string request_line = req.method + " " + req.path + " HTTP/1.0";
    string host_port = req.host + ":" + to_string(req.port);

    if (global_config.enable_blocklist && is_blocked(req.host))
    {
        metrics_record_blocked();
        log_info(task.client_ip + ":" + to_string(task.client_port) +
                 " | \"" + request_line + "\"" +
                 " | " + host_port +
                 " | BLOCKED | 403 | bytes=0");

        string body = "Access to the requested domain is blocked.\n";

        string response = "HTTP/1.0 403 Forbidden\r\n"
                          "Content-Type: text/plain\r\n"
                          "Content-Length: " +
                          to_string(body.size()) + "\r\n"
                                                   "Connection: close\r\n"
                                                   "\r\n" +
                          body;

        send_all(task.client_fd, response.c_str(), response.size()); // 403 Forbidden Response sent
        close(task.client_fd);
        return;
    }

    if (req.method == "CONNECT")
    {
        if (!global_config.enable_https_tunnel)
        {
            metrics_record_blocked();
            string body = "HTTPS tunneling is disabled by server policy.\n";

            string response = "HTTP/1.0 403 Forbidden\r\n"
                              "Content-Type: text/plain\r\n"
                              "Content-Length: " +
                              to_string(body.size()) + "\r\n"
                                                       "Connection: close\r\n"
                                                       "\r\n" +
                              body;

            send_all(task.client_fd, response.c_str(), response.size());
            close(task.client_fd);
            return;
        }

        bytes = tunnel_tcp(task.client_fd, req); // start https tunnelling
    }
    else
    {
        bytes = forward_tcp(task.client_fd, req); // start http forwarding
    }

    metrics_record_allowed(bytes);
    log_info(task.client_ip + ":" + to_string(task.client_port) +
             " | \"" + request_line + "\"" +
             " | " + host_port +
             " | ALLOWED | 200 | bytes=" + to_string(bytes));
}