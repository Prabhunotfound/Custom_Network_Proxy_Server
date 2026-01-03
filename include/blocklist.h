#ifndef BLOCKLIST_H
#define BLOCKLIST_H

#include <string>

using namespace std;

bool load_blocklist(const string &filename);

bool is_blocked(const string &host);

#endif
