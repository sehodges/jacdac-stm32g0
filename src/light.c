#include "jdsimple.h"

#define FRAME_TIME 50000

struct light_state {
    actuator_state_t hd;
    uint8_t cmd;
    uint8_t padding;
    uint16_t duration;
    uint32_t color;
};

struct light_config {
    uint16_t numpixels;
};

static struct light_config config;
static struct light_state state;
static uint32_t *pxbuffer;
static uint16_t pxbuffer_allocated;
static uint32_t nextFrame;

void light_init(uint8_t service_num) {
    state.hd.service_number = service_num;
    state.hd.size = sizeof(state) - sizeof(state.hd);
    state.hd.intensity = 20;
}

static inline uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16) | (g << 8) | b;
}

static uint32_t hsv(uint8_t hue, uint8_t sat, uint8_t val) {
    // scale down to 0..192
    hue = (hue * 192) >> 8;

    // reference: based on FastLED's hsv2rgb rainbow algorithm
    // [https://github.com/FastLED/FastLED](MIT)
    int invsat = 255 - sat;
    int brightness_floor = (val * invsat) >> 8;
    int color_amplitude = val - brightness_floor;
    int section = (hue / 0x40) >> 0; // [0..2]
    int offset = (hue % 0x40) >> 0;  // [0..63]

    int rampup = offset;
    int rampdown = (0x40 - 1) - offset;

    int rampup_amp_adj = ((rampup * color_amplitude) / (256 / 4));
    int rampdown_amp_adj = ((rampdown * color_amplitude) / (256 / 4));

    int rampup_adj_with_floor = (rampup_amp_adj + brightness_floor);
    int rampdown_adj_with_floor = (rampdown_amp_adj + brightness_floor);

    int r, g, b;
    if (section) {
        if (section == 1) {
            // section 1: 0x40..0x7F
            r = brightness_floor;
            g = rampdown_adj_with_floor;
            b = rampup_adj_with_floor;
        } else {
            // section 2; 0x80..0xBF
            r = rampup_adj_with_floor;
            g = brightness_floor;
            b = rampdown_adj_with_floor;
        }
    } else {
        // section 0: 0x00..0x3F
        r = rampdown_adj_with_floor;
        g = rampup_adj_with_floor;
        b = brightness_floor;
    }
    return rgb(r, g, b);
}

static const uint8_t b_m16[] = {0, 49, 49, 41, 90, 27, 117, 10};
static int isin(uint8_t theta) {
    // reference: based on FASTLed's sin approximation method:
    // [https://github.com/FastLED/FastLED](MIT)
    int offset = theta;
    if (theta & 0x40) {
        offset = 255 - offset;
    }
    offset &= 0x3F; // 0..63

    int secoffset = offset & 0x0F; // 0..15
    if (theta & 0x40)
        secoffset++;

    int section = offset >> 4; // 0..3
    int s2 = section * 2;

    int b = b_m16[s2];
    int m16 = b_m16[s2 + 1];
    int mx = (m16 * secoffset) >> 4;

    int y = mx + b;
    if (theta & 0x80)
        y = -y;

    y += 128;

    return y;
}

static bool is_enabled() {
    return actuator_enabled(&state.hd) && config.numpixels > 0 && state.hd.intensity > 0;
}

static void do_nothing() {}

static void set(uint32_t index, uint32_t color) {
    px_set(pxbuffer, index, state.hd.intensity, color);
}

static void show() {
    px_tx(pxbuffer, PX_WORDS(config.numpixels) << 2, do_nothing);
}

static void set_all(uint32_t color) {
    for (int i = 0; i < config.numpixels; ++i)
        set(i, color);
}

static void anim_set_all() {
    set_all(state.color);
    show();
}

static uint32_t anim_step, anim_value;
static uint8_t anim_flag;

static cb_t anim_fn;
static uint32_t anim_end;
static void anim_start(cb_t fn, uint32_t duration) {
    anim_fn = fn;
    if (duration == 0)
        anim_end = 0;
    else
        anim_end = now + duration * 1000;
}
static void anim_finished() {
    if (anim_end == 0)
        anim_fn = NULL;
}

static void anim_frame() {
    if (anim_fn) {
        anim_fn();
        show();
        if (anim_end && in_past(anim_end)) {
            anim_fn = NULL;
        }
    }
}

// ---------------------------------------------------------------------------------------
// rainbow
// ---------------------------------------------------------------------------------------

