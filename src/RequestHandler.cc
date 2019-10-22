#include <sysexits.h>
#include <fstream>
#include "RequestHandler.hpp"
#include "ResponseBuilder.hpp"

RequestHandler::RequestHandler(int socketfd, struct config_info &info)
    : clntSocket(socketfd), ci(info) {}

/* parse the the first substring separated by the delimiter, and update str */
string RequestHandler::parse(string &str, string delimiter)
{
    // auto log  = logger();
    int endIndex = str.find(delimiter);
    // log->info("length of delimiter is: {}", delimiter.length());

    string substring = endIndex == 0 ? "" : str.substr(0, endIndex); // get the first part separated by
                                                                     // the delimiter (excluded)

    str.erase(0, endIndex + delimiter.length()); // get rid of the first part as well as the delimiter

    return substring; // if no substring before the delimiter, it returns empty string
}

/**
*   parse the initial line of the request, return 200 with the requested file if it is valid,
*   or 400 if malformed, or 404 if escaped or file not found
*/
int RequestHandler::parseInitialLine(string &initialLine, string &requestedFile)
{
    auto log = logger();
    if (initialLine.length() < 14) // the minimum should be "GET / HTTP/1.1"
    {
        log->error("Missing info in the initial line.");
        return 400; // client error
    }

    size_t urlStart = 4;                      // start after "GET ", inclusive
    size_t urlEnd = initialLine.length() - 9; // end at " HTTP/1.1", exclusive

    if (initialLine.find("GET ") != 0) // request does not start with GET
    {
        log->error("Malformed http method.");
        return 400; // client error
    }

    if (initialLine.find(" HTTP/1.1") != urlEnd) // request does not end validly
    {
        log->error("Malformed http version.");
        return 400; // client error
    }

    if (initialLine.find("/") != urlStart) // url does not start correctly
    {
        log->error("URL does not start with \"/\".");
        return 400; // client error
    }

    string url = initialLine.substr(urlStart, urlEnd - urlStart);
    string concate_path = ci.doc_root + url; // concatenate to form a raw path
    // log->info("URL found: \"{}\"", url);
    // log->info("ci.doc_root: \"{}\"", ci.doc_root);
    // log->info("concate_path: \"{}\"", concate_path);

    char pathBuffer[516];
    if (realpath(concate_path.c_str(), pathBuffer) == NULL) // raw path cannot be converted
    {
        log->error("Failed to convert the url to an absolute path. \n{}", strerror(errno));
        return 404; // not found
    }

    requestedFile = string(pathBuffer);
    // string absolutePath(pathBuffer); // store the converted absolute path
    // log->info("Absolute path of concate_path: \"{}\"", absolutePath);
    if (requestedFile.find(ci.doc_root) != 0) // root directory is not in the path
    {
        log->error("URL escaped.");
        return 404; // not found
    }

    size_t lastSlash = requestedFile.find_last_of("/\\");
    string lastFile = requestedFile.substr(lastSlash + 1);
    // log->info("lastFile: \"{}\"", lastFile);
    if (lastFile.find(".") == string::npos) // it is a directory, route to the index website
                                            // by appending "/index.html"
    {
        requestedFile += "/index.html";
        fstream fs;
        fs.open(requestedFile);
        if (!fs.is_open()) // index website does not exist
        {
            log->error("Failed to find index.html in the given directory.");
            return 404; // not found
        }
    }

    return 200; // ok
}

/* check if the given line can form a valid key-value pair */
bool RequestHandler::validateKeyValuePair(string &line, string &key, string &value)
{
    return false;
}

/* handle a request and send back a response to the client */
void RequestHandler::handle(string &request)
{
    auto log = logger();
    // ResponseBuilder rb(ci);
    // log->info("In RequestHandler.cc, doc_root=\"{}\"", ci.doc_root);
    string response;
    int code;

    // string requestCopy = request;  // create a copy of the request
    string initialLine = parse(request, "\r\n"); // get the initial line
    string requestedFile;                        // store the absolute path of the requested file

    /* if the initial line of the request is not valid, send back a response error code */
    if ((code = parseInitialLine(initialLine, requestedFile)) != 200)
    {
        sendFailureResponse(code); // send back a failure response and stop
    }

    /* parse key-value pair in each line */
    unordered_map<string, string> headerMapping;
    string key, value;
    for (string line = parse(request, "\r\n");
         !line.empty();
         line = parse(request, "\r\n"))
    {
        if (!validateKeyValuePair(line, key, value)) // not a valid key-value pair
        {
            log->error("Request contains invalid key-value pair line.");
            sendFailureResponse(400); // send back a failure response and stop
        }

        if (headerMapping.count(key) > 0)  // duplicate found
        {
            log->error("Request contains duplicate keys.");
            sendFailureResponse(400); // send back a failure response and stop
        }
    }

    // log->info("the requested file is: \"{}\"", requestedFile);
    // send(clntSocket, request.c_str(), request.size(), 0);
    // send(clntSocket, initialLine.c_str(), initialLine.size(), 0);

    // response = rb.response_error(404, false);
    // log->info("test config info: map size = {}", rb.getMapSize());
    // log->info("test config info: doc_root = {}", rb.getDocRoot());
    // send(clntSocket, response.c_str(), response.size(), 0);
    // return "done.\n";
}

void RequestHandler::sendFailureResponse(int code)
{
    ResponseBuilder rb(ci);
    string response = rb.response_error(code); // create response according to error code
    send(clntSocket, response.c_str(), response.size(), 0);
    logger()->error("Invalid request with code {}.\nConnection closed.", code);
    close(clntSocket);
    exit(1);
}
