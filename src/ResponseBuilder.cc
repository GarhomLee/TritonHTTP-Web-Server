#include <sysexits.h>
#include <fstream>
#include "ResponseBuilder.hpp"

/* constructor */
ResponseBuilder::ResponseBuilder(struct config_info &info) : ci(info)
{
    httpVersion = "HTTP/1.1 ";            // specify http version
    server = "Server: ProjectServer 1.0"; // specify server version
    CRLF = "\r\n";                        // delimiter
    // logger()->info("In MessageParse.cc, doc_root=\"{}\"", ci.doc_root);
}

/* build response for error */
string ResponseBuilder::response_error(const int &errorCode)
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
    // if (isClosed)
    // {
    //     response += "Connection: close" + CRLF;
    // }

    response += CRLF;
    return response;
}

// string ResponseBuilder::getDocRoot() {
//     return ci.doc_root;
// }

size_t ResponseBuilder::getMapSize()
{
    return ci.mime_mapping.size();
}