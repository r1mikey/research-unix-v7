#ifndef BCM283X_DMA_H
#define BCM283X_DMA_H

#include "../kstdint.h"
#include "../kstddef.h"

#define DMAC_DRQ_DEVICE_NONE            0
#define DMAC_DRQ_DEVICE_DSI             1
#define DMAC_DRQ_DEVICE_PCM_TX          2
#define DMAC_DRQ_DEVICE_PCM_RX          3
#define DMAC_DRQ_DEVICE_SMI             4
#define DMAC_DRQ_DEVICE_PWM             5
#define DMAC_DRQ_DEVICE_SPI_TX          6
#define DMAC_DRQ_DEVICE_SPI_RX          7
#define DMAC_DRQ_DEVICE_SPI_SLAVE_TX    8   /* BSC/SPI Slave TX */
#define DMAC_DRQ_DEVICE_SPI_SLAVE_RX    9   /* BSC/SPI Slave RX */
#define DMAC_DRQ_DEVICE_UNUSED          10  /* Entry 10 is unused */
#define DMAC_DRQ_DEVICE_EMMC            11
#define DMAC_DRQ_DEVICE_UART_TX         12
#define DMAC_DRQ_DEVICE_SD_HOST         13
#define DMAC_DRQ_DEVICE_UART_RX         14
#define DMAC_DRQ_DEVICE_DSI_ALT         15
#define DMAC_DRQ_DEVICE_SLIMBUS_MCTX    16
#define DMAC_DRQ_DEVICE_HDMI            17
#define DMAC_DRQ_DEVICE_SLIMBUS_MCRX    18
#define DMAC_DRQ_DEVICE_SLIMBUS_DC0     19
#define DMAC_DRQ_DEVICE_SLIMBUS_DC1     20
#define DMAC_DRQ_DEVICE_SLIMBUS_DC2     21
#define DMAC_DRQ_DEVICE_SLIMBUS_DC3     22
#define DMAC_DRQ_DEVICE_SLIMBUS_DC4     23
#define DMAC_DRQ_DEVICE_SCALER_FIFO_0   24
#define DMAC_DRQ_DEVICE_SCALER_FIFO_1   25
#define DMAC_DRQ_DEVICE_SCALER_FIFO_2   26
#define DMAC_DRQ_DEVICE_SLIMBUS_DC5     27
#define DMAC_DRQ_DEVICE_SLIMBUS_DC6     28
#define DMAC_DRQ_DEVICE_SLIMBUS_DC7     29
#define DMAC_DRQ_DEVICE_SLIMBUS_DC8     30
#define DMAC_DRQ_DEVICE_SLIMBUS_DC9     31
#define DMAC_DRQ_DEVICE_MAX             (DMAC_DRQ_DEVICE_SLIMBUS_DC9)

#define DMA_DIR_DEVICE_TO_MEMORY        0
#define DMA_DIR_MEMORY_TO_DEVICE        1
#define DMA_DIR_MEMORY_TO_MEMORY        2

extern int bcm283x_dma_init(void);
extern int dmastart(uint32_t c, uint32_t drqdev, uint32_t dmadir,
    void *src, void *dst, size_t len);

#endif
