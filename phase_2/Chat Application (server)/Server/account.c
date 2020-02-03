//account related functions decelerations

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <winsock2.h>
#include "cJSON.h"
#include "database.h"

#define MAX_LEN1 100
#define MAX_LEN2 1000
#define MAX_CLIENTS 10
#define AUTHTOKEN_LEN 30

extern cJSON* clients;
extern char users_path[MAX_LEN1];

char* AuthToken_generator(){
    srand(time(NULL));
    char* AuthToken = (char*)malloc(MAX_LEN1);
    char ch;
    int upper_range = ('Z'-'A'+1), lower_range = ('z'-'a'+1), num_range = ('9'-'0'+1);
    int length;
    for(length = 0; length < AUTHTOKEN_LEN; length++){
        ch = rand()%(upper_range+lower_range+num_range);
        if(ch < num_range){
            ch += '0';
        }
        else if(ch < num_range+upper_range){
            ch -= num_range;
            ch += 'A';
        }
        else{
            ch -= num_range+upper_range;
            ch += 'a';
        }
        AuthToken[length] = ch;
    }
    AuthToken[length] = '\0';

    return AuthToken;
}

cJSON* sign_up(const char* buffer){

    cJSON* response = cJSON_CreateObject();
    char username[MAX_LEN1], password[MAX_LEN1];

    // input validation check
    if(sscanf(buffer, "%[^, ], %s", username, password) != 2 ||
       strlen(buffer) != strlen(username)+strlen(password)+3  ){
        printf("|Error| : Invalid input pattern.\n");
        cJSON_AddStringToObject(response, "type", "Error");
        cJSON_AddStringToObject(response, "content", "Invalid input pattern.");
    }
    else{
        char path[MAX_LEN1];
        sprintf(path, "%s/%s.txt", users_path, username);

        // checking if username already exists
        FILE* user;
        if( (user = fopen(path, "r")) != NULL){
            fclose(user);
            printf("|Error| : Username already taken.\n");
            cJSON_AddStringToObject(response, "type", "Error");
            cJSON_AddStringToObject(response, "content", "Username already taken.");
        }
        else{
            cJSON* data = cJSON_CreateObject();
            cJSON_AddStringToObject(data, "username", username);
            cJSON_AddStringToObject(data, "password", password);
            write_data(data, path);
            cJSON_Delete(data);
            printf("|Successful| : %s was registered.\n", username);
            cJSON_AddStringToObject(response, "type", "Successful");
            cJSON_AddStringToObject(response, "content", "");
        }
    }
    return response;
}

cJSON* sign_in(const char* buffer){

    cJSON* response = cJSON_CreateObject();
    char username[MAX_LEN1], password[MAX_LEN1];

    // input validation check
    if(sscanf(buffer, "%[^, ], %s", username, password) != 2 ||
       strlen(buffer) != strlen(username)+strlen(password)+3  ){
        printf("|Error| : Invalid input pattern.\n");
        cJSON_AddStringToObject(response, "type", "Error");
        cJSON_AddStringToObject(response, "content", "Invalid input pattern.");
    }

    else{
        char path[MAX_LEN1];
        sprintf(path, "%s/%s.txt", users_path, username);

        // checking if user file exists
        cJSON* data = read_data(path);
        if(data == NULL){
            printf("|Error| : User '%s' not found.\n", username);
            cJSON_AddStringToObject(response, "type", "Error");
            cJSON_AddStringToObject(response, "content", "Username not found.");
        }
        else if(strcmp(cJSON_GetObjectItem(data, "password")->valuestring, password) != 0){
            cJSON_AddStringToObject(response, "type", "Error");
            cJSON_AddStringToObject(response, "content", "Wrong password.");
        }
        else{
            char* AuthToken = AuthToken_generator();
            printf("|Successful| : %s logged in.\n", username);
            cJSON_AddStringToObject(response, "type", "AuthToken");
            cJSON_AddStringToObject(response, "content", AuthToken);
            cJSON* new_client = cJSON_AddObjectToObject(clients, AuthToken);
            cJSON_AddStringToObject(new_client, "username", username);
            free(AuthToken);
        }
        cJSON_Delete(data);
    }
    return response;
}

cJSON* sign_out(const char* buffer){

    cJSON* response = cJSON_CreateObject();
    char AuthToken[MAX_LEN1];
    sscanf(buffer, "%s", AuthToken);

    if(strlen(buffer) != strlen(AuthToken)+1){
        printf("|Error| : Invalid input pattern.\n");
        cJSON_AddStringToObject(response, "type", "Error");
        cJSON_AddStringToObject(response, "content", "Invalid Input pattern.");
    }

    else if(cJSON_GetObjectItem(clients, AuthToken) == NULL){
        printf("|Error| : Invalid AuthToken.\n");
        cJSON_AddStringToObject(response, "type", "Error");
        cJSON_AddStringToObject(response, "content", "Invalid AuthToken.");
    }

    else{
        cJSON_DeleteItemFromObject(clients, AuthToken);
        printf("|Successful| : %s logged out.\n");
        cJSON_AddStringToObject(response, "type", "Successful");
        cJSON_AddStringToObject(response, "content", "");
    }

    return response;
}
