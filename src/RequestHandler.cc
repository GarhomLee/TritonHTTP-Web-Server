#include <sysexits.h>
#include <fstream>
#include "RequestHandler.hpp"
#include "ResponseBuilder.hpp"

RequestHandler::RequestHandler(int socketfd, struct config_info &info)
    : clntSocket(socketfd), ci(info) {}

/* parse the the first substring separated by the delimiter, and update str */
string RequestHandler::parse(string &str, string delimiter)
{
    int endIndex = str.find(delimiter);

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

    string url = initialLine.substr(urlStart, urlEnd - urlStart);
    if (url.find("/") != 0 || url.find(" ") != string::npos) // url does not start correctly
    {
        log->error("Malformed URL.");
        return 400; // client error
    }
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

/**
 *  check if the given line can form a valid key-value pair, return true if it is valid,
 *  otherwise return false.
 */
bool RequestHandler::validateKeyValuePair(string &line, string &key, string &value)
{
    size_t splitPos = line.find(":");
    if (splitPos == string::npos) // miss the colon
    {
        logger()->error("Missing colon.");
        return false;
    }

    key = line.substr(0, splitPos);
    value = line.substr(splitPos + 1);
    if (key.find(" ") != string::npos) // key cannot contain space
    {
        logger()->error("Not a valid key.");
        return false;
    }
    // logger()->info("check if space at value: {}", secondHalf.find(" ", 1));
    if (value.find(" ") != 0)
    // space not placed correctly in value
    {
        logger()->error("Not a valid value.");
        return false;
    }

    return true;
}

/* handle a request and send back a response to the client */
void RequestHandler::handle(string &request)
{
    auto log = logger();
    int code;
    string initialLine = parse(request, "\r\n"); // get the initial line
    string requestedFile;                        // store the absolute path of the requested file

    /* if the initial line of the request is not valid, send back a response error code */
    if ((code = parseInitialLine(initialLine, requestedFile)) != 200)
    {
        sendFailureResponse(code); // send back a failure response and stop

        if (code == 404) // if it's 404, continue to wait for the next request
        {
            return;
        }
    }

    /* parse key-value pair in each line and check if they are valid */
    // log->info("Starting validation check...");
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

        // log->info("key={}, value={}",key, value);
        headerMapping[key] = value; // put valid key-value pair into hash map
    }

    log->info("Validation check passed.");
    /* validation check passed, prepare for the final response */
    if (headerMapping.count("Host") == 0) // not contain "Host"
    {
        log->error("Request does not contain Host key.");
        sendFailureResponse(400); // send back a failure response and stop
    }

    /* form a response */
    bool isClosed = headerMapping.count("Connection") > 0 && headerMapping["Connection"] == "close";
    sendSuccessResponse(requestedFile, isClosed);
}

/* send back a success response */
void RequestHandler::sendSuccessResponse(string &requestedFile, bool isClosed)
{
    ResponseBuilder rb(ci);
    struct stat file_stat;
    int requestedFd = open(requestedFile.c_str(), O_RDONLY);
    fstat(requestedFd, &file_stat); // get content length and last modified time

    string extenstion = requestedFile.substr(requestedFile.find("."));  // get extension type
    string response = rb.response_200(extenstion, isClosed, file_stat); // create response according to error code
    send(clntSocket, response.c_str(), response.size(), 0);

    off_t offset = 0;
    // if (sendfile(requestedFd, clntSocket, offset, &offset, NULL, 0) < 0)  // OSX version
    if (sendfile(clntSocket, requestedFd, &offset, file_stat.st_size) < 0)  // Linux version
    {
        logger()->error("Failed to send the file.");
        // exit(1);
        return;
    }

    logger()->info("Response to a valid request has sent back.");
}

/* send back a failure response */
void RequestHandler::sendFailureResponse(int code)
{
    ResponseBuilder rb(ci);
    string response = rb.response_error(code); // create response according to error code
    send(clntSocket, response.c_str(), response.size(), 0);
    if (code == 400)
    {
        logger()->error("Invalid request with code {}.\nConnection closed.", code);
        close(clntSocket);
        exit(1); // exit if 400
    }

    logger()->error("Invalid request with code {}.\nWaiting for the next request.", code);
    return; // wait for next request if 404
}
