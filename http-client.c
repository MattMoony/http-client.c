#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>

#include "http-client.h"
#include "libs/propaganda.h"

/**
 * @brief   The "ip_from_hostname" function resolves a given
 *          hostname to an IP-Address.
 * @param   (char*) host_name: The hostname
 * @return  (char*) the IP Address
 */

char* ip_from_hostname(char* host_name) {
    WSADATA wsaData;

    struct hostent *remoteHost;
    struct in_addr addr;

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    remoteHost = gethostbyname(host_name);

    if (remoteHost == NULL) {
        printf("Error : %d\n", WSAGetLastError());
        return 1;
    }

    addr.s_addr = *(u_long *) remoteHost->h_addr_list[0];
    return inet_ntoa(addr);
}

/**
 * @brief   The "host_from_url" function gets the hostname from
 *          the specified URL.
 * @param   (char*) url: the URL
 * @return  (char*) the hostname
 */

char *host_from_url(char* url) {
    if (pstrlen(url) < 7)
        return 1;

    char prefix[8];
    for (int i = 0; i < 7; i++)
        prefix[i] = url[i];
    prefix[8] = '\0';

    int startIndex;

    if (strcmp(prefix, "https:/")==0) {
        startIndex = 8;
    } else if (strcmp(prefix, "http://") == 0) {
        startIndex = 7;
    } else {
        return 1;
    }

    int i;

    char buff[256];
    for (i = startIndex; url[i] != '\0' && url[i] != '/'; i++)
        buff[i-startIndex] = url[i];

    char *host_name = malloc(sizeof(char)*(i-startIndex+1));
    for (int j = 0; j < i-startIndex; j++)
        host_name[j] = buff[j];
    host_name[i-startIndex] = '\0';

    return host_name;
}

/**
 * @brief   The "path_from_url" function gets the path
 *          of a URL.
 * @param   (char*) url: the URL
 * @return  (char*) the path
 */

char *path_from_url(char* url) {
    int startIndex;
    if (pstrlen(url) < 7)
        return 1;

    char prefix[8];
    for (int i = 0; i < 7; i++)
        prefix[i] = url[i];
    prefix[8] = '\0';

    if (strcmp(prefix, "https:/")==0) {
        startIndex = 8;
    } else if (strcmp(prefix, "http://") == 0) {
        startIndex = 7;
    } else {
        return 1;
    }


    int i = startIndex;
    while (url[i] != '/' && url[i++] != '\0') {}

    if (i >= pstrlen(url)) {
        char *path = malloc(sizeof(char)*2);
        path[0] = '/';
        path[1] = '\0';

        return path;
    } else {
        startIndex = i;
        char *path = malloc(sizeof(char)*(pstrlen(url)-startIndex+1));

        for (i; url[i] != '\0'; i++)
            path[i-startIndex] = url[i];
        path[pstrlen(url)-startIndex] = '\0';

        return path;
    }
}

/**
 * @brief   Returns both hostname and path of a given URL.
 * @param   (char*) url: the URL
 * @return  (char**) [hostname, path]
 */

char **parseURL(char* url) {
    if (pstrlen(url) < 7)
        return 1;

    char** ret = malloc(2*sizeof(char*));
    ret[0] = host_from_url(url);
    ret[1] = path_from_url(url);

    return ret;
}

/**
 * @brief   The function "createSocket" creates a winsocket.
 * @param   (WSADATA*) wsaData: The WSADATA reference
 * @param   (char[]) remote_host_name: the remote host's name
 * @return  (SOCKET) the created win-socket
 */

SOCKET createSocket(WSADATA* wsaData, char remote_host_name[]) {
    SOCKET s;
    struct sockaddr_in server;


    int iRes = WSAStartup(MAKEWORD(2, 2), wsaData);

    if (iRes != 0) {
        printf("WSAStartup failed: %d\n", iRes);
        return 1;
    }



    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
    }

    puts(remote_host_name);
    printf("IP of \"%s\": %s\n", remote_host_name, ip_from_hostname(remote_host_name));

    server.sin_addr.s_addr = inet_addr(ip_from_hostname(remote_host_name));
    server.sin_family = AF_INET;
    server.sin_port = htons(80);

    // Connect to remote server ...
    if (connect(s, (struct sockaddr * )&server, sizeof(server)) < 0) {
        puts("connect error ... ");
        return 1;
    }

    return s;
}

