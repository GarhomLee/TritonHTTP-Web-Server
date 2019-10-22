#ifndef MESSAGEPARSER_HPP
#define MESSAGEPARSER_HPP

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
#include "logger.hpp"
#include "RequestHandler.hpp"
#include "ResponseBuilder.hpp"

#define BUFFER_SIZE 256

using namespace std;

class MessageParser {
    public:
        MessageParser(int socketfd, struct config_info& info);
        // string getRequest();
        void receive();
        void parseRequests();
    private:
        int clntSocket;
        string receivedMessage;
        // string request;
        struct config_info ci;
};

#endif // MESSAGEPARSER_HPP