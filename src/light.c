#include "jdsimple.h"

#define FRAME_TIME 50000
#define MAX_PIXELS 24

#define STATE_SIZE 5

struct light_state {
    actuator_state_t hd;
    uint8_t data[STATE_SIZE];
};

static struct light_state state;
static uint32_t nextFrame;

void light_init(uint8_t service_num) {
    state.hd.service_number = service_num;
    state.hd.size = STATE_SIZE;
}

void light_process() {
    if (!should_sample(&nextFrame, FRAME_TIME))
        return;
}

static void sync_state() {
    // PIN_PWR has reverse polarity
    pin_set(PIN_PWR, !actuator_enabled(&state.hd));
    if (!actuator_enabled(&state.hd))
        return;

    if (!(state.hd.status & ACTUATOR_INITED)) {
        state.hd.status |= ACTUATOR_INITED;
        px_init();
    }
}

void light_handle_packet(jd_packet_t *pkt) {
    int r = actuator_handle_packet(&state.hd, pkt);
    if (r >= 2) {
        sync_state();
    }
}

const host_service_t host_light = {
    .service_class = JD_SERVICE_CLASS_LIGHT,
    .init = light_init,
    .process = light_process,
    .handle_pkt = light_handle_packet,
};
