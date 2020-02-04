#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "my_cJSON.h"
#include "account.h"
#include "channel.h"
#include "database.h"

#define MAX_LEN1 100
#define MAX_LEN2 1000
#define MAX_CLIENTS 10
#define PORT 12345

int socket_create();
void request_process();

char users_path[MAX_LEN1], channels_path[MAX_LEN1];
cJSON* clients, *channels;

int main()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

	// Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        // Tell the user that we could not find a usable Winsock DLL.
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }

    // initializing...
    init_database("../Server_Config.txt");
    clients = cJSON_CreateObject();
    cJSON* server = cJSON_AddObjectToObject(clients, "server");
    cJSON_AddStringToObject(server, "username", "server");
    channels = cJSON_CreateObject();

    // main while
    while(1){
        request_process();
    }

    return 0;
}

int socket_create(){

    // socket creation
    int server_socket = socket(AF_INET, SOCK_STREAM, 6);
    if (server_socket == -1){
        printf("socket creation failed\n");
        exit(0);
    }

    // socket address
    struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(PORT);

    // binding
	if(bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) != 0){
        printf("binding failed\n");
        exit(0);
	}

	// listening
	if(listen(server_socket, 5) != 0){
        printf("listening failed\n");
        exit(0);
	}
	return server_socket;
}

void request_process(){

    // connecting to client
    int server_socket = socket_create();
    int client_socket = accept(server_socket, NULL, NULL);

    // receiving request from client
    cJSON* response;
    char buffer[MAX_LEN2];
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);

    if(strstr(buffer, "register ") == buffer){
        response = sign_up(buffer+strlen("register "));
    }
    else if(strstr(buffer, "login ") == buffer){
        response = sign_in(buffer+strlen("login "));
    }
    else if(strstr(buffer, "create channel ") == buffer){
        response = create_channel(buffer+strlen("create channel "));
    }
    else if(strstr(buffer, "join channel ") == buffer){
        response = join_channel(buffer+strlen("join channel "));
    }
    else if(strstr(buffer, "send message ") == buffer){
        response = send_message(buffer+strlen("send message "));
    }
    else if(strstr(buffer, "refresh ") == buffer){
        response = refresh(buffer+strlen("refresh "));
    }
    else if(strstr(buffer, "channel members ") == buffer){
        response = members(buffer+strlen("channel members "));
    }
    else if(strstr(buffer, "leave ") == buffer){
        response = leave_channel(buffer+strlen("leave "));
    }
    else if(strstr(buffer, "logout ") == buffer){
        response = sign_out(buffer+strlen("logout "));
    }
    else{
        response = cJSON_CreateObject();
        cJSON_AddStringToObject(response, "type", "Error");
        cJSON_AddStringToObject(response, "content", "Invalid Request.");
    }

    // sending response to client
    char* final_response = cJSON_PrintUnformatted(response);
    printf("Response: %s\n\n", final_response);
    send(client_socket, final_response, MAX_LEN2, 0);
    cJSON_Delete(response);
    free(final_response);
    closesocket(server_socket);
}
