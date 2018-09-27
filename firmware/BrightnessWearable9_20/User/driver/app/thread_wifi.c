#include "thread_wifi.h"
#include "config.h"
#include "fatfserr.h"

#define MAX_SIZE 1024

//重设备中读取size字节，后者遇到换行和回车中值读操作。
rt_size_t read(rt_device_t dev, rt_off_t pos, char *buffer, rt_size_t size)
{
	int32_t timeout = 0;
	rt_size_t br = 0;
	while (br != size)
	{
		br += rt_device_read(dev, 0, buffer+br, size-br);
		delay_ms(100);
		if (buffer[br-1] == '\n' || buffer[br-1] == '\r') {
			break;
		}
		timeout++;
		if (timeout >= 10) {
			break;
		}
	}
	return br;
}

//for test
void print_buffer(const char *buf, int lenght)
{
	int i;
	for (i = 0; i < lenght; i++) {
		debug("%c", buf[i]);
	}
}

//在字符串中寻找特定的子字符串，找到返回1， 找不到返回0 
int find_str(const char *src, int length, const char *str)
{
	int cur, same = 0;
	int cnt = rt_strlen(str);
	
	for (cur = 0; cur < length; cur++) {
		if (src[cur] == str[same]) {
			same++;
		} else {
			same = 0;
		}
		if (same == cnt)
			return 1;
	}
	return 0;
}


int32_t write_data_to_wifi(struct rt_device *device, const char *data, int32_t cnt, const char *check)
{
	int32_t ret;
	int32_t timeout = 0;
	char buffer[MAX_SIZE];	
	rt_device_write(device, 0, data, cnt);
	if (check != NULL) { 	//需要检查服务器的应答
		while (1) {
			rt_memset(buffer, 0, MAX_SIZE);
			ret = read(device, 0, buffer, MAX_SIZE);
			debug("received: ");
			print_buffer(buffer, ret);
			if (find_str(buffer, ret, check) == 1) {
				return 0;
			}
			timeout++;
			if (timeout >= 10) {
				err("等待服务器应答超时...\r\n");
				return -1;
			}
			sleep(1);
		}		
	}
	return 0;
}



#define isdigit(c) ((unsigned)((c) - '0') < 10)
	
int easy_atoi(const char *s)
{
	int i = 0; 
	while (isdigit(*s))
		i = i * 10 + *(s++) - '0';
	return i;
}

#define isspace(c) ((int)c == 32 || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r' )
void trim(char *s, int length)
{
	char *start, *end;
	
	start = s;
	end = s + length -1;
	
	while (1) {
		char c = *start;
		if (!isspace(c)) 
			break;
		start++;
		if (start > end) {
			s[0] = '\0';
			return;
		}
	}
	
	while (1) {
		char c = *end;
		if (!isspace(c)) {
			break;
		}
		end--;
		if (start > end) {
			s[0] = '\0';
			return;
		}
	}
	
	rt_memmove(s, start, end-start+1);
	s[end-start+1] = '\0';
}

//发送字符串给服务器，先发送字符串的字节数，然后发送字符串
void send_string_to_server(struct rt_device *device, const char *buf, int cnt, const char *check)
{
	char tmp[8];
	int32_t ret;
	
	rt_memset(tmp, 0, 8);
	ret = rt_snprintf(tmp, 8, "%d", cnt);
//	debug("下一包数据大小: %s\r\n", tmp);
	if (write_data_to_wifi(device, tmp, ret, "OK") != 0) {
		err("发送需要传送的字节数失败!\r\n");
		goto timeout;
	}
	else
//		debug("发送需要传送的字节数成功!\r\n\r\n");
	
	if (write_data_to_wifi(device, buf, cnt, check) != 0) {
		err("发送数据失败！\r\n");
		goto timeout;
	}
	else
//		debug("发送数据成功!\r\n\r\n");
	return;
	
timeout:
	if (write_data_to_wifi(device, "+++", 3, NULL) != 0) {
		err("结束透传模式失败!\r\n");
	}
	else
		debug("结束透传模式成功！\r\n");
	while (1);
}

void upload_file_test(struct rt_device *device)
{
	char buf[512];
	int32_t ret;
	rt_memset(buf, 0, 512);
	ret = rt_snprintf(buf, 512, "UPLOADtest5.txt");
	send_string_to_server(device, buf, ret,  "OK");
	rt_memset(buf, 0, 512);
	ret = rt_snprintf(buf, 512, "1. hello world, this is a test.\r\n");
	send_string_to_server(device, buf, ret,  "OK");
	rt_memset(buf, 0, 512);
	ret = rt_snprintf(buf, 512, "2. hello world, this is a test.\r\n");
	send_string_to_server(device, buf, ret,  "OK");
	rt_memset(buf, 0, 512);
	ret = rt_snprintf(buf, 512, "3. hello world, this is a test.\r\n");
	send_string_to_server(device, buf, ret,  "OK");
	rt_memset(buf, 0, 512);
	ret = rt_snprintf(buf, 512, "4. hello world, this is a test.\r\n");
	send_string_to_server(device, buf, ret,  "OK");
	rt_memset(buf, 0, 512);
	ret = rt_snprintf(buf, 512, "5. hello world, this is a test.\r\n");
	send_string_to_server(device, buf, ret,  "OK");
	
	rt_memset(buf, 0, 512);
	ret = rt_snprintf(buf, 512, "END");
	send_string_to_server(device, buf, ret,  "OK");	
}


