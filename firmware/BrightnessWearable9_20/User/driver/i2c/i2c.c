/*
 * File      : i2c.c
 * GPIO 模拟I2C 
 */
#include "i2c.h"
#include "config.h"

/* 该函数仅供i2c内部控制时序使用, 阻塞！！！ */
static void i2c_delay(void)
{
	uint8_t i;
/*　
	CPU主频168MHz时，在内部Flash运行, MDK工程不优化。用台式示波器观测波形。
	循环次数为i = 5时，SCL频率 = 1.78MHz (读耗时: 92ms, 读写正常，但是用示波器探头碰上就读写失败。时序接近临界)
	循环次数为i = 10时，SCL频率 = 1.1MHz (读耗时: 138ms, 读速度: 118724B/s)
	循环次数为i = 30时，SCL频率 = 440KHz， SCL高电平时间1.0us，SCL低电平时间1.2us
	上拉电阻选择2.2K欧时，SCL上升沿时间约0.5us，如果选4.7K欧，则上升沿约1us
	实际应用选择400KHz左右的速率即可
*/
	for (i = 0; i < 50; i++);	
}

/* 
 * i2c 起始信号， 开始之前先检查一下i2c是否空闲
 */
int32_t i2c_start(i2c_bus_t *i2c)
{	
	pin_t *scl = &i2c->pin.scl;
	pin_t *sda = &i2c->pin.sda;
	
	assert(i2c);
	/* 当SCL高电平时，SDA出现一个下跳沿表示I2C总线启动信号 */
	PIN_SET(sda);	
	PIN_SET(scl);
	i2c_delay();
	/* 检查I2C是否空闲 */
	if (!(PIN_GET(scl) && PIN_GET(sda))) {
		err("i2c bus is not idle!\r\n");
		return -1;
	}
	PIN_CLR(sda);
	i2c_delay();
	PIN_CLR(scl);
	i2c_delay();
	return 0;
}	

/* 
 * i2c结束信号
 */
int32_t i2c_stop(i2c_bus_t *i2c)
{
	pin_t *scl = &i2c->pin.scl;
	pin_t *sda = &i2c->pin.sda;
	
	assert(i2c);
	/* 当SCL高电平时，SDA出现一个上跳沿表示I2C总线停止信号 */
	PIN_CLR(sda);
	PIN_SET(scl);
	i2c_delay();
	PIN_SET(sda);
	return 0;
}

/* 
 * 向i2c设备发送一个字节
*/
int32_t i2c_send_byte(i2c_bus_t *i2c, uint8_t val)
{
	uint8_t i;
	pin_t *scl = &i2c->pin.scl;
	pin_t *sda = &i2c->pin.sda;
	
	assert(i2c);
	/* 先发送字节的高位bit7 */
	for (i = 0; i < 8; i++) {
		if (val & 0x80)
			PIN_SET(sda);
		else
			PIN_CLR(sda);
		i2c_delay();
		PIN_SET(scl);
		i2c_delay();
		PIN_CLR(scl);
		if (i == 7)
			PIN_SET(sda);
		val <<= 1;	/* 左移一个bit */
		i2c_delay();
	}
	return 0;
}

/*
 * 从i2c设备读一个字节
*/
int32_t i2c_recv_byte(i2c_bus_t *i2c, uint8_t *val)
{
	uint8_t i;
	pin_t *scl = &i2c->pin.scl;
	pin_t *sda = &i2c->pin.sda;
	
	assert(i2c);
	/* 读到第1个bit为数据的bit7 */
	*val = 0;
	for (i = 0; i < 8; i++) {
		*val <<= 1;
		PIN_SET(scl);
		i2c_delay();
		if (PIN_GET(sda))
			(*val)++;
		PIN_CLR(scl);
		i2c_delay();
	}
	return 0;
}

