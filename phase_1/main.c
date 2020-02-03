#include <stdio.h>
#include<stdbool.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "cJSON.h"
#include <signal.h>

#define PORT 12345
#define MAX_LEN1 100
#define MAX_LEN2 1000

//socket creation
void socket_create();

//menu declaration
void account_menu();
void main_menu();
void channel_menu();

//account menu
void sign_up();
bool sign_in();

//main menu
bool create();
bool join();
bool sign_out();

//chat menu
void send_message();
void refresh();
void message_print();
void members();
void members_print();
bool leave();

int client_socket;
struct sockaddr_in server_address;
char auth_token[MAX_LEN1], buffer[MAX_LEN2];
cJSON* response;

int main(){
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
    account_menu();
    return 0;
}

void socket_create(){
    //socket creation
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1){
        printf("socket creation failed\n");
        exit(0);
    }
    //socket address
    memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_address.sin_port = htons(PORT);

    //server connection
	int connection = connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address));
	if (connection == -1){
        printf("connection to server failed\n");
        exit(0);
	}
}

void account_menu(){
    while(1){
        printf("----------------------------------------------------------------\n");
        printf("|ACCOUNT MENU|\n");
        printf("1 : Sign Up\n");
        printf("2 : Sign In\n");
        printf("----------------------------------------------------------------\n");
        char option[MAX_LEN1];
        fflush(stdin);
        scanf("%s", option);
        if(strcmp(option, "1") == 0){
            sign_up();
        }
        else if(strcmp(option, "2") == 0){
            if(sign_in())
                main_menu();
        }
        else{
            system("@cls||clear");
            printf("\n");
            printf("\033[0;31m");
            printf("Invalid option!\nPlease try again...\n");
            printf("\033[0m");
        }
    }
}

void main_menu(){
    while(1){
        printf("----------------------------------------------------------------\n");
        printf("|MAIN MENU|\n");
        printf("1 : Create Channel\n");
        printf("2 : Join Channel\n");
        printf("3 : Sign Out\n");
        printf("----------------------------------------------------------------\n");
        char option[MAX_LEN1];
        fflush(stdin);
        scanf("%s", option);
        if(strcmp(option, "1") == 0){
            if(create())
                channel_menu();
        }
        else if(strcmp(option, "2") == 0){
            if(join())
                channel_menu();
        }
        else if(strcmp(option, "3") == 0){
            if(sign_out())
                break;
        }
        else{
            system("@cls||clear");
            printf("\n");
            printf("\033[0;31m");
            printf("Invalid option!\nPlease try again...\n");
            printf("\033[0m");
        }
    }
}

void channel_menu(){
    while(1){
        printf("----------------------------------------------------------------\n");
        printf("|CHANNEL MENU|\n");
        printf("1 : Send Message\n");
        printf("2 : Refresh\n");
        printf("3 : Channel Members\n");
        printf("4 : Leave Channel\n");
        printf("----------------------------------------------------------------\n");
        char option[MAX_LEN1];
        fflush(stdin);
        scanf("%s", option);
        if(strcmp(option, "1") == 0){
            send_message();
        }
        else if(strcmp(option, "2") == 0){
            refresh();
        }
        else if(strcmp(option, "3") == 0){
            members();
        }
        else if(strcmp(option, "4") == 0){
            if(leave())
                break;
        }
        else{
            system("@cls||clear");
            printf("\n");
            printf("\033[0;31m");
            printf("Invalid option!\nPlease try again...\n");
            printf("\033[0m");
        }
    }
}

void sign_up(){
    printf("----------------------------------------------------------------\n");
    printf("|SIGN UP|\nEnter a valid Username and Password...\n");
    printf("----------------------------------------------------------------\n");

    // input
    char username[MAX_LEN1], password[MAX_LEN1];
    printf("'Username':\n");
    fflush(stdin);
    scanf(" %[^\n]%*c", username);
    printf("'Password':\n");
    fflush(stdin);
    scanf(" %[^\n]%*c", password);
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "register %s, %s\n", username, password);

    // connection and send/recv
    socket_create();
    send(client_socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);
    closesocket(client_socket);

    // response analysis
    response = cJSON_Parse(buffer);
    if (strcmp(cJSON_GetObjectItem(response, "type")->valuestring, "Successful") == 0){
        system("@cls||clear");
        printf("\n");
        printf("\033[0;32m");
        printf("Welcome '%s'\nYour account has been created successfully!\n", username);
        printf("\033[0m");
    }
    else if(strcmp(cJSON_GetObjectItem(response, "content")->valuestring, "this username is not available.") == 0){
        system("@cls||clear");
        printf("\n");
        printf("\033[0;31m");
        printf("Username already existing!\nPlease either sign in or try a different name...\n");
        printf("\033[0m");
    }
    else{
        system("@cls||clear");
        printf("\n");
        printf("\033[0;31m");
        printf("Invalid Username or Password!\nPlease try again...\n");
        printf("\033[0m");
    }

}

