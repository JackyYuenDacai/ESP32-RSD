#include "nas_task.h"

//nas?time=22-01-2015 10:15:55 AM&milli_mul=100&milli_index=0&avg_amp=0.2&max_amp=0.2&min_amp=0.1
const char *NAS_REQUEST_FORMAT = "GET https://deeplink-portal.xyz/nas/?%s HTTP/1.1\r\nHost: deeplink-portal.xyz\r\nUser-Agent: esp32-dcj\r\n\r\n";
const char *NAS_HOST = "deeplink-portal.xyz";
char NAS_URL[180] = {0};

char *nas_request_pack(struct nas_entry *entry)
{
	char param_str[128] = {0};
	if (entry == NULL)
		return NULL;
	//nas_entry_to_params((char*)param_str, entry);

	sprintf(param_str, "time=%s&milli_mul=%d&milli_index=%d&avg_amp=%2f&max_amp=%2f&min_amp=%2f",
			entry->time_str,
			entry->milli_mul, entry->milli_index,
			entry->avg_amp, entry->max_amp, entry->min_amp);

	sprintf(HTTPS_REQUEST, NAS_REQUEST_FORMAT, param_str);
	sprintf(NAS_URL, "https://deeplink-portal.xyz/nas/?%s", param_str);
	return HTTPS_REQUEST;
}
static unsigned char nas_request(struct nas_entry *entry)
{
	char buf[512];
	char html_status[16];
	int status_code = 0;
	int len = 0, ret = 0;
	nas_request_pack(entry);
	esp_tls_cfg_t cfg = {0};
	bzero(&cfg, sizeof(esp_tls_cfg_t));
	cfg.crt_bundle_attach = esp_crt_bundle_attach;
	cfg.timeout_ms = 5000;
	struct esp_tls *tls = esp_tls_conn_http_new(NAS_URL, &cfg);
	if (tls != NULL)
	{
		ESP_LOGI(TAG, "Connection established...");
	}
	else
	{
		ESP_LOGE(TAG, "Connection failed...");
		vTaskDelay(500 / portTICK_PERIOD_MS);
		goto nas_request_exit;
	}
	size_t written_bytes = 0;
	ESP_LOGI(TAG, "%s", NAS_URL);
	do
	{
		ret = esp_tls_conn_write(tls, HTTPS_REQUEST + written_bytes, strlen(HTTPS_REQUEST) - written_bytes);
		if (ret >= 0)
		{
			ESP_LOGI(TAG, "%d bytes written", ret);
			written_bytes += ret;
		}
		else if (ret != ESP_TLS_ERR_SSL_WANT_READ && ret != ESP_TLS_ERR_SSL_WANT_WRITE)
		{
			ESP_LOGE(TAG, "esp_tls_conn_write  returned 0x%x", ret);
			vTaskDelay(500 / portTICK_PERIOD_MS);
			goto nas_request_exit;
		}
	} while (written_bytes < strlen(HTTPS_REQUEST));
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
		if (status_code != 302)
		{
			int i = 0, j = 0;
			while (buf[i] != 'T' && buf[i + 1] != 'P' && buf[i + 2] != '/' && i < ret - 3)
				i++;
			i += 5;
			if (i >= ret - 5)
				goto nas_request_exit;
			while (buf[i] != ' ' && i < ret - 3)
				i++;
			while (buf[i] == ' ' && i < ret - 3)
				i++;
			while (buf[i] != ' ' && buf[i] != '\n' && i < ret - 3)
			{
				html_status[j++] = buf[i++];
			}

			status_code = atoi(html_status);
			ESP_LOGI(TAG, "STATUS: %d", status_code);
		}
		if (status_code == 302)
		{
			esp_tls_conn_delete(tls);
			return 0;
		}
		else
		{
			esp_tls_conn_delete(tls);
			return -1;
		}
	} while (1);
nas_request_exit:
	esp_tls_conn_delete(tls);
	if (status_code == 302)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}
/*
	unsigned char nas_queue_push(struct nas_entry* entry);
	unsigned char nas_queue_empty();
	unsigned char nas_queue_is_empty();
	nas_entry* nas_queue_pop();
	nas_entry* nas_queue_top();
*/
void nas_task(void *pvp)
{
	struct nas_entry entry = {0};
	struct nas_entry* entryp = NULL;
	while (1)
	{
		if (free_wifi_request_count >= 3 && abs(sound_samples_index - sound_samples_start)>=50)
		{
			time_t now;
			struct tm timeinfo;
			time(&now);
			localtime_r(&now, &timeinfo);
			get_record_package(&entry,&timeinfo);
			if (nas_request(&entry) == 0)
			{
				ESP_LOGI(TAG, "NAS RECORD SUBMITTED.");
			}
			else
			{
				ESP_LOGI(TAG, "NAS RECORD SUBMIT FAILED.");
			}
		}
		else
		{
			vTaskDelay(2000 / portTICK_PERIOD_MS);
		}
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
}
void nas_task_init()
{
	xTaskCreate(&nas_task, "nas_task", 12288, NULL, 4, NULL);
}