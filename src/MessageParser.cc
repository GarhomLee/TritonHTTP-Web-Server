#include <sysexits.h>
#include <fstream>
// #include <unordered_map>
#include "MessageParser.hpp"
#include "logger.hpp"

MessageParser::MessageParser(int socketfd, struct config_info &info)
    : clntSocket(socketfd), ci(info) {}

/* receive messages and parse requests from a single connection */
void MessageParser::receive()
{
    auto log = logger();
    char buffer[BUFFER_SIZE];

    /* receive messages and parse all possible requests */
    bool isClosed = false;
    while (!isClosed)
    {
        memset(buffer, 0, BUFFER_SIZE);                           // clear out the receive buffer
        int recvBytes = recv(clntSocket, buffer, BUFFER_SIZE, 0); // get actually received bytes

        if (recvBytes < 0) // error ocurrs at client side
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                log->error("Client timed out. Closing connection.");
            }
            else
            {
                log->error("Unknown error. Closing connection.");
            }

            RequestHandler requestHandler(clntSocket, ci);
            requestHandler.sendFailureResponse(400); // send back a failure response and stop
        }
        else if (recvBytes == 0) // client has closed connection
        {
            log->info("Client has closed connection.");
            close(clntSocket);
            exit(1);
            // return;
        }

        /* transfer chars from a buffer char array to a buffer string */
        for (int i = 0; i < recvBytes; i++)
        {
            receivedMessage += buffer[i];
        }

        parseRequests(isClosed);
        // log->info("isClosed has been change: {}", isClosed);
    }

    // log->info("out of the while loop.");
}

/* process all complete requests */
void MessageParser::parseRequests(bool &isClosed)
{
    RequestHandler requestHandler(clntSocket, ci);
    auto log = logger();
    // log->info("In MessageParse.cc, doc_root=\"{}\"", ci.doc_root);
    int count = 0;
    while (!isClosed && receivedMessage.find("\r\n\r\n") != string::npos)
    {
        count++;
        log->info("Request {} found.", count); // print log

        int endIndex = 4 + receivedMessage.find("\r\n\r\n");
        string request = receivedMessage.substr(0, endIndex); // get the request including 2 CRLFs

        // send(clntSocket, request.c_str(), request.size(), 0);

        receivedMessage.erase(0, endIndex); // remove the first complete request and keep the rest

        // send(clntSocket, receivedMessage.c_str(), receivedMessage.size(), 0);

        isClosed = requestHandler.handle(request); // handle the request
                                                   // send(clntSocket, response.c_str(), response.size(), 0);
    }
}