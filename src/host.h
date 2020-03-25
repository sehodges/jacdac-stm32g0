#pragma once

#define PKT_UNHANDLED 0
#define PKT_HANDLED_RO 1
#define PKT_HANDLED_RW 2

// keep sampling at period, using state at *sample
bool should_sample(uint32_t *sample, uint32_t period);

// sensor helpers
struct _sensor_state {
    uint16_t status;
    uint8_t service_number;
    uint32_t sample_interval;
    uint32_t next_sample;
};
typedef struct _sensor_state sensor_state_t;

#define SENSOR_STREAMING 0x01
#define SENSOR_INITED 0x02

int sensor_handle_packet(sensor_state_t *state, jd_packet_t *pkt);
int sensor_should_stream(sensor_state_t *state);

#define ACTUATOR_ENABLED 0x01
#define ACTUATOR_INITED 0x02
struct _actuator_state {
    uint8_t status;
    uint8_t size;
    uint8_t service_number;
    uint8_t intensity;
    uint8_t data[0];
};
typedef struct _actuator_state actuator_state_t;

int handle_get_set(uint8_t get_cmd, void *state, int size, jd_packet_t *pkt);

int actuator_handle_packet(actuator_state_t *state, jd_packet_t *pkt);
static inline bool actuator_enabled(actuator_state_t *state) {
    return (state->status & ACTUATOR_ENABLED) != 0;
}

typedef void (*pkt_handler_t)(jd_packet_t *pkt);
typedef void (*service_fn_t)(uint8_t service_num);

struct _host_service {
    uint32_t service_class;
    service_fn_t init;
    cb_t process;
    pkt_handler_t handle_pkt;
};
typedef struct _host_service host_service_t;

extern const host_service_t host_ctrl;
extern const host_service_t host_accelerometer;
extern const host_service_t host_light;
