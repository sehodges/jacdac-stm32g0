#include "jdsimple.h"

const host_service_t *services[] = {
    &host_ctrl,
    &host_accelerometer,
    &host_light,
};

#define NUM_SERVICES (sizeof(services) / sizeof(services[0]))

uint32_t now;

void app_queue_annouce() {
    static uint8_t ledcnt;
    if (++ledcnt >= 1) {
        led_blink(50);
        ledcnt = 0;
    }

    alloc_stack_check();

    uint32_t *dst =
        txq_push(JD_SERVICE_NUMBER_CTRL, JD_CMD_ADVERTISEMENT_DATA, NULL, NUM_SERVICES * 4);
    if (!dst)
        return;
    for (int i = 0; i < NUM_SERVICES; ++i)
        dst[i] = services[i]->service_class;
}

#ifdef CNT_FLOOD
static uint32_t cnt_count;
static uint32_t prevCnt;
uint32_t numErrors, numPkts;
#endif

void app_init_services() {
    for (int i = 0; i < NUM_SERVICES; ++i) {
        services[i]->init(i);
    }
}

void app_process() {
    now = tim_get_micros();

#ifdef CNT_FLOOD
    if (txq_is_idle()) {
        cnt_count++;
        txq_push(0x42, 0x80, 0, &cnt_count, sizeof(cnt_count));
    }
#endif

    for (int i = 0; i < NUM_SERVICES; ++i) {
        services[i]->process();
    }

    txq_flush();
}

static void handle_packet(jd_packet_t *pkt) {
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
        for (int i = 0; i < NUM_SERVICES; ++i) {
            if (pkt->device_identifier == services[i]->service_class) {
                pkt->service_number = i;
                matched_devid = true;
                break;
            }
        }
    }

    if (!matched_devid)
        return;

    // dump_pkt(pkt, "TOP");

    if (pkt->service_number < NUM_SERVICES)
        services[pkt->service_number]->handle_pkt(pkt);
}

int app_handle_frame(jd_frame_t *frame) {
    now = tim_get_micros();

    if (frame->flags & JD_FRAME_FLAG_ACK_REQUESTED && frame->flags & JD_FRAME_FLAG_COMMAND &&
        frame->device_identifier == device_id())
        txq_push(JD_SERVICE_NUMBER_CRC_ACK, frame->crc, NULL, 0);

    for (;;) {
        handle_packet((jd_packet_t *)frame);
        if (!jd_shift_frame(frame))
            break;
    }

    return 0;
}
