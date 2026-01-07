#ifndef HW_K230_UART_H
#define HW_K230_UART_H

#include "hw/core/sysbus.h"
#include "chardev/char-fe.h"

#define K230_UART_BAUD        0x00
#define K230_UART_TX          0x04
#define K230_UART_RX          0x08
#define K230_UART_STATUS      0x0C
#define K230_UART_DELAY       0x10
#define K230_UART_CONTROL     0x14
#define K230_UART_INT_EN      0x18
#define K230_UART_IQ_CYCLES   0x1C
#define K230_UART_RX_THRES    0x20

#define K230_UART_STATUS_TX_EMPTY     (1 << 0)
#define K230_UART_STATUS_TX_FULL      (1 << 1)
#define K230_UART_STATUS_RX_NOT_EMPTY (1 << 2)
#define K230_UART_STATUS_RX_FULL      (1 << 3)
/* 9600 8N1 is the default setting */
/* Reg value = (50000000 Hz)/(16 * 9600)*/
#define K230_UART_BAUD_DEFAULT    0x0145
#define K230_UART_CONTROL_DEFAULT 0x0100

#define TYPE_K230_UART "k230-uart"
#define K230_UART(obj) \
    OBJECT_CHECK(K230UartState, (obj), TYPE_K230_UART)

typedef struct {
    /* <private> */
    SysBusDevice parent_obj;

    /* <public> */
    MemoryRegion mmio;

    uint32_t uart_baud;
    uint32_t uart_tx;
    uint32_t uart_rx;
    uint32_t uart_status;
    uint32_t uart_delay;
    uint32_t uart_control;
    uint32_t uart_interrupt;
    uint32_t uart_iq_cycles;
    uint32_t uart_rx_threshold;
    qemu_irq irq; 
    CharFrontend chr;
} K230UartState;

#endif /* HW_K230_UART_H */
