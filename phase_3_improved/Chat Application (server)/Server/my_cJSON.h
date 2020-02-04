#include <stdbool.h>

/*
 --McSheen Chat Application (Phase3: JSON Parser and printer)--
 By: Shayan Mohammadi Kubijari

* This is a personal derivative of cJSON library to use in Chat Application project.
* Not all the functions are included; Only the ones used in Phase 1 and 2.
* Same naming standard is used for the functions to prevent rewriting the server and client codes.
* Only string values are supported for items. (there is no 'valueint' , 'cJSON_AddNumberToObject()' and etc.
* Unformatted printing is used in this library. (there is no extra \n, \t or space in printed string)
*/

/* cJSON types definition */
#define cJSON_Object 0
#define cJSON_Array  1
#define cJSON_String 2


typedef struct cJSON{

    // next and previous items in an object/array
    struct cJSON* next;
    struct cJSON* prev;

    // child of an object/array (initializes with NULL)
    struct cJSON* child;

    // item's type; object(0), array(1) or string(2)
    int type;

    // cJSON_Print() output size
    int print_size;

    // only string values are supported
    char* valuestring;

    // object's name (if it is child or member of another object/array
    char* name;

} cJSON;

/*Private functions(used inside my_cJSON.c */
cJSON* create_item(int create_type);

char* print(cJSON* item, bool main_output);
void print_object(cJSON* object);
void print_array(cJSON* array);
void print_string(cJSON* string);

cJSON* parse(void);
cJSON* parse_object(void);
cJSON* parse_array(void);
cJSON* parse_string(void);


/* Public functions */

// Create functions
cJSON* cJSON_CreateObject(void);
cJSON* cJSON_CreateArray(void);
cJSON* cJSON_CreateString(const char* string);

// Add functions
cJSON* cJSON_AddItemToObject(cJSON* object, const char* name, cJSON* item);
cJSON* cJSON_AddItemToArray(cJSON* array, cJSON* item);
cJSON* cJSON_AddArrayToObject(cJSON* object, const char* name);
cJSON* cJSON_AddObjectToObject(cJSON* object, const char* name);
cJSON* cJSON_AddStringToObject(cJSON* object, const char* name, const char* string);

// Get functions
cJSON* cJSON_GetArrayItem(cJSON* array, int index);
int cJSON_GetArraySize(cJSON* array);
cJSON* cJSON_GetObjectItem(cJSON* object, const char* name);
char* cJSON_GetStringValue(cJSON* item);

// Delete functions
void cJSON_Delete(cJSON* item);
void cJSON_DeleteItemFromArray(cJSON* array, int index);
void cJSON_DeleteItemFromObject(cJSON* object, const char* name);

// Print, Parse and Duplicate
char* cJSON_PrintUnformatted(cJSON* item);
cJSON* cJSON_Parse(char* string);
