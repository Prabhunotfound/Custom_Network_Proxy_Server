#include <fstream>
#include <iostream>
#include <set>
#include <algorithm>
#include <cctype>
#include "blocklist.h"

using namespace std;

static set<string> blocked_rules; // Contains the blocked domains name in proper format

static void to_lowercase(string &s)
{
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c)
              { return tolower(c); });
}

bool load_blocklist(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "[ERROR] Could not open blocklist file: " << filename << endl;
        return false;
    }

    string line;
    while (getline(file, line))
    {
        // Trim leading whitespace
        line.erase(line.begin(), find_if(line.begin(), line.end(), [](unsigned char c)
                                         { return !isspace(c); }));

        // Trim trailing whitespace
        line.erase(find_if(line.rbegin(), line.rend(), [](unsigned char c)
                           { return !isspace(c); })
                       .base(),
                   line.end());

        if (!line.empty())
        {
            to_lowercase(line); // convert the blocked domain into lowercase
            blocked_rules.insert(line);
        }
    }

    cout << "[INFO] Loaded " << blocked_rules.size() << " blocked domain rules" << endl; // log to the terminal

    return true;
}

bool is_blocked(const string &host)
{
    string h = host;
    to_lowercase(h);

    for (const auto &rule : blocked_rules)
    {
        if (rule == h)
            return true;

        string suffix = "." + rule;
        if (h.size() > suffix.size() && h.compare(h.size() - suffix.size(), suffix.size(), suffix) == 0)
        {
            return true;
        }
    }

    return false;
}