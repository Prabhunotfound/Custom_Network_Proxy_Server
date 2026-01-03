#ifndef FORWARDER_H
#define FORWARDER_H

#include <cstddef>
#include "http_parser.h"

size_t forward_tcp(int client_fd, const HttpRequest &req);

size_t tunnel_tcp(int client_fd, const HttpRequest &req);

bool send_all(int fd, const char *buf, size_t len);

#endif
