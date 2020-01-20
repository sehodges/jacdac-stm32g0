#ifndef __JDPROTOCOL_H
#define __JDPROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JD_SERIAL_PAYLOAD_SIZE 255

typedef struct {
    uint16_t crc;
    uint8_t size;
    uint8_t service_number;
    uint64_t device_identifier;
} __attribute((__packed__)) __attribute__((aligned(4))) jd_packet_header_t;

typedef struct {
    jd_packet_header_t header;
    uint8_t data[JD_SERIAL_PAYLOAD_SIZE + 1];
} jd_packet_t;

#define JD_DEVICE_FLAGS_NACK 0x08
#define JD_DEVICE_FLAGS_HAS_NAME 0x04
// #define JD_DEVICE_FLAGS_PROPOSING                       0x02
// #define JD_DEVICE_FLAGS_REJECT                          0x01

#define JD_SERVICE_FLAGS_ERROR 0x80

typedef struct {
    uint32_t service_class; // the class of the service
    uint8_t service_flags;
    uint8_t advertisement_size;   // size of the following field
    uint8_t advertisement_data[]; // optional additional data
} jd_service_information;

#ifdef __cplusplus
}
#endif

#endif
