#include "jdsimple.h"

bool should_sample(uint32_t *sample, uint32_t period) {
    if (in_future(*sample))
        return false;

    *sample += period;

    if (in_past(*sample))
        // we lost some samples
        *sample = now + period;

    return true;
}

int sensor_handle_packet(sensor_state_t *state, jd_packet_t *pkt) {
    int val = pkt->service_size >= 4 ? *(int *)pkt->data : 0;
    switch (pkt->service_command) {
    case JD_CMD_SET_STREAMING:
        if (pkt->service_arg == 1) {
            state->status |= SENSOR_STREAMING;
            if (val) {
                if (val < 20)
                    val = 20; // min 20ms
                if (val > 100000)
                    val = 100000; // max 100s
                state->sample_interval = val * 1000;
            }
            if (!state->sample_interval)
                state->sample_interval = 100000; // default to 100ms
            state->next_sample = now;
        } else if (pkt->service_arg == 0) {
            state->status &= ~SENSOR_STREAMING;
        }
        break;
    case JD_CMD_GET_STREAMING:
        val = state->sample_interval / 1000;
        txq_push(pkt->service_number, JD_CMD_GET_STREAMING,
                 state->status & SENSOR_STREAMING ? 1 : 0, &val, 4);
        break;
    default:
        return 0;
    }

    return 1;
}

int sensor_should_stream(sensor_state_t *state) {
    if (!(state->status & SENSOR_STREAMING))
        return false;
    return should_sample(&state->next_sample, state->sample_interval);
}

