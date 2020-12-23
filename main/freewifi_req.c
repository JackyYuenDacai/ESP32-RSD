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

void freewifi_init(void)
{
	xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
	xTaskCreate(&https_get_task, "https_get_task", 8192, NULL, 6, NULL);
}