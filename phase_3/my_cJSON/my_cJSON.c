#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "my_cJSON.h"

char *input_buffer, *output_buffer;

//private function for internal use
cJSON* create_item(int create_type){
    cJSON* item;

    item = (cJSON*)malloc(sizeof(cJSON));
    item->child = NULL;
    item->next = NULL;
    item->prev = NULL;
    item->type = create_type;
    item->valuestring = NULL;
    item->name = NULL;
    item->print_size = 5; // initializing

    return item;
}


/* Create functions */
//------------------------------------------------------------------------
cJSON* cJSON_CreateObject(void){

    return create_item(cJSON_Object);
}
cJSON* cJSON_CreateArray(void){

    return create_item(cJSON_Array);
}

cJSON* cJSON_CreateString(const char* string){
    cJSON* json_str;

    if(string == NULL){
        return NULL;
    }

    json_str = create_item(cJSON_String);
    json_str->valuestring = (char*)malloc(strlen(string)+1);
    strcpy(json_str->valuestring, string);
    json_str->print_size += strlen(string) + 2; // "<string>"

    return json_str;
}
//------------------------------------------------------------------------


/* Add functions */
//------------------------------------------------------------------------
cJSON* cJSON_AddItemToArray(cJSON* array, cJSON* item){
    cJSON* child;

    if(array == NULL || item == NULL){
        return NULL;
    }

    child = array->child;
    if (child == NULL){
        array->child = item;
    }
    else{
        while(child->next != NULL){
            child = child->next;
        }
        child->next = item;
        item->prev = child;
    }
    array->print_size += item->print_size + 1; // <item>,

    return item;
}

cJSON* cJSON_AddItemToObject(cJSON* object, const char* name, cJSON* item){

    if(object == NULL || name == NULL || item == NULL){
        return NULL;
    }

    item->name = (char*)malloc(strlen(name)+1);
    strcpy(item->name, name);
    object->print_size += strlen(name) + 3;// "<name>":

    cJSON_AddItemToArray(object, item);

    return item;
}

cJSON* cJSON_AddArrayToObject(cJSON* object, const char* name){
    cJSON* json_arr;

    if(object == NULL || name == NULL){
        return NULL;
    }
    json_arr = cJSON_CreateArray();

    return cJSON_AddItemToObject(object, name, json_arr);
}

cJSON* cJSON_AddObjectToObject(cJSON* object, const char* name){
    cJSON* json_obj;

    if(object == NULL || name == NULL){
        return NULL;
    }
    json_obj = cJSON_CreateObject();

    return cJSON_AddItemToObject(object, name, json_obj);
}

cJSON* cJSON_AddStringToObject(cJSON* object, const char* name, const char* string){
    cJSON* json_str;

    if(object == NULL || name == NULL || string == NULL){
        return NULL;
    }
    json_str = cJSON_CreateString(string);

    return cJSON_AddItemToObject(object, name, json_str);
}
//------------------------------------------------------------------------


/* Get functions */
//------------------------------------------------------------------------
cJSON* cJSON_GetArrayItem(cJSON* array, int index){
    cJSON* item;

    if(array == NULL || index < 0){
        return NULL;
    }

    item = array->child;
    for(int i = 1; i <= index && item != NULL; i++){
        item = item->next;
    }

    return item;
}

int cJSON_GetArraySize(cJSON* array){
    cJSON* child;
    int size;

    if(array == NULL){
        return 0;
    }

    child = array->child;
    size = 0;
    while(child != NULL){
        size++;
        child = child->next;
    }

    return size;
}

cJSON* cJSON_GetObjectItem(cJSON* object, const char* string){
    cJSON* item;

    if(object == NULL || string == NULL){
        return NULL;
    }

    item = object->child;
    while(item != NULL){
        if (strcmp(item->name, string) == 0){
            break;
        }
        item = item->next;
    }

    return item;
}

char* cJSON_GetStringValue(cJSON* item){

    if (item == NULL){
        return NULL;
    }

    return item->valuestring;
}
//------------------------------------------------------------------------


/* Delete functions */
//------------------------------------------------------------------------
void cJSON_Delete(cJSON* item){
    cJSON *child, *next;

    if(item == NULL){
        return;
    }

    child = item->child;
    while(child != NULL){
        next = child->next;
        cJSON_Delete(child);
        child = next;
    }

    if(item->name != NULL){
        free(item->name);
    }
    if(item->valuestring != NULL){
        free(item->valuestring);
    }
    free(item);
}

void cJSON_DeleteItemFromArray(cJSON* array, int index){
    cJSON* item;

    if(array == NULL || index < 0){
        return;
    }

    item = array->child;
    if(item!= NULL && index == 0){
        array->child = item->next;
        if(item->next != NULL){
            (item->next)->prev = NULL;
        }
    }

    else{
        for(int i = 1; i <= index && item != NULL; i++){
            item = item->next;
        }
        if(item != NULL){
            (item->prev)->next = item->next;
            if(item->next != NULL){
                (item->next)->prev = item->prev;
            }
        }
    }

    if(item != NULL){
        array->print_size -= item->print_size;
    }

    cJSON_Delete(item);
}

