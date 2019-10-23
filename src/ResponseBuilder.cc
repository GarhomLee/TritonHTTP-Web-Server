#include <sysexits.h>
#include <fstream>
#include "ResponseBuilder.hpp"

/* constructor */
ResponseBuilder::ResponseBuilder(struct config_info &info) : ci(info)
{
    httpVersion = "HTTP/1.1 ";            // specify http version
    server = "Server: ProjectServer 1.0"; // specify server version
    CRLF = "\r\n";                        // delimiter
}

/* build response for 200 */
string ResponseBuilder::response_200(string &extension, bool isClosed,
                                     struct stat file_stat)
{
    string response = httpVersion + "200 OK" + CRLF;

    response += server + CRLF;

    char time[256] = "";
    strftime(time, 256, "%a, %d %b %y %T %z", localtime(&file_stat.st_mtime));
    response += "Last-Modified: " + string(time) + CRLF;

    response += "Content-Length: " + to_string(file_stat.st_size) + CRLF;

    string type = ci.mime_mapping.count(extension) > 0 ? ci.mime_mapping[extension] : "application/octet-stream";
    response += "Content-Type: " + type + CRLF;

    if (isClosed)
    {
        response += "Connection: close" + CRLF;
    }

    response += CRLF;
    return response;
}

/* build response for error */
string ResponseBuilder::response_error(int errorCode)
{
    string response = httpVersion;

    if (errorCode == 400)
    {
        response += "400 Client Error" + CRLF;
    }
    else if (errorCode == 404)
    {
        response += "404 Not Found" + CRLF;
    }

    response += server + CRLF;
    response += CRLF;
    return response;
}