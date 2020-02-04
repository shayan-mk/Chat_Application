//account related functions prototypes


char* AuthToken_generator(void);

cJSON* sign_up(const char* buffer);
cJSON* sign_in(const char* buffer);
cJSON* sign_out(const char* buffer);
