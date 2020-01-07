#include "jdsimple.h"

static uint32_t rxBuffer[256 / 4];
static void set_tick_timer();
static uint64_t nextAnnounce;

void handle_raw_pkt(const void *data, uint32_t size) {
    uint8_t *dd = (uint8_t *)data;
    DMESG("handle %d bytes; %d", size, dd[12]);
}

void jd_init() {
    tim_init();
    uart_init();
    set_tick_timer();
}

void jd_tx_completed(int errCode) {
    DMESG("tx done: %d", errCode);
}

static void tick() {
    if (tim_get_micros() > nextAnnounce) {
        pulse_log_pin();
        nextAnnounce = tim_get_micros() + random_around(400000);
    }
    set_tick_timer();
}

static void set_tick_timer() {
    tim_set_timer(10000, tick);
}

static void rx_timeout() {
    uart_disable();
    set_tick_timer();
    DMESG("RX timeout");
}

void jd_line_falling() {
    // pulse_log_pin();
    rxBuffer[0] = 0;
    rxBuffer[1] = 0;
    wait_us(15); // otherwise we can enable RX in the middle of LO pulse
    uart_start_rx(rxBuffer, sizeof(rxBuffer));
    tim_set_timer(sizeof(rxBuffer) * 11 + 60, rx_timeout);
}

void jd_rx_completed(int dataLeft) {
    set_tick_timer();
    handle_raw_pkt(rxBuffer, sizeof(rxBuffer) - dataLeft);
}