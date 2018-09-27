#ifndef _DMA_H_
#define _DMA_H_

#include <stm32f4xx.h>


/*
 * DMA 传输完成时的回调函数
*/
extern void (*i2s_dma_tx_callback)(void);
extern void (*i2s_dma_rx_callback)(void);

void i2s_dma_tx_start(void);
void i2s_dma_tx_stop(void);
void i2s_dma_rx_start(void);
void i2s_dma_rx_stop(void);
void i2s_dma_set_tx_mode(uint8_t *dma_buffer0, uint8_t *dma_buffer1, uint16_t dma_buffer_size);
void i2s_dma_set_rx_mode(uint8_t *dma_buffer0, uint8_t *dma_buffer1, uint16_t dma_buffer_size);




#endif

