#include "qemu/osdep.h"
#include "hw/char/k230_uart.h"
#include "hw/core/qdev-properties.h"
#include "hw/core/qdev-properties-system.h"
#include "hw/core/irq.h"
#include "qemu/log.h"

static uint64_t k230_uart_read(void *opaque, hwaddr addr, unsigned size)
{
    K230UartState *s = opaque;

    switch (addr) {
    case K230_UART_BAUD:
        return s->uart_baud;
    case K230_UART_RX:
        qemu_chr_fe_accept_input(&s->chr);
        s->uart_status &= ~K230_UART_STATUS_RX_NOT_EMPTY;
        return s->uart_rx;
    case K230_UART_STATUS:
        return s->uart_status;
    case K230_UART_DELAY:
        return s->uart_delay;
    case K230_UART_CONTROL:
        return s->uart_control;
    case K230_UART_INT_EN:
        return s->uart_interrupt;
    case K230_UART_IQ_CYCLES:
        return s->uart_iq_cycles;
    case K230_UART_RX_THRES:
        return s->uart_rx_threshold;
    default:
        /* Also handles TX REG which is write only */
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: Bad offset 0x%"HWADDR_PRIx"\n", __func__, addr);
    }

    return 0;
}

static void k230_uart_write(void *opaque, hwaddr addr,
                              uint64_t data, unsigned size)
{
    K230UartState *s = opaque;
    uint32_t value = data;
    uint8_t ch;

    switch (addr) {
    case K230_UART_BAUD:
        s->uart_baud = value;
        break;
    case K230_UART_TX:
        ch = value;
        qemu_chr_fe_write_all(&s->chr, &ch, 1);
        s->uart_status &= ~K230_UART_STATUS_TX_FULL;
        break;
    case K230_UART_STATUS:
        s->uart_status = value;
        break;
    case K230_UART_DELAY:
        s->uart_delay = value;
        break;
    case K230_UART_CONTROL:
        s->uart_control = value;
        break;
    case K230_UART_INT_EN:
        s->uart_interrupt = value;
        break;
    case K230_UART_IQ_CYCLES:
        s->uart_iq_cycles = value;
        break;
    case K230_UART_RX_THRES:
        s->uart_rx_threshold = value;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: Bad offset 0x%"HWADDR_PRIx"\n", __func__, addr);
    }
}

static const MemoryRegionOps k230_uart_ops = {
    .read = k230_uart_read,
    .write = k230_uart_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .impl = {.min_access_size = 1, .max_access_size = 4},
    .valid = {.min_access_size = 1, .max_access_size = 4},
};

static void k230_uart_reset(DeviceState *dev)
{
    K230UartState *s = K230_UART(dev);

    s->uart_baud = K230_UART_BAUD_DEFAULT;
    s->uart_tx = 0x0;
    s->uart_rx = 0x0;
    s->uart_status = 0x0000;
    s->uart_delay = 0x0000;
    s->uart_control = K230_UART_CONTROL_DEFAULT;
    s->uart_interrupt = 16; //k230.dtsi
    s->uart_iq_cycles = 0x00;
    s->uart_rx_threshold = 0x00;
}

static int k230_uart_can_receive(void *opaque)
{
    K230UartState *s = opaque;

    return !(s->uart_status & K230_UART_STATUS_RX_NOT_EMPTY);
}

static void k230_uart_raise_irq(K230UartState *s)
{
    if (s->uart_status & K230_UART_STATUS_RX_NOT_EMPTY) {
        qemu_set_irq(s->irq, 1);
    } else {
        qemu_set_irq(s->irq, 0);
    }
}

static void k230_uart_receive(void *opaque, const uint8_t *buf, int size)
{
    K230UartState *s = opaque;

    s->uart_rx = *buf;
    s->uart_status |= K230_UART_STATUS_RX_NOT_EMPTY;
    /* raise IRQ line */
    k230_uart_raise_irq(s);
}


static void k230_uart_realize(DeviceState *dev, Error **errp)
{
    K230UartState *sus =  K230_UART(dev);
    qemu_chr_fe_set_handlers(&sus->chr,  k230_uart_can_receive,
                              k230_uart_receive, NULL, NULL, sus, NULL, true);
}

static void  k230_uart_instance_init(Object *obj)
{
    K230UartState *sus = K230_UART(obj);
    memory_region_init_io(&sus->mmio,
                          obj,
                          &k230_uart_ops,
                          sus,
                          TYPE_K230_UART,
                          0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &sus->mmio);
}

static const Property k230_uart_properties[] = {
    DEFINE_PROP_CHR("chardev", K230UartState, chr),
};

static void k230_uart_class_init(ObjectClass *klass, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    device_class_set_legacy_reset(dc, k230_uart_reset);
    dc->realize = k230_uart_realize;
    device_class_set_props(dc, k230_uart_properties);
    set_bit(DEVICE_CATEGORY_INPUT, dc->categories);
}

static const TypeInfo k230_uart_info = {
    .name = TYPE_K230_UART,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(K230UartState),
    .class_init = k230_uart_class_init,
    .instance_init = k230_uart_instance_init,
};

static void k230_uart_register_types(void)
{
    type_register_static(&k230_uart_info);
}
type_init(k230_uart_register_types)
