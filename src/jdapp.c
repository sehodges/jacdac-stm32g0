#include "jdsimple.h"

#define ANN_SIZE 12

struct {
    jd_packet_header_t hd;
    char data[ANN_SIZE];
} ann;

void app_queue_annouce() {
    pulse_log_pin();
    ann.hd.size = ANN_SIZE;
    ann.hd.device_identifier = device_id();
    ann.hd.service_number = 0;
    strcpy(ann.data, "Hello");
    ann.data[8] = random();
    ann.data[9] = random();
    jd_queue_packet((jd_packet_t *)&ann);
}

void app_handle_packet(jd_packet_t *pkt) {
    DMESG("handle pkt; dst=%x/%d sz=%d", (uint32_t)pkt->header.device_identifier,
          pkt->header.service_number, pkt->header.size);
}
