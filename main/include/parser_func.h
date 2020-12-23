#include"main.h"
#ifndef PARSER_FUNC
#define PARSER_FUNC
char *get_date(char *http_response,int rlen);
char *get_login_url(char *http_response);
uint8_t parse_time2tm(struct tm* time,char* time_string);
#endif