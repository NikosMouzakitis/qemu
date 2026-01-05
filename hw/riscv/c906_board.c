/*
 * T-HEAD C906 RISC-V Board
 * Based on Shakti C board
 */

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/core/boards.h"
#include "hw/core/loader.h"
#include "hw/riscv/boot.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/char/serial.h"
#include "target/riscv/cpu.h"
#include "qemu/osdep.h"
#include "hw/core/sysbus.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/boot.h"
#include "hw/char/serial-mm.h"
#include "target/riscv/cpu.h"
#include "qapi/error.h"
#include "qemu/units.h"
#include "qemu/error-report.h"
#include "system/address-spaces.h"


#define TYPE_C906_BOARD MACHINE_TYPE_NAME("c906")
#define C906_RAM_SIZE     0x8000000   /* 128 MB */

static void c906_simple_init(MachineState *machine)
{
    /* Create CPU */
    RISCVHartArrayState *soc = RISCV_HART_ARRAY(object_new(TYPE_RISCV_HART_ARRAY));

    object_property_set_str(OBJECT(soc), "cpu-type", RISCV_CPU_TYPE_NAME("rv64"), &error_fatal);
    object_property_set_int(OBJECT(soc), "num-harts", machine->smp.cpus, &error_fatal);
    object_property_set_int(OBJECT(soc), "hartid-base", 0, &error_fatal);

    sysbus_realize(SYS_BUS_DEVICE(soc), &error_fatal);

    /* Create RAM */
    MemoryRegion *system_memory = get_system_memory();
    MemoryRegion *ram = g_new(MemoryRegion, 1);

    memory_region_init_ram(ram, NULL, "c906.ram",
                           C906_RAM_SIZE, &error_fatal);
    memory_region_add_subregion(system_memory, 0x80000000, ram);

    /* Load kernel using simple API */
        /* Load kernel */
    if (machine->kernel_filename) {
        RISCVBootInfo binfo;

        /* Initialize boot info */
        riscv_boot_info_init(&binfo, soc);

        /* Calculate kernel start address */
        vaddr kernel_start_addr = riscv_calc_kernel_start_addr(&binfo, 0);

        /* Load kernel with all required arguments */
        riscv_load_kernel(machine,          /* MachineState *machine */
                          &binfo,           /* RISCVBootInfo *info */
                          kernel_start_addr, /* vaddr kernel_start_addr */
                          true,             /* bool load_initrd */
                          NULL);            /* symbol_fn_t sym_cb */
    }

}

static void c906_simple_machine_class_init(ObjectClass *oc, const void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "T-HEAD C906 RISC-V Board (Simple)";
    mc->init = c906_simple_init;
    mc->max_cpus = 1;
    mc->default_cpus = 1;
    mc->default_ram_size = C906_RAM_SIZE;
    mc->default_ram_id = "c906.ram";
}

static const TypeInfo c906_simple_machine_typeinfo = {
    .name = MACHINE_TYPE_NAME("c906_simple"),
    .parent = TYPE_MACHINE,
    .class_init = c906_simple_machine_class_init,
};

static void c906_simple_machine_register_types(void)
{
    type_register_static(&c906_simple_machine_typeinfo);
}

type_init(c906_simple_machine_register_types)
