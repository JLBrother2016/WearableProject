#include "uart2.h"
#include "config.h"

#define UART2_GPIO_TX       GPIO_Pin_2
#define UART2_TX_PIN_SOURCE GPIO_PinSource2
#define UART2_GPIO_RX       GPIO_Pin_3
#define UART2_RX_PIN_SOURCE GPIO_PinSource3
#define UART2_GPIO          GPIOA
#define UART2_GPIO_RCC      RCC_AHB1Periph_GPIOA
#define RCC_APBPeriph_UART2 RCC_APB1Periph_USART2
#define UART2_TX_DMA        DMA1_Channel4
#define UART2_RX_DMA        DMA1_Channel5

struct uart2_buffer rx_buf;


void uart2_config(void)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	rx_buf.pos = 0;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);//ʹ��USART1ʱ��
	
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(UART2_GPIO, UART2_TX_PIN_SOURCE, GPIO_AF_USART2); //GPIOA9����ΪUSART1
	GPIO_PinAFConfig(UART2_GPIO, UART2_RX_PIN_SOURCE, GPIO_AF_USART2); //GPIOA10����ΪUSART1
	
	//USART1�˿�����
	GPIO_InitStructure.GPIO_Pin = UART2_GPIO_TX | UART2_GPIO_RX; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(UART2_GPIO, &GPIO_InitStructure); //��ʼ��PA9��PA10
	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//��������ж�
		//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����
	
	USART_Cmd(USART2, ENABLE);
}

static int uart2_putc(char c)
{
	//�������ݼĴ�����
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
    USART2->DR = c;

    return 1;
}

static int uart2_getc(void)
{
    int ch;

    ch = -1;
    if (USART2->SR & USART_FLAG_RXNE)
        ch = USART2->DR & 0xff;

    return ch;
}

void hw_serial_rx(void)
{
	int ch = -1;
	rt_base_t level;
	
	while (1) {
		ch = uart2_getc();
		if (ch == -1)
			break;
		
		/* disable interrupt */
        level = rt_hw_interrupt_disable();
		rx_buf.buffer[rx_buf.pos] = ch;
		rx_buf.pos += 1;
		/* enable interrupt */
		rt_hw_interrupt_enable(level);
	}
}

int usart_send(char *buf, int length)
{
	int size = length;
	while (length) {
		if (uart2_putc(*(char *)buf) == -1) {
			//�ȴ�
			continue;
		}
		buf++; length--;
	}
	
	return size - length;
}


void USART2_IRQHandler(void)
{
    
    /* enter interrupt */
    rt_interrupt_enter();
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //�����ж�
    {
		hw_serial_rx();//��������
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);/* clear interrupt */
    }
    if (USART_GetITStatus(USART2, USART_IT_TC) != RESET)//�����ж�
    {
        /* clear interrupt */
        USART_ClearITPendingBit(USART2, USART_IT_TC);
    }

    /* leave interrupt */
    rt_interrupt_leave();
}
