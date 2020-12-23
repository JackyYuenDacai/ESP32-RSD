#include "parser_func.h"

char *get_date(char *http_response, int rlen)
{
	static char ret_url[128] = {0};
	char html_status[32] = {0};
	int i = 0, j = 0;
	bzero(ret_url, sizeof(char)*128);
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
	parse_time2tm(&remote_ti, ret_url);
	time_t sec = mktime(&remote_ti); // make time_t
	ESP_LOGI(TAG, "Y:%d M:%d D:%d", remote_ti.tm_year, remote_ti.tm_mon, remote_ti.tm_mday);

	if (timeinfo.tm_year < (2016 - 1900) || abs(timeinfo.tm_sec - remote_ti.tm_sec) >= 5)
	{
		time_t t = mktime(&remote_ti);
		printf("Set time: %s ;TV_SEC: %ld", asctime(&remote_ti), sec);
		struct timeval now = {.tv_sec = t};
		now.tv_usec = 0;
		settimeofday(&now, NULL);
	}
	return ret_url;
}

char *get_login_url(char *http_response)
{
	static char ret_url[128] = {0};
	char html_status[32] = {0};
	int i = 0, j = 0;
	int line_len[10] = {0};
	int cl = 0;
	int rlen = strlen(http_response);
	bzero(ret_url, sizeof(char)*128);
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

uint8_t parse_time2tm(struct tm *time, char *time_string)
{
	const char *wd_abrev[7] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	const char *mt_abrev[12] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	char wd_buf[16] = {0};
	char md_buf[8] = {0};
	char mt_buf[16] = {0};
	char yr_buf[8] = {0};
	char arbi_buf[6] = {0};
	//char tz_buf[8] = {0};
	int i = 0, j = 0, len = 0;
	if (time == NULL)
		return -1;
	while (time_string[i++] != '\0')
	{
		if (i >= 40)
			return -1;
	}
	if ((len = strlen(time_string)) <= 1)
		return -1;

	i = 0;
	j = 0;
	while (time_string[i] != ',' && i < len)
		wd_buf[j++] = time_string[i++];

	i++;
	j = 0;
	while (time_string[i] == ' ' && i < len)
		i++;
	while (time_string[i] != ' ' && i < len)
		md_buf[j++] = time_string[i++];
	time->tm_mday = atoi(md_buf);

	i++;
	j = 0;
	while (time_string[i] == ' ' && i < len)
		i++;
	while (time_string[i] != ' ' && i < len)
		mt_buf[j++] = time_string[i++];

	i++;
	j = 0;
	while (time_string[i] == ' ' && i < len)
		i++;
	while (time_string[i] != ' ' && i < len)
		yr_buf[j++] = time_string[i++];
	time->tm_year = atoi(yr_buf) - 1900;
	i++;
	j = 0;
	while (time_string[i] == ' ' && i < len)
		i++;
	while (time_string[i] != ':' && i < len)
		arbi_buf[j++] = time_string[i++];
	arbi_buf[j] = 0;
	time->tm_hour = atoi(arbi_buf);

	i++;
	j = 0;
	while (time_string[i] == ' ' && i < len)
		i++;
	while (time_string[i] != ':' && i < len)
		arbi_buf[j++] = time_string[i++];
	arbi_buf[j] = 0;
	time->tm_min = atoi(arbi_buf);

	i++;
	j = 0;
	while (time_string[i] == ' ' && i < len)
		i++;
	while (time_string[i] != ' ' && i < len)
		arbi_buf[j++] = time_string[i++];
	arbi_buf[j] = 0;
	time->tm_sec = atoi(arbi_buf);

	//Parse month
	for (i = 0; i < 12; i++)
	{
		if (strstr(mt_buf, mt_abrev[i]) != NULL)
		{
			time->tm_mon = i;
			break;
		}
	}

	//Parse weekday
	for (i = 0; i < 7; i++)
	{
		if (strstr(wd_buf, wd_abrev[i]) != NULL)
		{
			time->tm_wday = i;
			break;
		}
	}

	//ESP_LOGI(TAG,"TBUF: %s %s %s %s",wd_buf,md_buf,mt_buf,yr_buf);
	ESP_LOGI(TAG, "TPAR: %d %d %d %d %d %d", time->tm_mday, time->tm_mon, time->tm_year + 1900, time->tm_hour, time->tm_min, time->tm_sec);
	return 0;
}