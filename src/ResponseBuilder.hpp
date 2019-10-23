#ifndef RESPONSEBUILDER_HPP
#define RESPONSEBUILDER_HPP

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sstream>
// #include <unordered_map>
// #include "inih/INIReader.h"
#include "logger.hpp"

using namespace std;

struct config_info
{
    string port;
    string doc_root;
    unordered_map<string, string> mime_mapping;
};

class ResponseBuilder
{
public:
    ResponseBuilder(struct config_info &info);
    string response_200(string &extension, bool isClosed,
                        struct stat file_stat);
    string response_error(int errorCode);

private:
    string httpVersion;
    string server;
    string CRLF;
    struct config_info ci;
};

#endif // RESPONSEBUILDER_HPP