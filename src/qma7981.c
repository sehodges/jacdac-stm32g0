#include "jdsimple.h"

/*
MakeCode accelerator position:
Laying flat: 0,0,-1000
Standing on left edge: -1000,0,0
Standing on bottom edge: 0,1000,0
*/

#define PIN_VCC PB_4
#define PIN_MISO PB_6
#define PIN_MOSI PB_5
#define PIN_SCK PB_7
#define PIN_CS PB_8
//#define PIN_CS -1

#define REG_CHIP_ID 0x00
#define REG_DX 0x01
#define REG_DY 0x03
#define REG_DZ 0x05
#define REG_STEP_CNT0 0x07
#define REG_STEP_CNT1 0x08
#define REG_INT_ST0 0x09
#define REG_INT_ST1 0x0a
#define REG_INT_ST2 0x0b
#define REG_STEP_CNT2 0x0e
#define REG_FSR 0x0f
#define REG_BW 0x10
#define REG_PM 0x11
#define REG_STEP_CONF0 0x12
#define REG_STEP_CONF1 0x13
#define REG_STEP_CONF2 0x14
#define REG_STEP_CONF3 0x15
#define REG_INT_EN0 0x16
#define REG_INT_EN1 0x17
#define REG_INT_EN2 0x18
#define REG_INT_MAP0 0x19
#define REG_INT_MAP1 0x1a
#define REG_INT_MAP2 0x1b
#define REG_INT_MAP3 0x1c
#define REG_SIG_STEP_TH 0x1d
#define REG_INTPIN_CONF 0x20
#define REG_INT_CFG 0x21
#define REG_OS_CUST_X 0x27
#define REG_OS_CUST_Y 0x28
#define REG_OS_CUST_Z 0x29
#define REG_MOT_CONF0 0x2c
#define REG_MOT_CONF1 0x2d
#define REG_MOT_CONF2 0x2e
#define REG_MOT_CONF3 0x2f
#define REG_ST 0x32
#define REG_RAISE_WAKE_PERIOD 0x35
#define REG_SR 0x36
#define REG_RAISE_WAKE_TIMEOUT_TH 0x3e

#define QMAX981_RANGE_2G 0x01
#define QMAX981_RANGE_4G 0x02
#define QMAX981_RANGE_8G 0x04
#define QMAX981_RANGE_16G 0x08
#define QMAX981_RANGE_32G 0x0f

static void send(const uint8_t *src, uint32_t len) {
    target_wait_us(5);
    for (int i = 0; i < len; ++i) {
        uint8_t mask = 0x80;
        uint8_t b = src[i];
        while (mask) {
            pin_set(PIN_MOSI, b & mask ? 1 : 0);
            pin_set(PIN_SCK, 1);
            pin_set(PIN_SCK, 0);
            mask >>= 1;
        }
    }
    pin_set(PIN_MOSI, 0);
}

static void recv(uint8_t *dst, uint32_t len) {
    target_wait_us(5);
    for (int i = 0; i < len; ++i) {
        uint8_t mask = 0x80;
        uint8_t b = 0x00;
        while (mask) {
            pin_set(PIN_SCK, 1);
            if (pin_get(PIN_MISO))
                b |= mask;
            pin_set(PIN_SCK, 0);
            mask >>= 1;
        }
        dst[i] = b;
    }
}

static void writeReg(uint8_t reg, uint8_t val) {
    target_wait_us(200);
    uint8_t cmd[] = {
        0xAE, // IDW
        0x00, // reg high
        reg,  // register
        0x00, // len high
        0x01, // len low,
        val,  // data
    };
    pin_set(PIN_CS, 0);
    send(cmd, sizeof(cmd));
    pin_set(PIN_CS, 1);
}

static void readData(uint8_t reg, uint8_t *dst, int len) {
    pin_set(PIN_CS, 0);
    uint8_t cmd[] = {
        0xAF, // IDR
        0x00, // reg high
        reg,  // register
        0x00, // len high
        len,  // len low,
    };

    send(cmd, sizeof(cmd));
    recv(dst, len);
    pin_set(PIN_CS, 1);
}

static int readReg(uint8_t reg) {
    uint8_t r = 0;
    readData(reg, &r, 1);
    return r;
}

static void init_chip() {
    writeReg(REG_PM, 0x80);
    writeReg(REG_SR, 0xB6);
    target_wait_us(500);
    writeReg(REG_SR, 0x00);
    writeReg(REG_PM, 0x80);
    writeReg(REG_FSR, QMAX981_RANGE_8G);
    writeReg(REG_BW, 0xE0); // 65Hz

#if 0
    // not sure what this is
    writeReg(0x5f, 0x80); // enable test mode,take control the FSM
    writeReg(0x5f, 0x00); // normal mode
#endif
}

static uint64_t nextRead = 0;
void acc_process() {
    if (tim_get_micros() < nextRead)
        return;
    nextRead = tim_get_micros() + 500000;

    int16_t data[3];
    readData(REG_DX, (uint8_t *)data, 6);
    int x = data[1] >> 2, y = -data[0] >> 2, z = -data[2] >> 2;
    DMESG("acc %d %d %d", x,y,z);
}

void acc_init() {
#if 1
    pin_setup_output(PIN_MOSI);
    pin_setup_output(PIN_SCK);
    pin_setup_input(PIN_MISO, -1);
#endif

    pin_setup_output(PIN_VCC);
    pin_setup_output(PIN_CS);
    pin_set(PIN_CS, 1);
    pin_set(PIN_VCC, 1);

    target_wait_us(10000);

    int v = readReg(REG_CHIP_ID);
    DMESG("acc id: %d", v);

    if (0xe0 <= v && v <= 0xe9) {
        if (v >= 0xe8) {
            // v2
        }
        init_chip();
    } else {
        DMESG("invalid chip");
    }
}
