#ifndef GLOBAL_CONFIG_H
#define GLOBAL_CONFIG_H

#include <sys/socket.h>
#include "config.h"

void apply_socket_timeout(int fd, int timeout_sec);

extern Config global_config;

#endif
