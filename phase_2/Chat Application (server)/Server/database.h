//database management functions prototype

void make_dir(char* path);
void init_database(const char* config_path, int* PORT_ptr);
cJSON* read_data(const char* path);
void write_data(cJSON*, const char* path);
