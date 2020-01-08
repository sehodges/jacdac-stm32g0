#include "jdsimple.h"

#define JD_STATUS_RX_ACTIVE 0x01
#define JD_STATUS_TX_ACTIVE 0x02
#define TX_QUEUE_SIZE 3

static jd_packet_t rxBuffer;
static void set_tick_timer(uint8_t statusClear);
static uint64_t nextAnnounce;
static volatile uint8_t status;
static volatile uint8_t numPending;

static jd_packet_t *txQueue[TX_QUEUE_SIZE];

void jd_init() {
    tim_init();
    set_tick_timer(0);
    uart_init();
}

static void tx_done() {
    set_log_pin3(0);
    set_tick_timer(JD_STATUS_TX_ACTIVE);
}

void jd_tx_completed(int errCode) {
    // DMESG("tx done: %d", errCode);
    target_disable_irq();
    numPending--;
    target_enable_irq();
    tx_done();
}

uint32_t jd_get_num_pending_tx() {
    return numPending;
}

uint32_t jd_get_free_queue_space() {
    for (int i = TX_QUEUE_SIZE - 1; i >= 0; ++i) {
        if (txQueue[i])
            return TX_QUEUE_SIZE - i - 1;
    }
    return 0;
}

static void tick() {
    if (tim_get_micros() > nextAnnounce) {
        // pulse_log_pin();
        nextAnnounce = tim_get_micros() + random_around(400000);
        app_queue_annouce();
    }
    set_tick_timer(0);
}

static void shift_queue() {
    target_disable_irq();
    for (int i = 1; i < TX_QUEUE_SIZE; ++i)
        txQueue[i - 1] = txQueue[i];
    target_enable_irq();
}

static void flush_tx_queue() {
    target_disable_irq();
    if (status & (JD_STATUS_RX_ACTIVE | JD_STATUS_TX_ACTIVE)) {
        target_enable_irq();
        return;
    }

    status |= JD_STATUS_TX_ACTIVE;
    target_enable_irq();

    set_log_pin3(1);
    if (uart_start_tx(txQueue[0], txQueue[0]->header.size + sizeof(jd_packet_header_t)) < 0) {
        DMESG("race on TX");
        tx_done();
        jd_line_falling();
        return;
    }

    shift_queue();

    set_tick_timer(0);
}

static void set_tick_timer(uint8_t statusClear) {
    target_disable_irq();
    if (statusClear) {
        // DMESG("st %d @%d", statusClear, status);
        status &= ~statusClear;
    }
    if ((status & JD_STATUS_RX_ACTIVE) == 0) {
        if (txQueue[0] && !(status & JD_STATUS_TX_ACTIVE))
            tim_set_timer(random_around(50), flush_tx_queue);
        else
            tim_set_timer(10000, tick);
    }
    target_enable_irq();
}

static void rx_done() {
    set_tick_timer(JD_STATUS_RX_ACTIVE);
}

static void rx_timeout() {
    uart_disable();
    DMESG("RX timeout");
    rx_done();
}

void jd_line_falling() {
    pulse_log_pin();
    target_disable_irq();
    if (status & (JD_STATUS_RX_ACTIVE | JD_STATUS_TX_ACTIVE)) {
        DMESG("stale EXTI %d", status);
        target_enable_irq();
        return;
    }
    status |= JD_STATUS_RX_ACTIVE;
    wait_us(15); // otherwise we can enable RX in the middle of LO pulse
    // also, only clear the buffer after we have waited - gives some more time for processing
    memset(&rxBuffer.header, 0, sizeof(rxBuffer.header));
    uart_start_rx(&rxBuffer, sizeof(rxBuffer));
    tim_set_timer(sizeof(rxBuffer) * 12 + 60, rx_timeout);
    target_enable_irq();
}

void jd_rx_completed(int dataLeft) {
    rx_done();

    if (dataLeft < 0) {
        DMESG("rx error: %d", dataLeft);
        return;
    }

    uint32_t txSize = sizeof(rxBuffer) - dataLeft;
    uint32_t declaredSize = rxBuffer.header.size + sizeof(jd_packet_header_t);
    if (txSize < declaredSize) {
        DMESG("pkt too short");
        return;
    }
    uint16_t crc = crc16((uint8_t *)&rxBuffer + 2, declaredSize - 2);
    if (crc != rxBuffer.header.crc) {
        DMESG("crc mismatch");
        return;
    }

    if (crc == 0) {
        DMESG("crc==0");
        return;
    }

    app_handle_packet(&rxBuffer);
}

void jd_queue_packet(jd_packet_t *pkt) {
    uint32_t declaredSize = pkt->header.size + sizeof(jd_packet_header_t);
    pkt->header.crc = crc16((uint8_t *)pkt + 2, declaredSize - 2);

    if (txQueue[TX_QUEUE_SIZE - 1]) {
        // drop first packet
        shift_queue();
    } else {
        target_disable_irq();
        numPending++;
        target_enable_irq();
    }

    for (int i = 0; i < TX_QUEUE_SIZE; ++i) {
        if (!txQueue[i]) {
            txQueue[i] = pkt;
            break;
        }
    }

    set_tick_timer(0);
}
