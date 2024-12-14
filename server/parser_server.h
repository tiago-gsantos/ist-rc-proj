#ifndef PARSER_SERVER
#define PARSER_SERVER

int parse_start(char *buffer, unsigned int *id, unsigned int *time);
int parse_try(char *buffer, int *trial_num, char c[4], unsigned int *id);
int parse_st(char *buffer, unsigned int *id);
int parse_sb(char *buffer);
int parse_quit_exit(char *buffer, unsigned int *id);
int parse_debug(char *buffer, unsigned int *id, unsigned int *time, char c[4]);

#endif // PARSER_SERVER