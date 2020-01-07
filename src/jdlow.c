#include "jdsimple.h"

void handle_raw_pkt(const void *data, uint32_t size) {
    uint8_t *dd = (uint8_t*)data;
    DMESG("handle %d bytes; %d", size, dd[12]);
}

void tx_completed(int errCode) {

}