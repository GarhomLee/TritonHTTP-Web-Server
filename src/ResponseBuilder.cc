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
string ResponseBuilder::response_200(int code, string &requestedFile, bool isClosed)
{
    string response = httpVersion + "200 OK" + CRLF;
    response += server + CRLF;
    if (isClosed) {
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

// string ResponseBuilder::getDocRoot() {
//     return ci.doc_root;
// }

// size_t ResponseBuilder::getMapSize()
// {
//     return ci.mime_mapping.size();
// }