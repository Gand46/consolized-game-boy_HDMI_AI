#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "hardware/gpio.h"

#include "dvi.h"
#include "dvi_serialiser.h"
#include "common_dvi_pin_configs.h"

#define VSYNC_PIN                   18
#define HSYNC_PIN                   17
#define PIXEL_CLOCK_PIN             16
#define DATA_1_PIN                  15
#define DATA_0_PIN                  14

#define DMG_PIXELS_X                160
#define DMG_PIXELS_Y                144
#define DMG_PIXEL_COUNT             (DMG_PIXELS_X * DMG_PIXELS_Y)

#define DVI_LOGICAL_WIDTH           320
#define DVI_LOGICAL_HEIGHT          240
#define DVI_TIMING                  dvi_timing_640x480p_60hz
#define VREG_VSEL                   VREG_VOLTAGE_1_20

#define OUT_ACTIVE_W                266
#define OUT_ACTIVE_H                240
#define OUT_OFFSET_X                ((DVI_LOGICAL_WIDTH - OUT_ACTIVE_W) / 2)

static struct dvi_inst dvi0;

static uint8_t framebuffer[2][DMG_PIXEL_COUNT];
static volatile uint8_t fb_write_index = 0;
static volatile uint8_t fb_read_index = 1;

static uint16_t dvi_scanline[2][DVI_LOGICAL_WIDTH];

// Esquema DMG-like (claro->oscuro)
static const uint16_t palette565[4] = {
    0xFFFF, // white
    0xAD55, // light gray
    0x52AA, // dark gray
    0x0000  // black
};

static inline void read_pixel(uint8_t **dst) {
    uint8_t v = (uint8_t)((gpio_get(DATA_0_PIN) << 1) | gpio_get(DATA_1_PIN));
    **dst = v;
    (*dst)++;
}

static void __not_in_flash_func(gpio_callback_video)(uint gpio, uint32_t events) {
    (void)gpio;
    (void)events;

    uint8_t *dst = framebuffer[fb_write_index];

    for (uint16_t y = 0; y < DMG_PIXELS_Y; ++y) {
        while (gpio_get(HSYNC_PIN) == 0) {}
        while (gpio_get(HSYNC_PIN) == 1) {}

        read_pixel(&dst);

        uint16_t x = 1;
        uint8_t clk = gpio_get(PIXEL_CLOCK_PIN);
        uint8_t clk_last = clk;
        while (x < DMG_PIXELS_X) {
            clk = gpio_get(PIXEL_CLOCK_PIN);
            if (clk == 0 && clk_last == 1) {
                read_pixel(&dst);
                ++x;
            }
            clk_last = clk;
        }
    }

    const uint8_t old_write = fb_write_index;
    fb_write_index = fb_read_index;
    fb_read_index = old_write;
}

static void init_gb_input_pins(void) {
    const uint pins[] = {VSYNC_PIN, HSYNC_PIN, PIXEL_CLOCK_PIN, DATA_0_PIN, DATA_1_PIN};
    for (size_t i = 0; i < sizeof(pins) / sizeof(pins[0]); ++i) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_IN);
    }

    gpio_set_irq_enabled_with_callback(VSYNC_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback_video);
}

static inline uint8_t sample_gb_pixel(const uint8_t *fb, uint16_t x, uint16_t y) {
    if (x >= DMG_PIXELS_X || y >= DMG_PIXELS_Y) {
        return 0;
    }
    return fb[y * DMG_PIXELS_X + x] & 0x03;
}

static void build_scanline(uint16_t out_y, uint16_t *dst) {
    const uint8_t read_idx = fb_read_index;
    const uint8_t *fb = framebuffer[read_idx];

    for (uint16_t x = 0; x < DVI_LOGICAL_WIDTH; ++x) {
        dst[x] = 0x0000;
    }

    if (out_y >= OUT_ACTIVE_H) {
        return;
    }

    // Mapeo con aspecto preservado: 266x240 -> 160x144
    const uint16_t src_y = (uint16_t)((out_y * DMG_PIXELS_Y) / OUT_ACTIVE_H);
    for (uint16_t x = 0; x < OUT_ACTIVE_W; ++x) {
        const uint16_t src_x = (uint16_t)((x * DMG_PIXELS_X) / OUT_ACTIVE_W);
        const uint8_t p = sample_gb_pixel(fb, src_x, src_y);
        dst[OUT_OFFSET_X + x] = palette565[p];
    }
}

static void core1_main(void) {
    dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
    while (queue_is_empty(&dvi0.q_colour_valid)) {
        __wfe();
    }
    dvi_start(&dvi0);
    dvi_scanbuf_main_16bpp(&dvi0);
}

int main(void) {
    // PicoDVI requiere overclock para 640x480p60
    vreg_set_voltage(VREG_VSEL);
    sleep_ms(10);
    set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);


    dvi0.timing = &DVI_TIMING;
    dvi0.ser_cfg = DVI_DEFAULT_SERIAL_CONFIG;
    dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

    multicore_launch_core1(core1_main);

    init_gb_input_pins();

    uint8_t line_buf_index = 0;

    while (true) {
        for (uint16_t y = 0; y < DVI_LOGICAL_HEIGHT; ++y) {
            uint16_t *scanline = dvi_scanline[line_buf_index];
            build_scanline(y, scanline);
            queue_add_blocking_u32(&dvi0.q_colour_valid, &scanline);

            const uint16_t *unused;
            while (queue_try_remove_u32(&dvi0.q_colour_free, &unused)) {
            }

            line_buf_index ^= 1u;
        }
    }
}
