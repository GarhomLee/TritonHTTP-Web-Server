#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

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
#include <fstream>
#include <limits.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/sendfile.h> // Linux version
#include <unordered_map>
#include "logger.hpp"
#include "ResponseBuilder.hpp"

using namespace std;

class RequestHandler
{
public:
    RequestHandler(int socketfd, struct config_info &info);
    void handle(string &request);
    void sendSuccessResponse(string &requestedFile, bool isClosed);
    void sendFailureResponse(int code);

private:
    int clntSocket;
    struct config_info ci;
    //     string receivedMessage;
    //     string request;
    //     string initialLine;
    string parse(string &request, string delimiter);
    int parseInitialLine(string &initialLine, string &requestedFile);
    bool validateKeyValuePair(string &line, string &key, string &value);
};

#endif // REQUESTHANDLER_HPP