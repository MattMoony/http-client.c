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
    char* asdf = host_from_url(url);
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
void echo(char* some) {
    puts(some);
}

char* get(char* url) {
    char** remoteURLParts = parseURL(url);

    printf("%s - %s\n", remoteURLParts[0], remoteURLParts[1]);
    printf("%s - %s\n", parseURL(url)[0], parseURL(url)[1]);

    WSADATA wsaData;
    SOCKET s = createSocket(&wsaData, parseURL(url)[0]);

    printf("connected!\n");
}
