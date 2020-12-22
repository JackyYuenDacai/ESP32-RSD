#include "freewifi_req.h"


const char *REQUEST = "GET " WEB_PATH " HTTP/1.0\r\n"
    "Host: "WEB_SERVER":"WEB_PORT"\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "\r\n";

char HTTPS_HOST[64] = {0};
char HTTPS_URL[128] = {0};
const char *HTTPS_REQUEST_FORMAT = 
	"POST %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 11_1_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.88 Safari/537.36\r\nContent-Type: application/x-www-form-urlencoded\r\nReferer: http://wifihk.ust.hk/\r\nUpgrade-Insecure-Requests: 1\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: ?1\r\nSec-Fetch-Dest: document\r\nOrigin: http://wifihk.ust.hk\r\nContent-Length: 53\r\n\r\n";
char HTTPS_REQUEST[1024]={0};
char IF_SEND_BODY = 1;
char HTTPS_BODY[64]= "buttonClicked=4&redirect_url=www.baidu.com&err_flag=0\r\n";

int free_wifi_request_count = 0;
unsigned char IS_AUTH_REQ_READY = 0;

void http_get_task(void *pvParameters)
{
	const struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
	};
	struct addrinfo *res;
	struct in_addr *addr;
	int s, r;
	char recv_buf[64];
	char recv_data[512] = {0};
	while (1)
	{
		int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

		if (err != 0 || res == NULL)
		{
			//ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
			vTaskDelay(10000 / portTICK_PERIOD_MS);
			continue;
		}

		/* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
		addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
		ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

		s = socket(res->ai_family, res->ai_socktype, 0);
		if (s < 0)
		{
			ESP_LOGE(TAG, "... Failed to allocate socket.");
			freeaddrinfo(res);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			continue;
		}
		ESP_LOGI(TAG, "... allocated socket");

		if (connect(s, res->ai_addr, res->ai_addrlen) != 0)
		{
			ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
			close(s);
			freeaddrinfo(res);
			vTaskDelay(4000 / portTICK_PERIOD_MS);
			continue;
		}

		ESP_LOGI(TAG, "... connected");
		freeaddrinfo(res);

		if (write(s, REQUEST, strlen(REQUEST)) < 0)
		{
			ESP_LOGE(TAG, "... socket send failed");
			close(s);
			vTaskDelay(4000 / portTICK_PERIOD_MS);
			continue;
		}
		ESP_LOGI(TAG, "... socket send success");

		struct timeval receiving_timeout;
		receiving_timeout.tv_sec = 5;
		receiving_timeout.tv_usec = 0;
		if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
					   sizeof(receiving_timeout)) < 0)
		{
			ESP_LOGE(TAG, "... failed to set socket receiving timeout");
			close(s);
			vTaskDelay(4000 / portTICK_PERIOD_MS);
			continue;
		}
		ESP_LOGI(TAG, "... set socket receiving timeout success");

		/* Read HTTP response */
		int rec = 0;
		do
		{
			bzero(recv_buf, sizeof(recv_buf));
			r = read(s, recv_buf, sizeof(recv_buf) - 1);
			for (int i = 0; i < r; i++)
			{
				putchar(recv_buf[i]);
				recv_data[rec++] = recv_buf[i];
			}
		} while (r > 0);

		ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.", r, errno);
		close(s);
		char *redirect_url = get_login_url(recv_data);
		if (redirect_url != NULL)
		{
			vTaskDelay(4000 / portTICK_PERIOD_MS);
			IS_AUTH_REQ_READY = 1;
			ESP_LOGI(TAG, "HTTP ENDING!");
			return;
		}
	}
}
char *get_date(char *http_response,int rlen)
{
	char ret_url[128] = {0};
	char **lines;
	char html_status[32] = {0};
	int i = 0, j = 0;
	int line_len[10] = {0};
	int cl = 0;
	while (http_response[i] != ' ')
		i++;
	i++;
	while (http_response[i] != '\n')
		html_status[j++] = http_response[i++];

	int status_code = atoi(html_status);
	if (status_code != 302)
	{
		return (char *)NULL;
	}
	i = 0;
	j = 0;
	//"ion: "
	while ((http_response[i] == 'a' && http_response[i + 1] == 't' && http_response[i + 2] == 'e' && http_response[i + 3] == ':') == 0 && i <= rlen - 3)
		i++;
	//Skip through "Location: "
	i += 4;
	while (http_response[i] == ' ')
		i++;
	while (http_response[i] != '\n')
	{
		ret_url[j++] = http_response[i++];
	}
//Mon, 21 Dec 2020 13:16:30 GMT
	ESP_LOGI(TAG, "DATE SYNC: %s", ret_url);

	time_t now;
	struct tm timeinfo;
	time(&now);
	localtime_r(&now, &timeinfo);

	struct tm remote_ti;
	parse_time2tm(&remote_ti,ret_url);
	time_t sec = mktime(&remote_ti); // make time_t
	ESP_LOGI(TAG,"Y:%d M:%d D:%d",remote_ti.tm_year,remote_ti.tm_mon,remote_ti.tm_mday);

	if(timeinfo.tm_year < (2016-1900) || abs(timeinfo.tm_sec - remote_ti.tm_sec)>=5){
		time_t t = mktime(&remote_ti);
    	printf("Set time: %s ;TV_SEC: %ld", asctime(&remote_ti),sec);
    	struct timeval now = { .tv_sec = t};
		now.tv_usec = 0;
    	settimeofday(&now, NULL);
	}
	return ret_url;
}
char *get_login_url(char *http_response)
{
	char ret_url[128] = {0};
	char **lines;
	char html_status[32] = {0};
	int i = 0, j = 0;
	int line_len[10] = {0};
	int cl = 0;
	int rlen = strlen(http_response);
	for (int i = 0, ll = 0; i < rlen + 1; i++)
	{
		if (http_response[i] == '\n' || http_response[i] == '\0')
		{
			line_len[cl] = i - ll;
			ll = i;
			cl++;
		}
	}

	while (http_response[i] != ' ')
		i++;
	i++;
	while (http_response[i] != ' ')
	{
		html_status[j++] = http_response[i];
		i++;
	}
	int status_code = atoi(html_status);
	if (status_code != 200)
		return (char *)NULL;
	if (strstr(http_response, "Location") == NULL)
		return (char *)NULL;

	i = 0;
	j = 0;
	//"ion: "
	while ((http_response[i] == '_' && http_response[i + 1] == 'u' && http_response[i + 2] == 'r' && http_response[i + 3] == 'l') == 0 && i <= rlen - 3)
		i++;
	//Skip through "Location: "
	i += 5;
	while (http_response[i] == ' ')
		i++;
	while (http_response[i] != '\n')
	{
		ret_url[j++] = http_response[i++];
	}

	i = 0;
	j = 0;
	while ((http_response[i] == 's' && http_response[i + 1] == ':' && http_response[i + 2] == '/' && http_response[i + 3] == '/') == 0 && i <= rlen - 3)
		i++;
	i += 5;
	while (http_response[i] == '/')
		i++;
	i -= 1;
	while (http_response[i] != '/')
	{
		HTTPS_HOST[j++] = http_response[i++];
	}
	sprintf(HTTPS_HOST, "wireless.ust.hk");
	sprintf(HTTPS_URL, "https://wireless.ust.hk/login.html");
	sprintf(HTTPS_REQUEST, HTTPS_REQUEST_FORMAT, HTTPS_URL, HTTPS_HOST);

	ESP_LOGI(TAG, "Redirect url obtained: %s", ret_url);
	ESP_LOGI(TAG, "Auth host obtained: %s", HTTPS_HOST);
	ESP_LOGI(TAG, "POST REQ: %s", HTTPS_REQUEST);
	return ret_url;
}
void freewifi_init(void)
{
	xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
	xTaskCreate(&https_get_task, "https_get_task", 8192, NULL, 6, NULL);
}