/* 
 * 连续写n个字节到i2c设备中, val为待写的首地址 
*/
int32_t i2c_send_nbytes(i2c_bus_t *i2c, uint8_t n, uint8_t *val)
{
	uint8_t i;
	
	assert(i2c);
	assert(val);
	for (i = 0; i < n; i++) {
		if (i2c_send_byte(i2c, *val) != 0) {
			err("send %dth byte error!\r\n", i);
			return -1;
		} else 
			val++;
	}
	return 0;
}

/* 
 * 从i2c设备中连续读取n个字节，val为读取的首地址 
*/
int32_t i2c_recv_nbytes(i2c_bus_t *i2c, uint8_t n, uint8_t *val)
{
	uint8_t i;
	
	assert(i2c);
	assert(val);
	for (i = 0; i < n; i++) {
		if (i2c_recv_byte(i2c, val) != 0) {
			err("recv %dth byte error!\r\n", i);
			return -1;
		} else 
			val++;
	}
	return 0;
}

int32_t i2c_wait_ack(i2c_bus_t *i2c)
{
	uint16_t timeout = 0;
	pin_t *scl = &i2c->pin.scl;
	pin_t *sda = &i2c->pin.sda;
	
	assert(i2c);
	PIN_SET(sda);	/* CPU释放SDA总线 */
	i2c_delay();
	PIN_SET(scl);	/* CPU驱动SCL = 1, 此时器件会返回ACK应答 */
	i2c_delay();	
	
	while (PIN_GET(sda)) {	
		timeout++;
		if (timeout >= I2C_WAIT_ACK_TIMEOUT)			/* 超时 */
			goto err_timeout;
	}
	PIN_CLR(scl);
	i2c_delay();
	return I2C_ACK;
	
err_timeout:
	err("i2c wait ack time out\r\n");
	i2c_stop(i2c);
	return I2C_NACK;
}

int32_t i2c_send_ack(i2c_bus_t *i2c)
{
	pin_t *scl = &i2c->pin.scl;
	pin_t *sda = &i2c->pin.sda;
	
	assert(i2c);
	PIN_CLR(sda);	/* CPU驱动SDA = 0 */
	i2c_delay();
	PIN_SET(scl);	/* CPU产生1个时钟 */
	i2c_delay();
	PIN_CLR(scl);
	i2c_delay();
	PIN_SET(sda);	/* CPU释放SDA总线 */
	return 0;
}

int32_t i2c_send_nack(i2c_bus_t *i2c)
{
	pin_t *scl = &i2c->pin.scl;
	pin_t *sda = &i2c->pin.sda;
	
	assert(i2c);
	PIN_SET(sda);	/* CPU驱动SDA = 1 */
	i2c_delay();
	PIN_SET(scl);	/* CPU产生1个时钟 */
	i2c_delay();
	PIN_CLR(scl);
	i2c_delay();
	return 0;
}

/* 将i2c 的引脚注册操作到i2c中 */
int32_t i2c_bus_register(i2c_bus_t *i2c,  GPIO_TypeDef *port, uint32_t scl, uint32_t sda)
{
	assert(i2c);
	/* 注册SCL引脚 */
	if (pin_register(&i2c->pin.scl, port, scl, OUT, OD, NP) != 0) {
		err("i2c_bus_register scl pin failed!\r\n");
		return -1;
	} 
	/* 注册SDA引脚 */
	if (pin_register(&i2c->pin.sda, port, sda, OUT, OD, NP) != 0) {
		err("i2c_bus_register sda pin failed!\r\n");
		return -1;
	} 	
	
	i2c->start 			= i2c_start;
	i2c->restart 		= i2c_start;
	i2c->stop 			= i2c_stop;
	i2c->send_ack   	= i2c_send_ack;
	i2c->send_nack  	= i2c_send_nack;
	i2c->send_byte  	= i2c_send_byte;
	i2c->send_nbytes 	= i2c_send_nbytes;
	i2c->wait_ack 		= i2c_wait_ack;
	i2c->recv_byte  	= i2c_recv_byte;
	i2c->recv_nbytes 	= i2c_recv_nbytes;
	
	i2c->stop(i2c);		/* 总线停止 */
	return 0;
}
