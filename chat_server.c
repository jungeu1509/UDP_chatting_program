//chat_server.c
#include "chat_header.h"

char buffer[MAX_MESSAGE_LENGTH];
char client_ID[MAX_CLIENT][MAX_NAME_LENGTH];
int client_num = 0;
int recv_s, send_group_s, send_s;
int len, addr_len, recv_len;
unsigned int yes = YES;
char message[MAX_MESSAGE_LENGTH];

struct sockaddr_in group_addr;
struct sockaddr_in client_addr[MAX_CLIENT];
struct sockaddr_in server_addr;

pthread_t tid[1];
int thread_state[1];
pthread_mutex_t Mutex_msg = PTHREAD_MUTEX_INITIALIZER;

void *thread_0(void *arg);

int main(void)
{
    int i;
    //create socket
    if((recv_s = socket(AF_INET, SOCK_DGRAM, 0))<0){
        perror("socket ");
        return 1;
    }

    //bzero(&group_addr,sizeof(group_addr));
    memset(&group_addr, 0x00, sizeof(group_addr));
    group_addr.sin_family = AF_INET; //IPv4 protocol
    group_addr.sin_addr.s_addr = inet_addr(INADDR_RIP_GROUP); // 32bit IPv4 addr
    group_addr.sin_port = htons(PORT);  //use port number

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (setsockopt(recv_s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
      perror("setting reuseaddr");
      exit(1);
    }

    if((send_s = socket(AF_INET, SOCK_DGRAM, 0))<0){
        perror("socket ");
        return 1;
    }
    if((send_group_s = socket(AF_INET, SOCK_DGRAM, 0))<0){
        perror("socket ");
        return 1;
    }

    if(bind(recv_s, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("bind recv");
        return 1;
    }


    if(setsockopt(send_group_s, IPPROTO_IP, IP_MULTICAST_IF, (char *)&server_addr, sizeof(server_addr)) < 0){
        perror("setsockopt");
    }

    thread_state[0] = pthread_create(&tid[0], NULL, thread_0, (void *)1);
    if(thread_state[0] != 0){
      perror("thread_0 create error\n");
      exit(1);
    }

    while(1)
    {
        if(i<0xf){i++;}
        else{i=0;}
        pthread_mutex_lock(&Mutex_msg);
        memset(buffer, '\0', sizeof(buffer));
        sprintf(buffer, "!Welcome chat program! [ip-%s / time-%x]", inet_ntoa(server_addr.sin_addr), i);
        if(sendto(send_group_s, buffer, strlen(buffer)+1, 0, (struct sockaddr *)&group_addr, sizeof(group_addr)) < strlen(buffer)){
            perror("sendto ");
            exit(0);
        }
        //printf("send to group : %s\n", buffer);
        pthread_mutex_unlock(&Mutex_msg);
        sleep(5);
    }
    close(recv_s);
    close(send_s);
    pthread_join(tid[0], NULL);
    return 0;
}

void *thread_0(void *arg)
{
    struct sockaddr_in recv_from;
    Buffer recv_msg;
    int check_id, from_ip, sender_id, temp;

    for(int i=0; i<MAX_CLIENT; i++)
    {
        client_addr[i].sin_family = AF_INET;
        client_addr[i].sin_port = htons(PORT);
    }

    while(1)
    {
        //memset(recv_msg.name, '\0', sizeof(recv_msg.name));
        //memset(recv_msg.msg, '\0', sizeof(recv_msg.msg));
        memset(&recv_msg, 0, sizeof(recv_msg));
        memset(message, 0, sizeof(message));

        len = sizeof(recv_from);
        if((recvfrom(recv_s, message, (MAX_MESSAGE_LENGTH-1), 0, (struct sockaddr*)&recv_from, &len))<0)
        {
            printf("recv error");
            exit(1);
        }
        recv_from.sin_port = htons(PORT);
        //printf("\nrecv raw message : %s\n", message);
        //strncpy(recv_msg.name, message, MAX_NAME_LENGTH); //save ID
        //strncpy(recv_msg.msg, &message[MAX_NAME_LENGTH], MAX_MESSAGE_LENGTH-MAX_NAME_LENGTH); //save msg
        memcpy(&recv_msg, message, MAX_MESSAGE_LENGTH);

#if 0
        for(int i =0;i < 48;i++)
        {
            if(i%16)
                printf("%02x ",message[i]);
            else
                printf("\n0x%04x :\t%02x ",i,message[i]);

        }
#endif
        printf("msg : %s\n", recv_msg.msg);
        if((memcmp(recv_msg.msg,"!new!client!", 12) == 0))//new client command
        {
            printf("new client want to join\n");
            check_id = 0;
            if(client_num < 10)
            {
                //Search if this is the only one
                for(int i=0;i<client_num; i++)
                {
                    if(memcmp(client_ID[i], recv_msg.name, MAX_NAME_LENGTH)==0)
                    {
                        check_id++;
                        temp = i;
                    }
                }
                if(check_id)//if this already have
                {
                    pthread_mutex_lock(&Mutex_msg);
                    memset(buffer, 0, sizeof(buffer));
                    sprintf(buffer, "IDerA");// ID ERROR ALREADY HAVE
                    if(sendto(send_s, buffer, strlen(buffer), 0, (struct sockaddr *)&recv_from, sizeof(recv_from)) < strlen(buffer)){
                        perror("sendto ");
                        exit(0);
                    }//send to recvfrom
                    pthread_mutex_unlock(&Mutex_msg);
                    printf("But already have id %s / %s\n", client_ID[temp], buffer);
                }
                else//this is unique
                {
                    //strncat(client_ID[client_num],recv_msg.name, 10);
                    memcpy(client_ID[client_num],recv_msg.name,MAX_NAME_LENGTH);

                    pthread_mutex_lock(&Mutex_msg);
                    memset(buffer, 0, sizeof(buffer));
                    sprintf(buffer, "server : %s is connected", client_ID[client_num]); //ID CREATE SUCCESS
                    if(sendto(send_s, buffer, strlen(buffer), 0, (struct sockaddr *)&group_addr, sizeof(group_addr)) < strlen(buffer)){
                        perror("sendto ");
                        exit(0);
                    }//send to group
                    pthread_mutex_unlock(&Mutex_msg);
                    client_addr[client_num].sin_addr = recv_from.sin_addr;
                    printf("New Client! %s\n", client_ID[client_num]);
                    client_num++;
                }
            }
            else
            {
                pthread_mutex_lock(&Mutex_msg);
                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer, "IDerF"); // ID ERROR FULL
                if(sendto(send_s, buffer, strlen(buffer), 0, (struct sockaddr *)&recv_from, sizeof(recv_from)) < strlen(buffer)){
                    perror("sendto ");
                    exit(0);
                }
                pthread_mutex_unlock(&Mutex_msg);
                printf("client is full\n");
            }

        }
        else // no command. client msg.
        {
            //printf("check msg\n");
            check_id = -1;
            sender_id = -1;
            for(int i=0; i < client_num; i++)//search receiver ID
            {
                if(memcmp(client_ID[i], recv_msg.name, MAX_NAME_LENGTH)==0)
                {
                    check_id=i;
                }
            }
            for(int i=0; i < client_num; i++)
            {
                if(client_addr[i].sin_addr.s_addr == recv_from.sin_addr.s_addr)
                {
                    sender_id=i;
                }
            }
            //printf("check id and server %d, %d\n", check_id, sender_id);
            if(check_id > -1)//if this already have
            {
                printf("search id success\n");
                pthread_mutex_lock(&Mutex_msg);
                memset(buffer, 0, sizeof(buffer));
                memcpy(buffer, client_ID[sender_id], MAX_NAME_LENGTH);
                memcpy(&buffer[MAX_NAME_LENGTH], recv_msg.msg, MAX_MESSAGE_LENGTH-MAX_NAME_LENGTH);// ID ERROR ALREADY HAVE
                if(sendto(send_s, buffer, strlen(buffer)+1, 0, (struct sockaddr *)&client_addr[check_id], sizeof(client_addr[check_id])) < strlen(buffer)){
                    perror("sendto ");
                    exit(0);
                }//send to recvfrom
                pthread_mutex_unlock(&Mutex_msg);
                printf("Send success!\nTo %s : %s\n", client_ID[check_id], recv_msg.msg);
                printf("raw send data : %s\n\n", buffer);
            }
            else//id is not here
            {
                printf("ID does not exist\n");
                pthread_mutex_lock(&Mutex_msg);
                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer, "IDerM"); // ID ERROR for MESSAGE
                if(sendto(send_s, buffer, strlen(buffer), 0, (struct sockaddr *)&recv_from, sizeof(recv_from)) < strlen(buffer)){
                    perror("sendto ");
                    exit(0);
                }
                pthread_mutex_unlock(&Mutex_msg);
                printf("Send error %s\n\n", buffer);
            }
        }
    }
}
