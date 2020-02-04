//channel related functions decelerations

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "database.h"

#define MAX_LEN1 100
#define MAX_LEN2 1000
#define MAX_CLIENTS 10

extern cJSON* clients, *channels;
extern char channels_path[MAX_LEN1];

void AuthToken_check(cJSON* response, const char* AuthToken){
    cJSON* client;

    if(cJSON_GetObjectItem(response, "type") == NULL){ // checking for previous errors

        client = cJSON_GetObjectItem(clients, AuthToken);
        if(client == NULL){
            printf("|Error| : Invalid AuthToken.\n");
            cJSON_AddStringToObject(response, "type", "Error");
            cJSON_AddStringToObject(response, "content", "Invalid AuthToken.");
        }
    }
}

void channel_check(cJSON* response, const char* AuthToken){
    cJSON* channel;

    if(cJSON_GetObjectItem(response, "type") == NULL){ // checking for previous errors

        channel = cJSON_GetObjectItem(cJSON_GetObjectItem(clients, AuthToken), "channel");
        if(channel == NULL){
            printf("|Error| : User is not a member of any channel.\n");
            cJSON_AddStringToObject(response, "type", "Error");
            cJSON_AddStringToObject(response, "content", "User is not a member of any channel.");
        }
    }
}

cJSON* send_message(const char* buffer){
    cJSON* response = cJSON_CreateObject();
    char message_str[MAX_LEN2], AuthToken[MAX_LEN1];
    int inputs_count;

    inputs_count = sscanf(buffer, "%[^,], %s", message_str, AuthToken);
    /* Error checking */
    // input check
     if(inputs_count != 2 || strlen(buffer) != strlen(message_str)+strlen(AuthToken)+3){
        printf("|Error| : Invalid input pattern.\n");
        cJSON_AddStringToObject(response, "type", "Error");
        cJSON_AddStringToObject(response, "content", "Invalid input pattern.");
    }

    // error checking and getting data from channel's file
    AuthToken_check(response, AuthToken);
    channel_check(response, AuthToken);

    /* Main operation */
    // if there is no previous error...
    if(cJSON_GetObjectItem(response, "type") == NULL){
        cJSON* client = cJSON_GetObjectItem(clients, AuthToken);
        char* ch_name = cJSON_GetObjectItem(client, "channel")->valuestring;
        char* username = cJSON_GetObjectItem(client, "username")->valuestring;
        cJSON* messages_arr;
        cJSON* message = cJSON_CreateObject();
        char path[MAX_LEN1];

        // reading data from channel's text file and adding it to response
        sprintf(path, "%s/%s.txt", channels_path, ch_name);
        messages_arr = read_data(path);

        // constructing the message
        cJSON_AddStringToObject(message, "sender", username);
        cJSON_AddStringToObject(message, "content", message_str);
        cJSON_AddItemToArray(messages_arr, message);

        // writing updated message list in file
        write_data(messages_arr, path);

        // deleting extra JSONs
        cJSON_Delete(messages_arr);

        // updating response
        if(strcmp(AuthToken, "server") != 0){
            printf("|Successful| : '%s' sent a message.\n", username);
            cJSON_AddStringToObject(response, "type", "Successful");
            cJSON_AddStringToObject(response, "content", "");
        }
    }

    return response;
}

