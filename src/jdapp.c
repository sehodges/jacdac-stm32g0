#include "jdsimple.h"

#define ANN_SIZE 12

static struct {
    jd_packet_t hd;
    char data[ANN_SIZE];
} ann;

void app_queue_annouce() {
    ann.hd.size = ANN_SIZE;
    ann.hd.device_identifier = device_id();
    ann.hd.service_number = 0;
    strcpy(ann.data, "Hello");
    ann.data[8] = jd_random();
    ann.data[9] = jd_random();
    jd_queue_packet((jd_packet_t *)&ann);
}

typedef struct {
    jd_packet_t hd;
    uint32_t count;
} count_service_pkt_t;

static count_service_pkt_t cnt;

void app_process() {
    if (jd_get_num_pending_tx() == 0) {
        cnt.count++;
        cnt.hd.size = 4;
        cnt.hd.device_identifier = device_id();
        cnt.hd.service_number = 255;
        jd_queue_packet((jd_packet_t *)&cnt);
    }
}

static uint32_t prevCnt;
uint32_t numErrors, numPkts;
void app_handle_packet(jd_packet_t *pkt) {
    // DMESG("handle pkt; dst=%x/%d sz=%d", (uint32_t)pkt->header.device_identifier,
    //      pkt->header.service_number, pkt->header.size);

    numPkts++;
    if (pkt->service_number == 255) {
        count_service_pkt_t *cs = (count_service_pkt_t *)pkt;
        uint32_t c = cs->count;
        if (prevCnt && prevCnt + 1 != c) {
            log_pin_set(2, 1);
            log_pin_set(2, 0);
            numErrors++;
            DMESG("ERR %d/%d %d", numErrors, numPkts, numErrors * 10000 / numPkts);
        }
        prevCnt = c;
    }
}
