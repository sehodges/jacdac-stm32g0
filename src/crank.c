#include "jdsimple.h"

static sensor_state_t sensor;
static int32_t sample;
static uint8_t last_p0, inited;

static void edge() {
    pin_set(PIN_SERVO,1);
    pin_set(PIN_SERVO,0);
    int curr_p0 = pin_get(PIN_P0);
    if (curr_p0 == last_p0)
        return; // hmmm...
    last_p0 = curr_p0;
    if (pin_get(PIN_P1) == curr_p0) {
        sample++;
    } else {
        sample--;
    }
}

static void maybe_init() {
    if (sensor.is_streaming && !inited) {
        inited = true;
        pin_setup_input(PIN_P0, 1);
        pin_setup_input(PIN_P1, 1);
        last_p0 = pin_get(PIN_P0);
        exti_set_callback(PIN_P0, edge, EXTI_FALLING | EXTI_RISING);
    }
}

void crank_init(uint8_t service_num) {
    sensor.service_number = service_num;
}

void crank_process() {
    static int last;
    if (sample != last)
        DMESG("s:%d", sample);
    last = sample;

    maybe_init();

    if (sensor_should_stream(&sensor))
        txq_push(sensor.service_number, JD_CMD_GET_REG | JD_REG_READING, &sample, sizeof(sample));
}

void crank_handle_packet(jd_packet_t *pkt) {
    sensor_handle_packet(&sensor, pkt);

    if (pkt->service_command == (JD_CMD_GET_REG | JD_REG_READING))
        txq_push(pkt->service_number, pkt->service_command, &sample, sizeof(sample));
}

const host_service_t host_crank = {
    .service_class = JD_SERVICE_CLASS_ROTARY_ENCODER,
    .init = crank_init,
    .process = crank_process,
    .handle_pkt = crank_handle_packet,
};
