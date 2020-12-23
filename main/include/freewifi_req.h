#include"main.h"
#ifndef FREEWIFI_REQ_H
#define FREEWIFI_REQ_H
#define WEB_SERVER "captive.apple.com"
#define WEB_PORT "80"
#define WEB_PATH "/"

extern const char* REQUEST;
extern char HTTPS_HOST[];
extern char HTTPS_URL[];
extern const char* HTTPS_REQUEST_FORMAT;
extern char HTTPS_REQUEST[1024];
extern char IF_SEND_BODY;
extern char HTTPS_BODY[];
extern int free_wifi_request_count;
extern unsigned char IS_AUTH_REQ_READY;


void https_get_task(void *pvParameters);
void http_get_task(void *pvParameters);

void freewifi_init(void);
char* get_login_url(char*);
char *get_date(char *http_response,int rlen);
char *get_login_url(char *http_response);

#endif