cJSON* create_channel(const char* buffer){
    cJSON* response = cJSON_CreateObject();
    char ch_name[MAX_LEN1], AuthToken[MAX_LEN1];
    int inputs_count;

    inputs_count = sscanf(buffer, "%[^, ], %s", ch_name, AuthToken);

    /* Error checking */
    // input check
     if(inputs_count != 2 || strlen(buffer) != strlen(ch_name)+strlen(AuthToken)+3){
        printf("|Error| : Invalid input pattern.\n");
        cJSON_AddStringToObject(response, "type", "Error");
        cJSON_AddStringToObject(response, "content", "Invalid input pattern.");
    }

    // AuthToken validity check
    AuthToken_check(response, AuthToken);

    /* Main operation */
    // if there is no previous error...
    if(cJSON_GetObjectItem(response, "type") == NULL){
        char path[MAX_LEN1];
        sprintf(path, "%s/%s.txt", channels_path, ch_name);

        // checking if channel already exists
        FILE* fptr;
        if( (fptr = fopen(path, "r")) != NULL){
            fclose(fptr);
            printf("|Error| : Channel already exists.\n");
            cJSON_AddStringToObject(response, "type", "Error");
            cJSON_AddStringToObject(response, "content", "Channel already exists.");
        }
        else{
            cJSON* channel = cJSON_AddArrayToObject(channels, ch_name);
            cJSON* client = cJSON_GetObjectItem(clients, AuthToken);
            char* username = cJSON_GetObjectItem(client, "username")->valuestring;
            char message_str[MAX_LEN2];
            cJSON* data;

            // creating messages array
            data = cJSON_CreateArray();
            write_data(data, path);
            cJSON_Delete(data);

            // adding username to members
            cJSON_AddItemToArray(channel, cJSON_CreateString(username));

            // adding channel name to user's and server's data
            cJSON_AddStringToObject(client, "channel", ch_name);
            cJSON_AddStringToObject(cJSON_GetObjectItem(clients, "server"), "channel", ch_name);

            // sending creation message
            sprintf(message_str, "%s created %s., server\n", username, ch_name);
            cJSON_Delete(send_message(message_str));
            cJSON_DeleteItemFromObject(cJSON_GetObjectItem(clients, "server"), "channel");

            // updating response
            printf("|Successful| : '%s' has been created.\n", ch_name);
            cJSON_AddStringToObject(response, "type", "Successful");
            cJSON_AddStringToObject(response, "content", "");
        }
    }

    return response;
}

cJSON* join_channel(const char* buffer){
    cJSON* response = cJSON_CreateObject();
    char ch_name[MAX_LEN1], AuthToken[MAX_LEN1];

    /* Error checking */
    // input check
     if(sscanf(buffer, "%[^, ], %s", ch_name, AuthToken) != 2 ||
       strlen(buffer) != strlen(ch_name)+strlen(AuthToken)+3  ){
        printf("|Error| : Invalid input pattern.\n");
        cJSON_AddStringToObject(response, "type", "Error");
        cJSON_AddStringToObject(response, "content", "Invalid input pattern.");
    }
    // AuthToken validity check
    AuthToken_check(response, AuthToken);

    /* Main operation */
    // if there is no error...
    if(cJSON_GetObjectItem(response, "type") == NULL){
        cJSON* client = cJSON_GetObjectItem(clients, AuthToken);
        cJSON* channel = cJSON_GetObjectItem(channels, ch_name);
        char* username = cJSON_GetObjectItem(client, "username")->valuestring;
        char message_str[MAX_LEN2];
        char path[MAX_LEN1];

        sprintf(path, "%s/%s.txt", channels_path, ch_name);

        // checking if channel file exists
        FILE* fptr;
        if((fptr = fopen(path, "r")) == NULL){
            printf("|Error| : Channel '%s' not found.\n", ch_name);
            cJSON_AddStringToObject(response, "type", "Error");
            cJSON_AddStringToObject(response, "content", "Channel not found.");
        }
        else{
            fclose(fptr);

            // creating channel's member list (if does'nt exist)
            if(channel == NULL){
                channel = cJSON_AddArrayToObject(channels, ch_name);
            }

            // adding username to members
            cJSON_AddItemToArray(channel, cJSON_CreateString(username));

            // adding channel name to user's data
            cJSON_AddStringToObject(client, "channel", ch_name);

            cJSON_AddStringToObject(cJSON_GetObjectItem(clients, "server"), "channel", ch_name);
            // sending creation message
            sprintf(message_str, "%s joined., server\n", username);
            cJSON_Delete(send_message(message_str));
            cJSON_DeleteItemFromObject(cJSON_GetObjectItem(clients, "server"), "channel");

            // updating response
            printf("|Successful| : '%s' joined '%s'.\n", username, ch_name);
            cJSON_AddStringToObject(response, "type", "Successful");
            cJSON_AddStringToObject(response, "content", "");
        }
    }
    return response;
}

