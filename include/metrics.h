#ifndef METRICS_H
#define METRICS_H

#include <string>
#include <cstddef>

using namespace std;

void init_metrics(const string &filename);

void metrics_record_request(const string &host);

void metrics_record_blocked();

void metrics_record_allowed(size_t bytes);

#endif