static void rainbow_step() {
    for (int i = 0; i < config.numpixels; ++i)
        set(i, hsv(((i * 256) / (config.numpixels - 1) + anim_value) & 0xff, 0xff, 0xff));
    anim_value += anim_step;
    if (anim_value >= 0xff) {
        anim_value = 0;
        anim_finished();
    }
}
static void anim_rainbow() {
    anim_value = 0;
    anim_step = 128 / config.numpixels + 1;
    anim_start(rainbow_step, state.duration);
}

// ---------------------------------------------------------------------------------------
// running lights
// ---------------------------------------------------------------------------------------

static void running_lights_step() {
    if (anim_value >= config.numpixels * 2) {
        anim_value = 0;
        anim_finished();
        return;
    }

    anim_value++;
    for (int i = 0; i < config.numpixels; ++i) {
        int level = (state.hd.intensity * ((isin(i + anim_value) * 127) + 128)) >> 8;
        px_set(pxbuffer, i, level, state.color);
    }
}

static void anim_running_lights() {
    anim_value = 0;
    if (!state.color)
        state.color = 0xff0000;
    anim_start(running_lights_step, state.duration);
}

// ---------------------------------------------------------------------------------------
// sparkle
// ---------------------------------------------------------------------------------------

static void sparkle_step() {
    if (anim_value == 0)
        set_all(0);
    anim_value++;

    if (anim_step < 0) {
        anim_step = random_int(config.numpixels - 1);
        set(anim_step, state.color);
    } else {
        set(anim_step, 0);
        anim_step = -1;
    }

    if (anim_value > 50) {
        anim_finished();
        anim_value = 0;
    }
}

static void anim_sparkle() {
    anim_value = 0;
    anim_step = -1;
    if (!state.color)
        state.color = 0xffffff;
    anim_start(sparkle_step, state.duration);
}

// ---------------------------------------------------------------------------------------
// color wipe
// ---------------------------------------------------------------------------------------

static void color_wipe_step() {
    if (anim_value < config.numpixels) {
        set(anim_value, anim_flag ? state.color : 0);
        anim_value++;
    } else {
        anim_flag = !anim_flag;
        anim_value = 0;
        if (anim_flag)
            anim_finished();
    }
}

static void anim_color_wipe() {
    anim_value = 0;
    anim_flag = 1;
    if (!state.color)
        state.color = 0x0000ff;
    anim_start(color_wipe_step, state.duration);
}

// ---------------------------------------------------------------------------------------
// theatre chase
// ---------------------------------------------------------------------------------------
static void theatre_chase_step() {
    if (anim_value < 10) {
        if (anim_step < 3) {
            for (int i = 0; i < config.numpixels; i += 3)
                set(i + anim_step, anim_flag ? state.color : 0);
            anim_flag = !anim_flag;
            anim_step++;
        } else {
            anim_step = 0;
        }
        anim_value++;
    } else {
        anim_value = 0;
        anim_finished();
    }
}

static void anim_theatre_chase() {
    anim_value = 0;
    anim_step = 0;
    anim_flag = 0;
    if (!state.color)
        state.color = 0xff0000;
    anim_start(theatre_chase_step, state.duration);
}

// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------

static cb_t animations[] = {
    NULL,
    anim_set_all,
    anim_rainbow,
    anim_running_lights,
    anim_color_wipe,
    NULL, // comet
    anim_theatre_chase,
    anim_sparkle,
};

void light_process() {
    if (!is_enabled())
        return;

    if (should_sample(&nextFrame, FRAME_TIME)) {
        anim_frame();
    }
}

static void sync_state() {
    if (!is_enabled()) {
        // PIN_PWR has reverse polarity
        pin_set(PIN_PWR, 1);
        return;
    }

    pin_set(PIN_PWR, 0);

    if (!(state.hd.status & ACTUATOR_INITED)) {
        state.hd.status |= ACTUATOR_INITED;
        px_init();
    }

    if (state.cmd < sizeof(animations) / sizeof(animations[0])) {
        cb_t f = animations[state.cmd];
        if (f)
            f();
    }
}

static void sync_config() {
    if (config.numpixels > pxbuffer_allocated) {
        pxbuffer_allocated = config.numpixels;
        pxbuffer = alloc(PX_WORDS(pxbuffer_allocated) * 4);
    }
    sync_state();
}

void light_handle_packet(jd_packet_t *pkt) {
    int r;
    r = handle_get_set(JD_CMD_GET_CONFIG, &config, sizeof(config), pkt);
    if (r == PKT_HANDLED_RW)
        sync_config();

    r = actuator_handle_packet(&state.hd, pkt);
    if (r == PKT_HANDLED_RW)
        sync_state();
}

const host_service_t host_light = {
    .service_class = JD_SERVICE_CLASS_LIGHT,
    .init = light_init,
    .process = light_process,
    .handle_pkt = light_handle_packet,
};
