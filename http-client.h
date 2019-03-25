#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <winsock2.h>

/**
 * @brief   The "HTTPResponse" struct contains information about
 *          the server's response to an HTTPRequest.
 */

struct HTTPResponse {
    int code;

    char* accessControlAllowOrigin;
    int age;
    char* connection;
    char* contentEncoding;
    char* contentType;
    int cookieAmount;
    char** cookies;
    char* date;
    char* etag;
    char* keepAlive;
    char* lastModified;
    char* location;
    char* server;
    char* transferEncoding;
    char* vary;
    char* xBackendServer;
    char* xCacheInfo;
    char* xkumarevision;
    char* xframeoptions;

    char* body;
};

char* ip_from_hostname(char*);
char* host_from_url(char*);
char* path_from_url(char*);

char** parseURL(char*);
SOCKET createSocket(WSADATA*, char*);
char* createHTTPRequest(char*, char*, char*);

struct HTTPResponse* defaultResponse();
struct HTTPResponse* get(char* url);

#endif // HTTPCLIENT_H
