#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <string>

using namespace std;

struct HttpRequest
{
    string method;
    string host;
    string path;
    string raw_request;
    int port;
};

bool parse_http_request(int client_fd, HttpRequest &req);

#endif
