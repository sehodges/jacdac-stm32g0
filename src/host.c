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
        return PKT_HANDLED_RW;
    case JD_CMD_GET_STREAMING:
        val = state->sample_interval / 1000;
        txq_push(pkt->service_number, JD_CMD_GET_STREAMING,
                 state->status & SENSOR_STREAMING ? 1 : 0, &val, 4);
        return PKT_HANDLED_RO;
    default:
        return PKT_UNHANDLED;
    }
}

int sensor_should_stream(sensor_state_t *state) {
    if (!(state->status & SENSOR_STREAMING))
        return false;
    return should_sample(&state->next_sample, state->sample_interval);
}

int actuator_handle_packet(actuator_state_t *state, jd_packet_t *pkt) {
    int r = handle_get_set(JD_CMD_GET_STATE, state->data, state->size, pkt);
    if (r)
        return r;

    switch (pkt->service_command) {
    case JD_CMD_SET_ENABLED:
        if (pkt->service_arg == 0)
            state->status &= ~ACTUATOR_ENABLED;
        else if (pkt->service_arg == 1)
            state->status |= ACTUATOR_ENABLED;
        return PKT_HANDLED_RW;
    case JD_CMD_GET_ENABLED:
        txq_push(pkt->service_number, JD_CMD_GET_ENABLED, actuator_enabled(state), NULL, 0);
        return PKT_HANDLED_RO;
    case JD_CMD_SET_INTENSITY:
        state->intensity = pkt->service_arg;
        return PKT_HANDLED_RW;
    default:
        return PKT_UNHANDLED;
    }
}

int handle_get_set(uint8_t get_cmd, void *state, int size, jd_packet_t *pkt) {
    if (pkt->service_command == get_cmd) {
        txq_push(pkt->service_number, get_cmd, 0, state, size);
        return PKT_HANDLED_RO;
    } else if (pkt->service_command == get_cmd + 1) {
        int sz = pkt->service_size;
        if (sz > size)
            sz = size;
        memcpy(state, pkt->data, sz);
        return PKT_HANDLED_RW;
    }
    return PKT_UNHANDLED;
}