void cJSON_DeleteItemFromObject(cJSON* object, const char* name){
    cJSON* item;

    if(object == NULL || name == NULL){
        return;
    }

    item = object->child;
    if(item != NULL && strcmp(item->name, name) == 0){
        object->child = item->next;
        if(item->next != NULL){
            (item->next)->prev = NULL;
        }
    }
    else{
        while(item != NULL){
            if(strcmp(item->name, name) == 0){
                break;
            }
            item = item->next;
        }
        if(item != NULL){
            (item->prev)->next = item->next;

            if (item->next != NULL){
                (item->next)->prev = item->prev;
            }
        }
    }
    if(item != NULL){
        object->print_size -= item->print_size + strlen(name) + 3;
    }

    cJSON_Delete(item);
}
//------------------------------------------------------------------------


/* Print */
//------------------------------------------------------------------------

void print_object(cJSON* object){
    cJSON* child;
    char* child_str;

    strcat(output_buffer, "{"); // {
    child = object->child;
    while(child != NULL){
        strcat(output_buffer, "\""); // "
        strcat(output_buffer, child->name); // <name>
        strcat(output_buffer, "\""); // "
        strcat(output_buffer, ":"); // :
        child_str = print(child, false);
        strcat(output_buffer, child_str); // <item value>
        free(child_str);
        if(child->next != NULL){
            strcat(output_buffer, ","); // ,
        }
        child = child->next;
    }
    strcat(output_buffer, "}"); // }
}

void print_array(cJSON* array){
    cJSON* child;
    char* child_str;

    strcat(output_buffer, "["); // [
    child = array->child;
    while(child != NULL){
        child_str = print(child, false);
        strcat(output_buffer, child_str); // <item value>
        free(child_str);
        if(child->next != NULL){
            strcat(output_buffer, ","); // ,
        }
        child = child->next;
    }
    strcat(output_buffer, "]"); // ]
}

void print_string(cJSON* string){
    strcat(output_buffer, "\""); // "
    strcat(output_buffer, string->valuestring);
    strcat(output_buffer, "\""); // "
}

char* print(cJSON* item, bool main_output){
    char* tmp_str;

    if(item == NULL){
        return NULL;
    }

    tmp_str = (char*)malloc(item->print_size+10);
    *tmp_str = '\0';

    if(main_output){
        output_buffer = tmp_str;
    }

    switch(item->type){

        case cJSON_Object:
            print_object(item);
            break;

        case cJSON_Array:
            print_array(item);
            break;

        case cJSON_String:
            print_string(item);
            break;

    }

    return tmp_str;
}

char* cJSON_PrintUnformatted(cJSON* item){

    return print(item, true);
}
//------------------------------------------------------------------------


/* Parse */
//------------------------------------------------------------------------
cJSON* parse_object(void){
    int name_len;
    char* name;
    cJSON *object, *child;

    object = cJSON_CreateObject();
    while(*input_buffer != '}'){
        input_buffer++; // "
        name_len = (int)(strstr(input_buffer, "\"") - input_buffer);
        name = (char*)malloc(name_len + 1);
        memcpy(name, input_buffer, name_len);
        name[name_len] = '\0';
        input_buffer += name_len+2; // <name>":
        child = parse();
        cJSON_AddItemToObject(object, name, child);
        free(name);

        if(*input_buffer == ','){
            input_buffer++;// ,
        }
    }
    input_buffer++; // }

    return object;
}

cJSON* parse_array(void){
    cJSON *array, *child;

    array = cJSON_CreateArray();
    while(*input_buffer != ']'){
        child = parse();
        cJSON_AddItemToArray(array, child);

        if(*input_buffer == ','){
            input_buffer++;// ,
        }
    }
    input_buffer++; // ]

    return array;
}

cJSON* parse_string(void){
    cJSON* string;
    char* str;
    int str_len;

    str_len = strstr(input_buffer, "\"") - input_buffer;
    str = (char*)malloc(str_len+1);
    memcpy(str, input_buffer, str_len);
    str[str_len] = '\0';
    input_buffer += str_len+1;
    string = cJSON_CreateString(str);
    printf("%s : %d\n", str, str_len);
    free(str);

    return string;
}

cJSON* parse(void){
    cJSON* new_item;

    switch (*input_buffer){

        case '{':
            input_buffer++;
            new_item = parse_object();
            break;

        case '[':
            input_buffer++;
            new_item = parse_array();
            break;

        case '"':
            input_buffer++;
            new_item = parse_string();
            break;

    }

    return new_item;
}

cJSON* cJSON_Parse(char* string){

    if(string == NULL){
        return NULL;
    }

    input_buffer = string;
    return parse();
}
//------------------------------------------------------------------------