cJSON* refresh(const char* buffer){
    cJSON* response = cJSON_CreateObject();
    char AuthToken[MAX_LEN1];

    sscanf(buffer, "%s", AuthToken);

    /* Error checking */
    // input check
    if(strlen(buffer) != strlen(AuthToken)+1){
        printf("|Error| : Invalid input pattern.\n");
        cJSON_AddStringToObject(response, "type", "Error");
        cJSON_AddStringToObject(response, "content", "Invalid input pattern.");
    }
    // AuthToken and channel validity check
    AuthToken_check(response, AuthToken);
    channel_check(response, AuthToken);

    /* Main operation */
    // if there is no error...
    if(cJSON_GetObjectItem(response, "type") == NULL){
        cJSON* client = cJSON_GetObjectItem(clients, AuthToken);
        char* ch_name = cJSON_GetObjectItem(client, "channel")->valuestring;
        char path[MAX_LEN1];
        cJSON* data;

        // reading data from channel's text file and adding it to response
        sprintf(path, "%s/%s.txt", channels_path, ch_name);
        data = read_data(path);

        // updating response
        printf("|Successful| : Messages list.\n");
        cJSON_AddStringToObject(response, "type", "List");
        cJSON_AddItemToObject(response, "content", data);
    }

    return response;
}

cJSON* members(const char* buffer){
    cJSON* response = cJSON_CreateObject();
    char AuthToken[MAX_LEN1];

    sscanf(buffer, "%s", AuthToken);

    /* Error checking */
    // input check
    if(strlen(buffer) != strlen(AuthToken)+1){
        printf("|Error| : Invalid input pattern.\n");
        cJSON_AddStringToObject(response, "type", "Error");
        cJSON_AddStringToObject(response, "content", "Invalid input pattern.");
    }
    // AuthToken and channel validity check
    AuthToken_check(response, AuthToken);
    channel_check(response, AuthToken);

    /* Main operation */
    // if there is no error...
    if(cJSON_GetObjectItem(response, "type") == NULL){
        cJSON* client = cJSON_GetObjectItem(clients, AuthToken);
        cJSON* channel = cJSON_GetObjectItem(channels, cJSON_GetObjectItem(client, "channel")->valuestring);
        char* tmp_str;
        cJSON* tmp_json;

        // response will be removed later in main, so as tmp...
        tmp_str = cJSON_PrintUnformatted(channel);
        tmp_json = cJSON_Parse(tmp_str);
        free(tmp_str);

        // adding members list to response
        printf("|Successful| : Members list.\n");
        cJSON_AddStringToObject(response, "type", "List");
        cJSON_AddItemToObject(response, "content", tmp_json);
    }

    return response;
}

cJSON* leave_channel(const char* buffer){
    cJSON* response = cJSON_CreateObject();
    char AuthToken[MAX_LEN1];

    sscanf(buffer, "%s", AuthToken);

    /* Error checking */
    // input check
    if(strlen(buffer) != strlen(AuthToken)+1){
        printf("|Error| : Invalid input pattern.\n");
        cJSON_AddStringToObject(response, "type", "Error");
        cJSON_AddStringToObject(response, "content", "Invalid input pattern.");
    }
    // AuthToken and channel validity check
    AuthToken_check(response, AuthToken);
    channel_check(response, AuthToken);

    /* Main operation */
    // if there is no error...
    if(cJSON_GetObjectItem(response, "type") == NULL){
        cJSON* client = cJSON_GetObjectItem(clients, AuthToken);
        cJSON* channel = cJSON_GetObjectItem(channels, cJSON_GetObjectItem(client, "channel")->valuestring);
        char* ch_name = cJSON_GetObjectItem(client, "channel")->valuestring;
        char* username = cJSON_GetObjectItem(client, "username")->valuestring;
        char message_str[MAX_LEN2];

        // finding and deleting user from channel's member list
        // removing channel from user's data
        for(int i = 0; i < cJSON_GetArraySize(channel); i++){
            if(strcmp(cJSON_GetArrayItem(channel, i)->valuestring, username) == 0){
                cJSON_DeleteItemFromArray(channel, i);
                cJSON_DeleteItemFromObject(client, "channel");
                break;
            }
        }

        cJSON_AddStringToObject(cJSON_GetObjectItem(clients, "server"), "channel", ch_name);
        // sending leave message
        sprintf(message_str, "%s left., server\n", username);
        cJSON_Delete(send_message(message_str));
        cJSON_DeleteItemFromObject(cJSON_GetObjectItem(clients, "server"), "channel");

        //updating response
        printf("|Successful| : '%s' left '%s'.\n", username, ch_name);
        cJSON_AddStringToObject(response, "type", "Successful");
        cJSON_AddStringToObject(response, "content", "");
    }

    return response;
}
