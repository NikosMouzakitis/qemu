/*
 * Kendryte K230 (minimal QEMU machine model)
 *
 * This is a reference-style machine, not full SoC emulation.
 */

#include "qemu/osdep.h"
#include "qemu/units.h"
#include "qapi/error.h"
#include "qemu/error-report.h"
#include "hw/core/boards.h"
#include "hw/core/loader.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/boot.h"
#include "target/riscv/cpu.h"
#include "hw/intc/sifive_plic.h"
#include "hw/intc/riscv_aclint.h"
#include "system/system.h"
#include "hw/core/qdev-properties.h"
#include "system/address-spaces.h"
#include "hw/riscv/kendryte_k230.h"


static const struct MemmapEntry {
    hwaddr base;
    hwaddr size;
} k230_memmap[] = {
    [K230_ROM]   =  {  0x00001000,  0x2000   },
    [K230_RAM]   =  {  0x80000000,  0x0      },
    [K230_UART]  =  {  0x00011300,  0x00040  },
    [K230_GPIO]  =  {  0x020d0000,  0x00100  },
    [K230_PLIC]  =  {  0x0c000000,  0x20000  },
    [K230_CLINT] =  {  0x02000000,  0xc0000  },
    [K230_I2C]   =  {  0x20c00000,  0x00100  },
};

static void k230_machine_state_init(MachineState *mstate)
{
    K230MachineState *sms = RISCV_K230_MACHINE(mstate);
    MemoryRegion *system_memory = get_system_memory();
    hwaddr firmware_load_addr = k230_memmap[K230_RAM].base;

    /* Initialize SoC */
    object_initialize_child(OBJECT(mstate), "soc", &sms->soc,
                            TYPE_RISCV_K230_SOC);
    qdev_realize(DEVICE(&sms->soc), NULL, &error_abort);

    /* register RAM */
    memory_region_add_subregion(system_memory,
                                k230_memmap[K230_RAM].base,
                                mstate->ram);

    if (mstate->firmware) {
        riscv_load_firmware(mstate->firmware, &firmware_load_addr, NULL);
    }

    /* ROM reset vector */
    riscv_setup_rom_reset_vec(mstate, &sms->soc.cpus, firmware_load_addr,
                              k230_memmap[K230_ROM].base,
                              k230_memmap[K230_ROM].size, 0, 0);
}

static void k230_machine_instance_init(Object *obj)
{
}
static void k230_machine_class_init(ObjectClass *klass, const void *data)
{
    MachineClass *mc = MACHINE_CLASS(klass);
    static const char * const valid_cpu_types[] = {
        RISCV_CPU_TYPE_NAME("k230"),
        NULL
    };

    mc->desc = "K2300-kendryte test";
    mc->init = k230_machine_state_init;
    mc->default_cpu_type = TYPE_RISCV_CPU_K230;
    mc->valid_cpu_types = valid_cpu_types;
    mc->default_ram_id = "riscv.k230.ram";
}


static const TypeInfo k230_machine_type_info = {
    .name = TYPE_RISCV_K230_MACHINE,
    .parent = TYPE_MACHINE,
    .class_init = k230_machine_class_init,
    .instance_init = k230_machine_instance_init,
    .instance_size = sizeof(K230MachineState),
};

static void k230_machine_type_info_register(void)
{
    type_register_static(&k230_machine_type_info);
}

type_init(k230_machine_type_info_register)



static void k230_soc_state_realize(DeviceState *dev, Error **errp)
{
    MachineState *ms = MACHINE(qdev_get_machine());
    K230SoCState *sss = RISCV_K230_SOC(dev);
    MemoryRegion *system_memory = get_system_memory();

    sysbus_realize(SYS_BUS_DEVICE(&sss->cpus), &error_abort);

    sss->plic = sifive_plic_create(k230_memmap[K230_PLIC].base,
        (char *)K230_PLIC_HART_CONFIG, ms->smp.cpus, 0,
        K230_PLIC_NUM_SOURCES,
        K230_PLIC_NUM_PRIORITIES,
        K230_PLIC_PRIORITY_BASE,
        K230_PLIC_PENDING_BASE,
        K230_PLIC_ENABLE_BASE,
        K230_PLIC_ENABLE_STRIDE,
        K230_PLIC_CONTEXT_BASE,
        K230_PLIC_CONTEXT_STRIDE,
        k230_memmap[K230_PLIC].size);

    riscv_aclint_swi_create(k230_memmap[K230_CLINT].base,
        0, 1, false);
    riscv_aclint_mtimer_create(k230_memmap[K230_CLINT].base +
            RISCV_ACLINT_SWI_SIZE,
        RISCV_ACLINT_DEFAULT_MTIMER_SIZE, 0, 1,
        RISCV_ACLINT_DEFAULT_MTIMECMP, RISCV_ACLINT_DEFAULT_MTIME,
        RISCV_ACLINT_DEFAULT_TIMEBASE_FREQ, false);

    qdev_prop_set_chr(DEVICE(&(sss->uart)), "chardev", serial_hd(0));
    if (!sysbus_realize(SYS_BUS_DEVICE(&sss->uart), errp)) {
        return;
    }
    sysbus_mmio_map(SYS_BUS_DEVICE(&sss->uart), 0,
                    k230_memmap[K230_UART].base);

    /* ROM */
    memory_region_init_rom(&sss->rom, OBJECT(dev), "riscv.k230.rom",
                           k230_memmap[K230_ROM].size, &error_fatal);
    memory_region_add_subregion(system_memory,
        k230_memmap[K230_ROM].base, &sss->rom);


}

static void k230_soc_class_init(ObjectClass *oc, const void *data)
{
	DeviceClass *dc = DEVICE_CLASS(oc);
	dc->realize = k230_soc_state_realize;	
        dc->user_creatable = false;
}

static void k230_soc_instance_init(Object *obj)
{
    K230SoCState *sss = RISCV_K230_SOC(obj);

    object_initialize_child(obj, "cpus", &sss->cpus, TYPE_RISCV_HART_ARRAY);
    object_initialize_child(obj, "uart", &sss->uart, TYPE_K230_UART);

    /*
     * CPU type is fixed and we are not supporting passing from commandline yet.
     * So let it be in instance_init. When supported should use ms->cpu_type
     * instead of TYPE_RISCV_CPU_K230
     */
    object_property_set_str(OBJECT(&sss->cpus), "cpu-type",
                            TYPE_RISCV_CPU_K230, &error_abort);
    object_property_set_int(OBJECT(&sss->cpus), "num-harts", 1,
                            &error_abort);

}
static const TypeInfo k230_typeinfo = {
    .name          = TYPE_RISCV_K230_SOC,
    .parent        = TYPE_DEVICE,
    .class_init    = k230_soc_class_init,
    .instance_init = k230_soc_instance_init,
    .instance_size = sizeof(K230SoCState),
};

static void k230_type_info_register(void)
{
    printf("%s:[%d]\n",__func__,__LINE__);
    type_register_static(&k230_typeinfo);
    printf("%s:[%d]\n",__func__,__LINE__);
}


type_init(k230_type_info_register);
