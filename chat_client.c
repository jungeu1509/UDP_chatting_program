//chat_client.c
#include "chat_header.h"

int recv_s, send_s, recv_len, from_len, ID_len;
struct sockaddr_in server_addr, from, my_addr;
struct ip_mreq mreq;

unsigned int yes = YES;
char buffer[MAX_MESSAGE_LENGTH];
char message[MAX_MESSAGE_LENGTH];
Buffer send_msg;
char name[10];

pthread_t tid[1];
int thread_state[1];
pthread_mutex_t Mutex_msg = PTHREAD_MUTEX_INITIALIZER;

void *thread_0(void *arg);
int id_check_f(char buf);

int main(int argc, char **argv)
{
    memset(name, 0, sizeof(name));
    if (argc != 2)
    {
        printf("usage: %s <YOUR_ID>\n", argv[0]);
        printf("EX) %s 123\n", argv[0]);
        exit(1);
    }
    ID_len = strlen(argv[1]);
    if((ID_len>9)||(ID_len<1))
    {
        printf("The ID must be between 1 and 9 characters long.\n");
        printf("EX) %s 123\n", argv[0]);
        exit(1);
    }
    sprintf(name,"%s",argv[1]);
    //printf("Your ID is : ");
    for(int i=0;i<ID_len;i++)
    {
        if(( ((name[i]>='0')&&(name[i]<='9')) || ((name[i]>='a')&&(name[i]<='z')) || ((name[i]>='A')&&(name[i]<='Z')) ) == 0 )
        {
            printf("\nerror!\n");
            printf("usage: %s <YOUR_ID>\n", argv[0]);
            printf("ID can only be English and numbers.\n");
            printf("EX) %s 123\n", argv[0]);
            exit(1);
        }
    }
    printf("\n");
    for(int i=ID_len; i<MAX_NAME_LENGTH; i++)
    {
        name[i]='-';
    }
    //printf("save id %s\n", name);

    //create socket
    if((recv_s = socket(AF_INET, SOCK_DGRAM, 0))<0){
        perror("socket ");
        return 1;
    }

    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port = htons(PORT);

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(INADDR_SERVER);
    server_addr.sin_port = htons(PORT);

    // Multicast Socket
    mreq.imr_multiaddr.s_addr = inet_addr(INADDR_RIP_GROUP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(recv_s, IPPROTO_IP, IP_ADD_MEMBERSHIP,&mreq, sizeof(mreq)) < 0) {
        perror("setting interface");
        exit(1);
    }

    if (setsockopt(recv_s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        perror("setting reuseaddr");
        exit(1);
    }

    if((send_s = socket(AF_INET, SOCK_DGRAM, 0))<0){
        perror("socket ");
        return 1;
    }

    if(bind(recv_s, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0){
        perror("bind ");
        return 1;
    }
    memset(buffer, '\0', sizeof(buffer));
    sprintf(buffer,"%s%s", name, "!new!client!");
    if(sendto(send_s, buffer, strlen(buffer)+1, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < strlen(buffer)){
        perror("sendto ");
        exit(0);
    }
    //printf("send to server %s\n", buffer);
    memset(buffer, '\0', sizeof(buffer));
    recvfrom(recv_s, message, MAX_MESSAGE_LENGTH, 0, (struct sockaddr*)&from, &from_len);
    if(strstr(message, "IDer") != NULL){
        printf("error! please try again\n");
        int error_code;
        error_code = id_check_f(message[4]);
        printf("error code : %d\n", error_code);
        if(error_code == 1)
            return 1;
        else if(error_code == 2)
            return 2;
    }
    else if(strstr(message,"!Welcome chat program! [ip-") != NULL)
    {
        memset(message, 0, sizeof(buffer));
    }
    else{
        printf("%s\n", message);
    }

    thread_state[0] = pthread_create(&tid[0], NULL, thread_0, (void *)1);
    if(thread_state[0] != 0){
        perror("thread_0 create error\n");
        exit(1);
    }

    while(1)
    {
        from_len = sizeof(from);
        recv_len = recvfrom(recv_s, message, MAX_MESSAGE_LENGTH, 0, (struct sockaddr*)&from, &from_len);
/*
        if(strstr(message,"!Welcome chat program! [ip-") == NULL)
        {
            for(int i =0;i < 48;i++)
            {
                if(i%16)
                    printf("%02x ",message[i]);
                else
                    printf("\n0x%04x :\t%02x ",i,message[i]);

            }
        }
*/
        if(strstr(message, "IDer") != NULL){
            printf("error! please try again\n");
            id_check_f(message[4]);
        }
        else if(strstr(message,"!Welcome chat program! [ip-") != NULL)
        {
            memset(message, 0, sizeof(message));
        }
        else if(recv_len > 0)
        {
            printf("new message : \n%s\n", message);
        }
    }
    printf("end program");
    return 0;
}

void *thread_0(void *arg)
{
    while(1)
    {
        printf("Enter the username you want to chat with : ");
        fgets(send_msg.name, (MAX_NAME_LENGTH-1), stdin);
        printf("Enter message :");
        fgets(send_msg.msg, (MAX_MESSAGE_LENGTH-MAX_NAME_LENGTH-1), stdin);
        for(int i=(strlen(send_msg.name)-1); i<(MAX_NAME_LENGTH); i++)
        {
            send_msg.name[i]='-';
        }
        pthread_mutex_lock(&Mutex_msg);
        memset(buffer, 0, sizeof(buffer));
        memcpy(buffer, &send_msg, MAX_MESSAGE_LENGTH);
        if(sendto(send_s, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < strlen(buffer)){
            perror("sendto ");
            exit(0);
        }
        printf("send : %s\n", buffer);
        pthread_mutex_unlock(&Mutex_msg);
        sleep(1);
    }
}

int id_check_f(char buf)
{
    if(buf == 'A')
    {
        printf("ID already exists\n");
        return 1;
    }
    else if(buf == 'F')
    {
        printf("server is full\n");
        return 2;
    }
    else if(buf == 'M')
    {
        printf("ID does not exist\n");
        return 3;
    }
    return 0;
}
