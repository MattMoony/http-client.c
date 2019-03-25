#include <stdio.h>
#include "http-client.h"

int main()
{
    struct HTTPResponse* res;
    res = get("http://www.google.com");

    printf("HTTPResponse: ");
    printf("%d\n", res->code);
    printf("%s\n", res->contentType);
    printf("%s\n", res->transferEncoding);
    printf("%d\n", res->cookieAmount);
    printf("%s\n", res->cookies[0]);

    return 0;
}
