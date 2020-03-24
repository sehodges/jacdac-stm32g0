#include "jdsimple.h"

static uint32_t announceData[] = {0, 2000};

uint32_t now;

void app_queue_annouce() {
    led_toggle();
    txq_push(JD_SERVICE_NUMBER_CTRL, JD_CMD_ADVERTISEMENT_DATA, 0, announceData,
             sizeof(announceData));
}

#ifdef CNT_FLOOD
static uint32_t cnt_count;
#endif

void app_process() {
    now = tim_get_micros();

    acc_process();

#ifdef CNT_FLOOD
    if (txq_is_idle()) {
        cnt_count++;
        txq_push(0x42, 0x80, 0, &cnt_count, sizeof(cnt_count));
    }
#endif

    txq_flush();
}

static uint32_t prevCnt;
uint32_t numErrors, numPkts;

static void handle_packet(jd_packet_t *pkt) {
    // DMESG("handle pkt; dst=%x/%d sz=%d", (uint32_t)pkt->header.device_identifier,
    //      pkt->header.service_number, pkt->header.size);
    
    numPkts++;
    if (pkt->service_number == 0x42) {
        uint32_t c;
        memcpy(&c, pkt->data, sizeof(c));
        if (prevCnt && prevCnt + 1 != c) {
            log_pin_set(2, 1);
            log_pin_set(2, 0);
            numErrors++;
            DMESG("ERR %d/%d %d", numErrors, numPkts, numErrors * 10000 / numPkts);
        }
        prevCnt = c;
    } else if (pkt->service_number == ACC_SERVICE_NUM) {
        acc_handle_packet(pkt);
    }
}

int app_handle_frame(jd_frame_t *frame) {
    now = tim_get_micros();

    if (frame->flags & JD_FRAME_FLAG_ACK_REQUESTED)
        txq_push(JD_SERVICE_NUMBER_CRC_ACK, frame->crc & 0xff, frame->crc >> 8, NULL, 0);

    for (;;) {
        handle_packet((jd_packet_t *)frame);
        if (!jd_shift_frame(frame))
            break;
    }

    return 0;
}
