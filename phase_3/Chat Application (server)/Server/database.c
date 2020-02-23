//database related functions decelerations

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "my_cJSON.h"

#define MAX_LEN1 100
#define MAX_LEN2 1000


extern char users_path[MAX_LEN1], channels_path[MAX_LEN1];

cJSON* read_data(const char* path){
    FILE* fptr = fopen(path, "r");
    if (fptr == NULL){
        return NULL;
    }
    char buffer[MAX_LEN2];
    int flength = 0;
    while (!feof(fptr)){
        buffer[flength] = fgetc(fptr);
        flength++;
    }
    buffer[flength] = '\0';
    fclose(fptr);
    cJSON* output = cJSON_Parse(buffer);
    return output;
}

void write_data(cJSON* data, const char* path){
    char* buffer = cJSON_PrintUnformatted(data);
    FILE* fptr = fopen(path, "w+");
    fprintf(fptr, "%s", buffer);
    fclose(fptr);
    free(buffer);
}

void make_dir(char* path){
    char* ptr = strstr(path, "/");
    while(ptr != NULL){
        *ptr = '\0';
        mkdir(path);
        *ptr = '/';
        ptr = strstr(ptr+1, "/");
    }
    mkdir(path);
}

void init_database(const char* config_path){

    // process config data using cJSON
    cJSON* config_json = read_data(config_path);
    if(config_json == NULL){
        printf("Server_Config.txt missing!\n");
        exit(0);
    }
    strcpy(users_path, cJSON_GetObjectItem(config_json, "usersPath")->valuestring);
    strcpy(channels_path, cJSON_GetObjectItem(config_json, "channelsPath")->valuestring);
    cJSON_Delete(config_json);

    // creating database
    make_dir(users_path);
    make_dir(channels_path);
}
