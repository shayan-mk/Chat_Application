//channel related functions prototypes


void AuthToken_check(cJSON* response, const char* AuthToken);
void channel_check(cJSON* response, const char* AuthToken);

cJSON* create_channel(const char* buffer);
cJSON* join_channel(const char* buffer);
cJSON* send_message(const char* buffer);
cJSON* refresh(const char* buffer);
cJSON* members(const char* buffer);
cJSON* leave_channel(const char* buffer);