bool sign_in(){
    bool success;
    printf("----------------------------------------------------------------\n");
    printf("|SIGN IN|\nSign in to your account...\n");
    printf("----------------------------------------------------------------\n");

    // input
    char username[MAX_LEN1], password[MAX_LEN1];
    printf("'Username':\n");
    fflush(stdin);
    scanf(" %[^\n]%*c", username);
    printf("'Password':\n");
    fflush(stdin);
    scanf(" %[^\n]%*c", password);
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "login %s, %s\n", username, password);

    // connection and send/recv
    socket_create();
    send(client_socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);
    closesocket(client_socket);

    //response analysis
    response = cJSON_Parse(buffer);
    if (strcmp(cJSON_GetObjectItem(response, "type")->valuestring, "AuthToken") == 0){
        system("@cls||clear");
        printf("\n");
        printf("\033[0;32m");
        printf("Welcome '%s'\nYou have successfully signed into your account!\n", username);
        printf("\033[0m");
        strcpy(auth_token, cJSON_GetObjectItem(response, "content")->valuestring);
        success = 1;
    }
    else{
        system("@cls||clear");
        printf("\n");
        printf("\033[0;31m");
        printf("Invalid Username or Password!\nPlease try again...\n");
        printf("\033[0m");
        success = 0;
    }
    return success;
}

bool create(){
    bool success;
    printf("----------------------------------------------------------------\n");
    printf("|CREATE CHANNEL|\nEnter a valid name for your channel...\n");
    printf("----------------------------------------------------------------\n");

    // input
    char ch_name[MAX_LEN1];
    printf("'Channel Name':\n");
    fflush(stdin);
    scanf(" %[^\n]%*c", ch_name);
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "create channel %s, %s\n", ch_name, auth_token);

    // connection and send/recv
    socket_create();
    send(client_socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);
    closesocket(client_socket);

    // response analysis
    response = cJSON_Parse(buffer);
    if (strcmp(cJSON_GetObjectItem(response, "type")->valuestring, "Successful") == 0){
        system("@cls||clear");
        printf("\n");
        printf("\033[0;32m");
        printf("Channel '%s' has been successfully created!\n", ch_name);
        printf("\033[0m");
        success = 1;
    }
    else{
        system("@cls||clear");
        printf("\n");
        printf("\033[0;31m");
        printf("Channel name already existing!\nPlease either join or try another name...\n");
        printf("\033[0m");
        success = 0;
    }
    return success;
}

bool join(){
    bool success;
    printf("----------------------------------------------------------------\n");
    printf("|JOIN CHANNEL|\nPlease enter channel name to join...\n");
    printf("-----------------------------------------------------------------\n");

    // input
    char ch_name[MAX_LEN1];
    printf("'Channel Name':\n");
    fflush(stdin);
    scanf(" %[^\n]%*c", ch_name);
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "join channel %s, %s\n", ch_name, auth_token);

    // connection and send/recv
    socket_create();
    send(client_socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);
    closesocket(client_socket);

    // response analysis
    response = cJSON_Parse(buffer);
    if (strcmp(cJSON_GetObjectItem(response, "type")->valuestring, "Successful") == 0){
        system("@cls||clear");
        printf("\n");
        printf("\033[0;32m");
        printf("You have successfully joined '%s'!\n", ch_name);
        printf("\033[0m");
        success = 1;
    }
    else{
        system("@cls||clear");
        printf("\n");
        printf("\033[0;31m");
        printf("Invalid channel name!\nPlease try again...\n");
        printf("\033[0m");
        success = 0;
    }
    return success;
}

bool sign_out(){
    bool success;
    // connection and send/recv
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "logout %s\n", auth_token);
    socket_create();
    send(client_socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);
    closesocket(client_socket);

    // response analysis
    response = cJSON_Parse(buffer);
    if (strcmp(cJSON_GetObjectItem(response, "type")->valuestring, "Successful") == 0){
        system("@cls||clear");
        printf("\n");
        printf("\033[0;32m");
        printf("You have successfully signed out!\n");
        printf("\033[0m");
        success = 1;
    }
    else{
        system("@cls||clear");
        printf("\n");
        printf("\033[0;31m");
        printf("Operation failed!\nPlease try again...\n");
        printf("\033[0m");
        success = 0;

    }
    return success;
}