/**
 * @brief   Creates a default HTTPRequest with the given parameters.
 * @param   (char*)   method: "GET", "POST", "DELETE", ...
 * @param   (char*) hostname: The target's hostname
 * @param   (char*)     path: The target path
 * @return  (char*) the created HTTPRequest
 */

char* createHTTPRequest(char* method, char* hostname, char* path) {
    char* req = newstr(pstrcat(method, " "));

    req = pstrcat(req, pstrcat(path, " "));
    req = pstrcat(req, "HTTP/1.1\r\n");
    req = pstrcat(req, pstrcat("Host: ", hostname));
    req = pstrcat(req, "\r\nUser-Agent: CClient\r\n\r\n");

    return req;
}

/**
 * @brief   Creates a default response.
 * @return  (struct HTTPResponse*) the default response
 */

struct HTTPResponse* defaultResponse() {
    struct HTTPResponse* res = (struct HTTPResponse*) malloc(sizeof(struct HTTPResponse));
    res->code = 404;

    res->accessControlAllowOrigin = "";
    res->age = 0;
    res->connection = "";
    res->contentType = "text/plain";
    res->cookieAmount = 0;
    res->cookies = NULL;
    res->transferEncoding = "identity";

    res->body = newstr("");

    return res;
}

/**
 * @brief   The "get" function makes an HTTPRequest to
 *          the given host, and requests the given path.
 * @param   (char*) url: the wanted URL
 * @return  (struct HTTPResponse*) the host's response
 */

struct HTTPResponse* get(char* url) {
    // -- GET HOSTNAME & PATH -- //
    char** remoteURLParts = parseURL(url);

    WSADATA wsaData;
    SOCKET s = createSocket(&wsaData, remoteURLParts[0]);

    char *resp = newstr(""),
            *request = createHTTPRequest("GET", remoteURLParts[0], remoteURLParts[1]);
    int recvBuffLen = 64,
            totalLength = 0;
    char recvBuff[recvBuffLen];

    int iResult = send(s, request, (int) pstrlen(request), 0);
    if (iResult == SOCKET_ERROR) {
        printf("Send failed: %d\n", WSAGetLastError());

        closesocket(s);
        WSACleanup();
        return "";
    }

    do {
        iResult = recv(s, recvBuff, recvBuffLen, 0);

        if (iResult > 0) {
            totalLength += (int) iResult;
            // printf("Bytes received: %d\n", iResult);
            resp = pstrcat(resp, recvBuff);

            if (stringOccurrence(resp, "\r\n\r\n") > 0) {
                break;
            }
        } else if (iResult == 0) {
            break;
        } else {
            printf("Recv failed: %d\n", WSAGetLastError());
            break;
        }
    } while (iResult > 0);

    int header_end_i = indexOfString(resp, "\r\n\r\n");

    char* f_recv_part = substr(resp, header_end_i+5);
    char* f_header = substring(resp, 0, header_end_i);

    char** h_parts = splits(f_header, "\r\n");
    int h_p_amount = stringOccurrence(f_header, "\r\n");

    struct HTTPResponse* httpRes;
    httpRes = (struct HTTPResponse*) malloc(sizeof(struct HTTPResponse));
    char* cookies = newstr("");

    // Skip first header line ("HTTP/1.1 200 OK", etc.)
    for (int i = 0; i < h_p_amount+1; i++) {
        char** l_part = split(h_parts[i], ' ');
        char* h_name = substring(l_part[0], 0, strlen(l_part[0])-1);

        if (strlen(h_name) >= 5 && strequals("HTTP/", substring(l_part[0], 0, 5))) {
            httpRes->code = stoi(l_part[1]);
        } else if (strequals(h_name, "Content-Type")) {
            httpRes->contentType = l_part[1];
            replace(httpRes->contentType, ';', '\0');
        } else if (strequals(h_name, "Transfer-Encoding")) {
            httpRes->transferEncoding = l_part[1];
        } else if (strequals(h_name, "Set-Cookie")) {
            cookies = strcat(cookies, strcat(l_part[1], " "));
        }
    }

    httpRes->cookieAmount = charOccurrence(cookies, ' ')+1;
    httpRes->cookies = split(cookies, ' ');



    return httpRes;
}
