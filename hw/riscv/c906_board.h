#ifndef HW_RISCV_C906_BOARD_H
#define HW_RISCV_C906_BOARD_H

#include "hw/core/boards.h"

#define TYPE_C906_BOARD MACHINE_TYPE_NAME("c906_board")

typedef enum {
    C906_DEV_RAM,
    C906_DEV_ROM,
    C906_DEV_CLINT,
    C906_DEV_PLIC,
    C906_DEV_UART0,
    C906_DEV_TEST,
    C906_DEV_MAX
} C906DeviceType;

#endif
