//chat_header.h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define INADDR_RIP_GROUP   "239.0.0.1"    /* 239.0.0.1 */
#define PORT 9999
#define MAX_MESSAGE_LENGTH 1024
#define MAX_NAME_LENGTH 10
#define YES 1
#define INADDR_SERVER   "0.0.0.0" /*please input your server IP*/
#define MAX_CLIENT 10

typedef struct _Buffer
{
    char name[10];
    char msg[1014];
} Buffer;
