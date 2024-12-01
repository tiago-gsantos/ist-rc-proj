#ifndef PARSER_PLAYER
#define PARSER_PLAYER

int parse_start(char *buffer, char *request);
int parse_try(char *buffer, char *request);
int parse_st(char *buffer, char *request);
int parse_sb(char *buffer, char *request);
int parse_quit_exit(char *buffer, char *request);
int parse_debug(char *buffer, char *request);

#endif // PARSER_PLAYER