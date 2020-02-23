//database management functions prototype

void make_dir(char* path);
void init_database(const char* config_path);
cJSON* read_data(const char* path);
void write_data(cJSON*, const char* path);
