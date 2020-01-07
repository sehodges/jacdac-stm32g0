#include "jdsimple.h"

static uint32_t rxBuffer[256 / 4];

void handle_raw_pkt(const void *data, uint32_t size) {
    uint8_t *dd = (uint8_t *)data;
    DMESG("handle %d bytes; %d", size, dd[12]);
}

void tx_completed(int errCode) {
    DMESG("tx done: %d", errCode);
}

static void rx_timeout() {
    uart_disable();
    DMESG("RX timeout");
}

void uart_line_falling() {
    pulse_log_pin();
    rxBuffer[0] = 0;
    rxBuffer[1] = 0;
    wait_us(15); // otherwise we can enable RX in the middle of LO pulse
    uart_start_rx(rxBuffer, sizeof(rxBuffer));
    pulse_log_pin();
    set_timer(sizeof(rxBuffer) * 11 + 60, rx_timeout);
}

void rx_completed(int dataLeft) {
    set_timer(0, NULL);
    handle_raw_pkt(rxBuffer, sizeof(rxBuffer) - dataLeft);
}