#include "thread_wifi.h"
#include "config.h"
#include "fatfserr.h"

#define MAX_SIZE 1024

//���豸�ж�ȡsize�ֽڣ������������кͻس���ֵ��������
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

//���ַ�����Ѱ���ض������ַ������ҵ�����1�� �Ҳ�������0 
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
	if (check != NULL) { 	//��Ҫ����������Ӧ��
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
				err("�ȴ�������Ӧ��ʱ...\r\n");
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

//�����ַ��������������ȷ����ַ������ֽ�����Ȼ�����ַ���
void send_string_to_server(struct rt_device *device, const char *buf, int cnt, const char *check)
{
	char tmp[8];
	int32_t ret;
	
	rt_memset(tmp, 0, 8);
	ret = rt_snprintf(tmp, 8, "%d", cnt);
//	debug("��һ�����ݴ�С: %s\r\n", tmp);
	if (write_data_to_wifi(device, tmp, ret, "OK") != 0) {
		err("������Ҫ���͵��ֽ���ʧ��!\r\n");
		goto timeout;
	}
	else
//		debug("������Ҫ���͵��ֽ����ɹ�!\r\n\r\n");
	
	if (write_data_to_wifi(device, buf, cnt, check) != 0) {
		err("��������ʧ�ܣ�\r\n");
		goto timeout;
	}
	else
//		debug("�������ݳɹ�!\r\n\r\n");
	return;
	
timeout:
	if (write_data_to_wifi(device, "+++", 3, NULL) != 0) {
		err("����͸��ģʽʧ��!\r\n");
	}
	else
		debug("����͸��ģʽ�ɹ���\r\n");
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
		err("�򿪸�Ŀ¼ʧ��\r\n");
	}
	
	if ((res = f_open(&fp, name, FA_OPEN_EXISTING | FA_READ)) != FR_OK) {
		err("��%s�ļ�ʧ��! %s(res: %d)\r\n", name, fserr(res), res);
	}
	
	rt_memset(buffer, 0, MAX_SIZE);
	ret = rt_snprintf(buffer, MAX_SIZE, "UPLOADgujun.csv");
	send_string_to_server(device, buffer, ret,  "OK");
	debug("��ʼ�ļ��ϴ�\r\n");
	while (1) {
		rt_memset(buffer, 0, MAX_SIZE);
		if ((res = f_read(&fp, buffer, MAX_SIZE, &br)) != FR_OK) {
			err("��%s�ļ�ʧ��! %s(res: %d)\r\n", name, fserr(res), res);
			ret = -1;
			break;
		}
		total += br;
//		debug("�ļ����ݶ�ȡ����: \r\n");
//		print_buffer(buffer, br);
//		debug("\r\n");
		send_string_to_server(device, buffer, br, "OK");
		
		debug("�Ѿ��ϴ�%dKByte����\r\n", total/1024);
		if (br != MAX_SIZE) {
			debug("�ļ��Ѿ����꣬ �ļ���С:%dKB", total/1024);
			ret = 0;
			break;
		}
	} 
	send_string_to_server(device, "END", 3, "OK");	
	debug("�����ļ��ϴ�!\r\n");
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
		err("WIFI ��λʧ��!\r\n");
		while (1);
	}
	debug("WIFI ��λ�ɹ�!\r\n");
	
	rt_memset(cmd, 0, 128);
	ret = rt_snprintf(cmd, 128, "AT+CWQAP\r\n");
	if (write_data_to_wifi(device, cmd, ret, "OK") != 0) {
		err("�Ͽ�ԭ����WIFI����ʧ��!\r\n");
		while (1);
	}
	debug("��WIFI�Ͽ����ӳɹ���\r\n");
	
	rt_memset(cmd, 0, 128);
	ret = rt_snprintf(cmd, 128, "AT+CWJAP=\"C2-403\",\"403403403\"\r\n");
	if (write_data_to_wifi(device, cmd, ret, "OK") != 0) {
		err("����WIFIʧ�ܣ�\r\n");
		while (1);
	}
	debug("����WIFI�ɹ���\r\n");
	
	
	rt_memset(cmd, 0, 128);
	ret = rt_snprintf(cmd, 128, "AT+CIPSTART=\"TCP\",\"120.76.234.108\",6666\r\n");
	if (write_data_to_wifi(device, cmd, ret, "OK") != 0) {
		err("���ӵ�������ʧ��!\r\n");
		while (1);
	}
	debug("���ӵ�������!\r\n");
	
	
	rt_memset(cmd, 0, 128);
	ret = rt_snprintf(cmd, 128, "AT+CIPMODE=1\r\n");
	if (write_data_to_wifi(device, cmd, ret, "OK") != 0) {
		err("���ͽ���͸��ģʽʧ��!\r\n");
		while (1);
	}
	debug("���ͽ���͸��ģʽ�ɹ���\r\n");
	
	rt_memset(cmd, 0, 128);
	ret = rt_snprintf(cmd, 128, "AT+CIPSEND\r\n");
	if (write_data_to_wifi(device, cmd, ret, NULL) != 0) {
		err("���÷���ģʽʧ��!\r\n");
		while (1);
	}
	debug("���÷���ģʽ�ɹ���\r\n");
	
	
	send_string_to_server(device, "GETTIME", 7, "2016");
	debug("��ȡʱ��ɹ���\r\n");
	
	sleep(3);
	upload_file(device, "/data.csv");
//	upload_file_test(device);
	
	rt_memset(cmd, 0, 128);
	ret = rt_snprintf(cmd, 128, "+++");
	if (write_data_to_wifi(device, cmd, ret, NULL) != 0) {
		err("����͸��ģʽʧ��!\r\n");
		while (1);
	}
	debug("����͸��ģʽ�ɹ���\r\n");
	sleep(1);
	
	rt_memset(cmd, 0, 128);
	ret = rt_snprintf(cmd, 128, "AT+CIPCLOSE\r\n");
	if (write_data_to_wifi(device, cmd, ret, "OK") != 0) {
		err("�ر�TCPʧ��!\r\n");
		while (1);
	}
	debug("�ر�TCP�ɹ���\r\n");
	
	while (1) {
		debug(">>>\r\n");
		sleep(1);
	}
}




