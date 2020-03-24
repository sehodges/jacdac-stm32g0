#include "jdsimple.h"

static uint32_t myServices[] = {
    JD_SERVICE_CLASS_CTRL,          // 0
    JD_SERVICE_CLASS_ACCELEROMETER, // 1
};

uint32_t now;

static void identify(int num) {
    static uint8_t id_counter;
    static uint32_t nextblink;
    if (num)
        id_counter = num;
    if (!id_counter)
        return;
    if (!should_sample(&nextblink, 150000))
        return;

    id_counter--;
    led_blink(50);
}

void app_queue_annouce() {
    static uint8_t ledcnt;
    if (++ledcnt >= 3) {
        led_blink(1);
        ledcnt = 0;
    }

    txq_push(JD_SERVICE_NUMBER_CTRL, JD_CMD_ADVERTISEMENT_DATA, 0, myServices, sizeof(myServices));
}

#ifdef CNT_FLOOD
static uint32_t cnt_count;
static uint32_t prevCnt;
uint32_t numErrors, numPkts;
#endif

void app_process() {
    now = tim_get_micros();

    acc_process();

    identify(0);

#ifdef CNT_FLOOD
    if (txq_is_idle()) {
        cnt_count++;
        txq_push(0x42, 0x80, 0, &cnt_count, sizeof(cnt_count));
    }
#endif

    txq_flush();
}

static void handle_ctrl_packet(jd_packet_t *pkt) {
    switch (pkt->service_command) {
    case JD_CMD_ADVERTISEMENT_DATA:
        app_queue_annouce();
        break;
    case JD_CMD_CTRL_IDENTIFY:
        identify(7);
        break;
    }
}

static void handle_packet(jd_packet_t *pkt) {
// DMESG("handle pkt; dst=%x/%d sz=%d", (uint32_t)pkt->header.device_identifier,
//      pkt->header.service_number, pkt->header.size);

#ifdef CNT_FLOOD
    if (pkt->service_number == 0x42) {
        numPkts++;
        uint32_t c;
        memcpy(&c, pkt->data, sizeof(c));
        if (prevCnt && prevCnt + 1 != c) {
            log_pin_set(2, 1);
            log_pin_set(2, 0);
            numErrors++;
            DMESG("ERR %d/%d %d", numErrors, numPkts, numErrors * 10000 / numPkts);
        }
        prevCnt = c;
    }
#endif

    if (!(pkt->flags & JD_FRAME_FLAG_COMMAND))
        return;

    bool matched_devid = pkt->device_identifier == device_id();

    if (pkt->flags & JD_FRAME_FLAG_IDENTIFIER_IS_SERVICE_CLASS) {
        for (int i = 0; i < sizeof(myServices) / sizeof(myServices[0]); ++i) {
            if (pkt->device_identifier == myServices[i]) {
                pkt->service_number = i;
                matched_devid = true;
                break;
            }
        }
    }

    if (!matched_devid)
        return;

    switch (pkt->service_number) {
    case JD_SERVICE_NUMBER_CTRL:
        handle_ctrl_packet(pkt);
        break;
    case ACC_SERVICE_NUM:
        acc_handle_packet(pkt);
        break;
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
