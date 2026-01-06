#ifndef HW_SHAKTI_C_H
#define HW_SHAKTI_C_H

#include "hw/riscv/riscv_hart.h"
#include "hw/core/boards.h"
#include "hw/char/k230_uart.h"

#define TYPE_RISCV_K230_SOC "riscv.k230.class.soc"
#define RISCV_K230_SOC(obj) \
    OBJECT_CHECK(K230SoCState, (obj), TYPE_RISCV_K230_SOC)

typedef struct K230SoCState {
    /*< private >*/
    DeviceState parent_obj;

    /*< public >*/
    RISCVHartArrayState cpus;
    DeviceState *plic;
    K230UartState uart;
    MemoryRegion rom;

} K230SoCState;

#define TYPE_RISCV_K230_MACHINE MACHINE_TYPE_NAME("k230")
#define RISCV_K230_MACHINE(obj) \
    OBJECT_CHECK(K230MachineState, (obj), TYPE_RISCV_K230_MACHINE)
typedef struct K230MachineState {
    /*< private >*/
    MachineState parent_obj;

    /*< public >*/
    K230SoCState soc;
} K230MachineState;

enum {
    K230_ROM,
    K230_RAM,
    K230_UART,
    K230_GPIO,
    K230_PLIC,
    K230_CLINT,
    K230_I2C,
};

#define K230_PLIC_HART_CONFIG "MS"
/* Including Interrupt ID 0 (no interrupt)*/
#define K230_PLIC_NUM_SOURCES 28
/* Excluding Priority 0 */
#define K230_PLIC_NUM_PRIORITIES 2
#define K230_PLIC_PRIORITY_BASE 0x00
#define K230_PLIC_PENDING_BASE 0x1000
#define K230_PLIC_ENABLE_BASE 0x2000
#define K230_PLIC_ENABLE_STRIDE 0x80
#define K230_PLIC_CONTEXT_BASE 0x200000
#define K230_PLIC_CONTEXT_STRIDE 0x1000

#endif