void send_message(){
    printf("----------------------------------------------------------------\n");
    printf("|SEND MESSAGE|\nPlease enter your message...\n");
    printf("----------------------------------------------------------------\n");

    // input
    printf("'Message':\n");
    char message_txt[MAX_LEN2];
    fflush(stdin);
    scanf(" %[^\n]%*c", message_txt);
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "send %s, %s\n", message_txt, auth_token);

    // connection and send/recv
    socket_create();
    send(client_socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);
    closesocket(client_socket);

    // response analysis
    response = cJSON_Parse(buffer);
    if (strcmp(cJSON_GetObjectItem(response, "type")->valuestring, "Successful") == 0){
        system("@cls||clear");
        printf("\n");
        printf("\033[0;32m");
        printf("Message sent successfully!\n");
        printf("\033[0m");
    }
    else{
        system("@cls||clear");
        printf("\n");
        printf("\033[0;31m");
        printf("Failed to send your message!\nPlease try again...\n");
        printf("\033[0m");
    }
}

void refresh(){
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "refresh %s\n", auth_token);

    // connection and send/recv
    socket_create();
    send(client_socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);
    closesocket(client_socket);
    response = cJSON_Parse(buffer);
    if (strcmp(cJSON_GetObjectItem(response, "type")->valuestring, "List") == 0){
        response = cJSON_GetObjectItem(response, "content");
        system("@cls||clear");
        printf("\n");
        message_print();
        printf("\nPress any key to continue...\n");
        getch();
        system("@cls||clear");
        printf("\n");
    }
    else{
        system("@cls||clear");
        printf("\n");
        printf("\033[0;31m");
        printf("Operation failed!\nPlease try again...\n");
        printf("\033[0m");
    }
}

void message_print(){
    printf("Recent messages:\n\n");
    int count = cJSON_GetArraySize(response);
    for (int i = 0; i < count; i++){
        cJSON* message = cJSON_GetArrayItem(response, i);
        if (strcmp(cJSON_GetObjectItem(message, "sender")->valuestring, "server") == 0){
            printf("\t\t%s\n", cJSON_GetObjectItem(message, "content")->valuestring);
        }
        else{
            printf("%14s: %s\n", (cJSON_GetObjectItem(message, "sender")->valuestring)
                               , (cJSON_GetObjectItem(message, "content")->valuestring));
        }
    }
}

void members(){
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "channel members %s\n", auth_token);

    // connection and send/recv
    socket_create();
    send(client_socket, buffer, strlen(buffer), 0);
    memset(&buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);
    closesocket(client_socket);

    // response analysis
    response = cJSON_Parse(buffer);
    if (strcmp(cJSON_GetObjectItem(response, "type")->valuestring, "List") == 0){
        response = cJSON_GetObjectItem(response, "content");
        system("@cls||clear");
        printf("\n");
        members_print();
        printf("\nPress any key to continue...\n");
        getch();
        system("@cls||clear");
    }
    else{
        system("@cls||clear");
        printf("\n");
        printf("\033[0;31m");
        printf("Operation failed!\nPlease try again...\n");
        printf("\033[0m");
    }
}

void members_print(){
    printf("Channel members:\n\n");
    int count = cJSON_GetArraySize(response);
    for (int i = 0; i < count; i++){
        printf("\t%s\n", cJSON_GetArrayItem(response, i)->valuestring);
    }
}

bool leave(){
    bool success;
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "leave %s\n", auth_token);

    // connection and send/recv
    socket_create();
    send(client_socket, buffer, strlen(buffer), 0);
    memset(&buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);
    closesocket(client_socket);

    // response
    response = cJSON_Parse(buffer);
    if (strcmp(cJSON_GetObjectItem(response, "type")->valuestring, "Successful") == 0){
        system("@cls||clear");
        printf("\n");
        printf("\033[0;32m");
        printf("You left channel successfully!\n");
        printf("\033[0m");
        success = 1;
    }
    else{
        system("@cls||clear");
        printf("\n");
        printf("\033[0;31m");
        printf("Operation failed!\nPlease try again...\n");
        printf("\033[0m");
        success = 0;
    }
}
