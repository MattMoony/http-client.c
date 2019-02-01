#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

char *host_from_url(char* url) {
    if (strlen(url) < 7)
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

char *path_from_url(char* url) {
    int startIndex;
    if (strlen(url) < 7)
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

    if (i >= strlen(url)) {
        char *path = malloc(sizeof(char)*2);
        path[0] = '/';
        path[1] = '\0';

        return path;
    } else {
        startIndex = i;
        char *path = malloc(sizeof(char)*(strlen(url)-startIndex+1));

        for (i; url[i] != '\0'; i++)
            path[i-startIndex] = url[i];
        path[strlen(url)-startIndex] = '\0';

        return path;
    }
}



char **parseURL(char* url) {
    if (strlen(url) < 7)
        return 1;

    char** ret = malloc(2*sizeof(char*));
    ret[0] = host_from_url(url);
    ret[1] = path_from_url(url);

    return ret;
}

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

char* createHTTPRequest(char* method, char* hostname, char* path) {
    char* req = malloc(sizeof(char));
    req[0] = '\0';

    strcpy(req, method);
    strcat(req, " ");
    strcat(req, path);
    strcat(req, " ");
    strcat(req, "HTTP/1.1\r\nHost: ");
    strcat(req, hostname);
    strcat(req, "\r\nUser-Agent: CClient\r\n\r\n");

    return req;
}

char* get(char* url) {
    // -- GET HOSTNAME & PATH -- //
    char** remoteURLParts = parseURL(url);

    WSADATA wsaData;
    SOCKET s = createSocket(&wsaData, remoteURLParts[0]);

    char *resp = malloc(sizeof(char)), *request = createHTTPRequest("GET", remoteURLParts[0], remoteURLParts[1]);
    int recvBuffLen = 2048,
            totalLength = 0;
    char recvBuff[recvBuffLen];

    resp[0] = '\0';

    int iResult = send(s, request, (int) strlen(request), 0);
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
            printf("Bytes received: %d\n", iResult);
            strcat(resp, recvBuff);
        } else if (iResult == 0) {
            break;
        } else {
            // printf("Recv failed: %d\n", WSAGetLastError());
            break;
        }
    } while (iResult > 0);

    int bodyStart = -1;
    for (int i = 0; i < totalLength-4; i++) {
        if (resp[i] == '\r'
                && resp[i+1] == '\n'
                && resp[i+2] == '\r'
                && resp[i+3] == '\n') {
            bodyStart = i+4;
            break;
        }
    }

    if (bodyStart < 0) {
        printf("ERROR: NO BODY!");
        return "";
    }

    int x = -1;
    while (resp[++x]!='\0') {}

    char body[totalLength-bodyStart+1];
    for (int i = bodyStart; i < totalLength; i++) {
        body[i-bodyStart] = resp[i];
    }
    body[totalLength-bodyStart] = '\0';

    printf("%s\n", body);
}