void after_req()
{
}

void https_get_task(void *pvParameters)
{
	
	char buf[512] = {0};
	int ret, len;

	while (1)
	{
		static esp_tls_cfg_t cfg = {0};
		bzero(&cfg, sizeof(esp_tls_cfg_t));
		cfg.crt_bundle_attach = esp_crt_bundle_attach;
		cfg.timeout_ms = 5000;

		if (strlen(HTTPS_HOST) <= 2 || strlen(HTTPS_URL) <= 2)
		{
			vTaskDelay(1500 / portTICK_PERIOD_MS);
			ESP_LOGI(TAG, "HTTPS WAITS...");
			continue;
		}



		if (free_wifi_request_count == 1)
		{
			HTTPS_REQUEST[0] = 'P';
			HTTPS_REQUEST[1] = 'O';
			HTTPS_REQUEST[2] = 'S';
			HTTPS_REQUEST[3] = 'T';
		}
		if (free_wifi_request_count >= 2)
		{
			sprintf(HTTPS_URL, "https://deeplink-portal.xyz/flash/go/HelloWorld");
			sprintf(HTTPS_REQUEST, "GET https://deeplink-portal.xyz/flash/go/HelloWorld HTTP/1.1\r\nHost: deeplink-portal.xyz\r\nUser-Agent: esp32-dcj\r\n\r\n");
			sprintf(HTTPS_HOST, "deeplink-portal.xyz");
		}
		if(free_wifi_request_count >=3){
			vTaskDelay(4000 / portTICK_PERIOD_MS);
			continue;
		}

		vTaskDelay(1000 / portTICK_PERIOD_MS);
		ESP_LOGI(TAG, "HTTPS START...");

		struct esp_tls *tls = esp_tls_conn_http_new(HTTPS_URL, &cfg);

		if (tls != NULL)
		{
			ESP_LOGI(TAG, "Connection established...");
		}
		else
		{
			ESP_LOGE(TAG, "Connection failed...");
			vTaskDelay(4000 / portTICK_PERIOD_MS);
			goto exit;
		}

		size_t written_bytes = 0;
		do
		{
			ret = esp_tls_conn_write(tls,
									 HTTPS_REQUEST + written_bytes,
									 strlen(HTTPS_REQUEST) - written_bytes);
			if (ret >= 0)
			{
				ESP_LOGI(TAG, "%d bytes written", ret);
				written_bytes += ret;
			}
			else if (ret != ESP_TLS_ERR_SSL_WANT_READ && ret != ESP_TLS_ERR_SSL_WANT_WRITE)
			{
				ESP_LOGE(TAG, "esp_tls_conn_write  returned 0x%x", ret);
				vTaskDelay(4000 / portTICK_PERIOD_MS);
				goto exit;
			}
		} while (written_bytes < strlen(HTTPS_REQUEST));
		ESP_LOGI(TAG, "%s", HTTPS_REQUEST);

		vTaskDelay(500 / portTICK_PERIOD_MS);
		if (IF_SEND_BODY && written_bytes != 0)
		{
			ESP_LOGI(TAG, "Sending HTTP body...");
			written_bytes = 0;
			do
			{
				ret = esp_tls_conn_write(tls,
										 HTTPS_BODY + written_bytes,
										 strlen(HTTPS_BODY) - written_bytes);
				if (ret >= 0)
				{
					ESP_LOGI(TAG, "%d bytes written", ret);
					written_bytes += ret;
				}
				else if (ret != ESP_TLS_ERR_SSL_WANT_READ && ret != ESP_TLS_ERR_SSL_WANT_WRITE)
				{
					ESP_LOGE(TAG, "esp_tls_conn_write  returned 0x%x", ret);
					vTaskDelay(4000 / portTICK_PERIOD_MS);
					goto exit;
				}
			} while (written_bytes < strlen(HTTPS_BODY));
		}
		ESP_LOGI(TAG, "Reading HTTP response...");
		do
		{
			len = sizeof(buf) - 1;
			bzero(buf, sizeof(buf));
			ret = esp_tls_conn_read(tls, (char *)buf, len);

			if (ret == ESP_TLS_ERR_SSL_WANT_WRITE || ret == ESP_TLS_ERR_SSL_WANT_READ)
				continue;

			if (ret < 0)
			{
				ESP_LOGE(TAG, "esp_tls_conn_read  returned -0x%x", -ret);
				break;
			}

			if (ret == 0)
			{
				ESP_LOGI(TAG, "connection closed");
				break;
			}

			len = ret;
			ESP_LOGD(TAG, "%d bytes read", len);
			/* Print response directly to stdout as it is read */
			for (int i = 0; i < len; i++)
			{
				putchar(buf[i]);
				//ESP_LOGD(TAG, "RB:%c",buf[i]);
			}
			char* date_str = get_date(buf,len);
			if (strstr(buf, "Continue") != NULL)
			{
				//Continue
				if (IF_SEND_BODY && written_bytes != 0)
				{
					written_bytes = 0;
					do
					{
						ret = esp_tls_conn_write(tls,
												 HTTPS_BODY + written_bytes,
												 strlen(HTTPS_BODY) - written_bytes);
						if (ret >= 0)
						{
							ESP_LOGI(TAG, "%d bytes written", ret);
							written_bytes += ret;
						}
						else if (ret != ESP_TLS_ERR_SSL_WANT_READ && ret != ESP_TLS_ERR_SSL_WANT_WRITE)
						{
							ESP_LOGE(TAG, "esp_tls_conn_write  returned 0x%x", ret);
							vTaskDelay(4000 / portTICK_PERIOD_MS);
							goto exit;
						}
					} while (written_bytes < strlen(HTTPS_BODY));
					written_bytes = 0;
				}
			}
		} while (1);

	exit:
		esp_tls_conn_delete(tls);
		putchar('\n'); // JSON output doesn't have a newline at end
		ESP_LOGI(TAG, "Completed %d requests", ++free_wifi_request_count);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}