int32_t upload_file(struct rt_device *device, const char *name)
{
	FRESULT res;
	DIR dir;
	FIL fp;
	char buffer[MAX_SIZE];
	uint32_t br;
	uint32_t total = 0;
	int32_t ret;
	
	if ((res = f_opendir(&dir, "/")) != FR_OK) {
		err("打开根目录失败\r\n");
	}
	
	if ((res = f_open(&fp, name, FA_OPEN_EXISTING | FA_READ)) != FR_OK) {
		err("打开%s文件失败! %s(res: %d)\r\n", name, fserr(res), res);
	}
	
	rt_memset(buffer, 0, MAX_SIZE);
	ret = rt_snprintf(buffer, MAX_SIZE, "UPLOADgujun.csv");
	send_string_to_server(device, buffer, ret,  "OK");
	debug("开始文件上传\r\n");
	while (1) {
		rt_memset(buffer, 0, MAX_SIZE);
		if ((res = f_read(&fp, buffer, MAX_SIZE, &br)) != FR_OK) {
			err("读%s文件失败! %s(res: %d)\r\n", name, fserr(res), res);
			ret = -1;
			break;
		}
		total += br;
//		debug("文件内容读取如下: \r\n");
//		print_buffer(buffer, br);
//		debug("\r\n");
		send_string_to_server(device, buffer, br, "OK");
		
		debug("已经上传%dKByte数据\r\n", total/1024);
		if (br != MAX_SIZE) {
			debug("文件已经读完， 文件大小:%dKB", total/1024);
			ret = 0;
			break;
		}
	} 
	send_string_to_server(device, "END", 3, "OK");	
	debug("结束文件上传!\r\n");
	return ret;
}

void rt_wifi_thread_entry(void *parameter)
{
	struct rt_device *device;
	char cmd[128];
	int ret;
	
	device = rt_device_find("uart2");
	rt_device_open(device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
	
	rt_memset(cmd, 0, 128);
	ret = rt_snprintf(cmd, 128, "AT+RST\r\n");
	if (write_data_to_wifi(device, cmd, ret, "ready") != 0) {
		err("WIFI 复位失败!\r\n");
		while (1);
	}
	debug("WIFI 复位成功!\r\n");
	
	rt_memset(cmd, 0, 128);
	ret = rt_snprintf(cmd, 128, "AT+CWQAP\r\n");
	if (write_data_to_wifi(device, cmd, ret, "OK") != 0) {
		err("断开原来的WIFI连接失败!\r\n");
		while (1);
	}
	debug("与WIFI断开连接成功！\r\n");
	
	rt_memset(cmd, 0, 128);
	ret = rt_snprintf(cmd, 128, "AT+CWJAP=\"C2-403\",\"403403403\"\r\n");
	if (write_data_to_wifi(device, cmd, ret, "OK") != 0) {
		err("连接WIFI失败！\r\n");
		while (1);
	}
	debug("连接WIFI成功！\r\n");
	
	
	rt_memset(cmd, 0, 128);
	ret = rt_snprintf(cmd, 128, "AT+CIPSTART=\"TCP\",\"120.76.234.108\",6666\r\n");
	if (write_data_to_wifi(device, cmd, ret, "OK") != 0) {
		err("连接到服务器失败!\r\n");
		while (1);
	}
	debug("连接到服务器!\r\n");
	
	
	rt_memset(cmd, 0, 128);
	ret = rt_snprintf(cmd, 128, "AT+CIPMODE=1\r\n");
	if (write_data_to_wifi(device, cmd, ret, "OK") != 0) {
		err("发送接收透传模式失败!\r\n");
		while (1);
	}
	debug("发送接收透传模式成功！\r\n");
	
	rt_memset(cmd, 0, 128);
	ret = rt_snprintf(cmd, 128, "AT+CIPSEND\r\n");
	if (write_data_to_wifi(device, cmd, ret, NULL) != 0) {
		err("设置发送模式失败!\r\n");
		while (1);
	}
	debug("设置发送模式成功！\r\n");
	
	
	send_string_to_server(device, "GETTIME", 7, "2016");
	debug("获取时间成功！\r\n");
	
	sleep(3);
	upload_file(device, "/data.csv");
//	upload_file_test(device);
	
	rt_memset(cmd, 0, 128);
	ret = rt_snprintf(cmd, 128, "+++");
	if (write_data_to_wifi(device, cmd, ret, NULL) != 0) {
		err("结束透传模式失败!\r\n");
		while (1);
	}
	debug("结束透传模式成功！\r\n");
	sleep(1);
	
	rt_memset(cmd, 0, 128);
	ret = rt_snprintf(cmd, 128, "AT+CIPCLOSE\r\n");
	if (write_data_to_wifi(device, cmd, ret, "OK") != 0) {
		err("关闭TCP失败!\r\n");
		while (1);
	}
	debug("关闭TCP成功！\r\n");
	
	while (1) {
		debug(">>>\r\n");
		sleep(1);
	}
}




