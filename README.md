# JACDAC for STM32F0/STM32G0

You might have heard of [JACDAC](https://jacdac.org).
It's a new protocol that aims to standardize connecting microcontrollers dynamically,
and with very little wiring (one wire for data, one for GND, and optionally one for power).
One scenario is networking of MCU-based devices, for example to enable multiplayer
for your awesome [MakeCode Arcade](https://arcade.makecode.com) games.
Another is connecting peripherals to a host device (joystick anyone?!).

What follows is a short operational specification for JACDAC v1.
The main changes with respect to [JACDAC v0](https://jacdac.org/specification/) are:

* there is only one baud-rate supported (1Mbit)
* frame starts (as it did) and ends (new) with a UART break "character";
  the break at the end typically generates an interrupt that can be used to shut down DMA transfer
* all packets include full 64 bit address of either source of destination;
  the 8 bit compressed address is gone, and so it address allocation
* CRC fixed to be CRC-16-CCITT
* announce packets now contain only service classes; device name is gone and
  advertisement data can be queried from the service
* all packets include standardized 16-bit command and 16-bit argument, plus any custom data
* a protocol-level light-weight acknowledge mechanism is introduced


Refer to [JACDAC v0 specification](https://jacdac.org/specification/) for general description of use
cases etc.
Please note that _services_ in this document refer to host services in v0 spec,
while _clients_ refer to client services.

## Requirements

MCUs implementing JACDAC typically need the following:
* a UART module
* DMA for said UART (if device does nothing but JACDAC, it may be fine to skip this)
* some source of randomness; can be ADC of a floating input or internal temperature sensor

It's important that devices don't come up with the same "random" numbers every time
they power on, and critical that two instances of the same device don't that.
Typically, you can take temperature readings a couple thousand times (they will fluctuate slightly),
hash the results and use that as a random seed.
Other option is a floating ADC.
Yet another is timing pin capacitance.

## Packets

```c
typedef struct {
    uint16_t crc;
    uint8_t size; // of data[]
    uint8_t service_number;

    uint16_t service_command;
    uint16_t service_arg;

    uint64_t device_identifier;

    uint8_t data[0];
} __attribute((__packed__)) __attribute__((aligned(4))) jd_packet_t;
```

|Offset | Size | Description
|------:|-----:| ------------------------------------
|     0 |    2 | CRC-16-CCITT of all following data
|     2 |    1 | size of the payload, `N`
|     3 |    1 | service number
|     4 |    2 | service command
|     6 |    2 | service argument
|     8 |    8 | device identifier
|    16 |  `N` | payload

All data is little endian.

Total size of the packet is thus `N + 16`.
The CRC covers all data from byte 2 on, in both header and payload,
and it is typical [CRC-16-CCITT](https://en.wikipedia.org/wiki/Cyclic_redundancy_check).

To keep the total size of packet under `255` (which is DMA limit on some hardware)
and aligned to 4, the maximum `N` is `236`.

## Physical layer

Communication is done over a single line, using UART peripheral running at 1MHz,
8 bit per byte, no parity, 1 stop bit, LSB first (standard).
The line is held high when idle with either internal or external pull up of
around 30kΩ per device.

### Reception

Normally, devices are set up to trigger an interrupt on line going low, with UART disabled.
When the line goes low, the device performs the following steps:
* zero-out header of the packet to be received
* wait for line going high again
* setup a UART in receive mode
* start reception
* setup a timer for 100us (TODO)
* when the timer fires, if the header is still all zero, signal timeout and abort reception
* otherwise, given that the header declares size to be `N`, setup timer for `(N + 16) * 12 + 60` (and don't touch reception)
* if UART break is detected, abort reception, abort the timer and process incoming packet (see below)
* if timer fires, signal timeout and abort reception
* setup line as input with pull up and reception interrupt
* possibly, start transmission timer (see below)

### Processing incoming packets

We arrive here when a UART break was detected.

* optionally, check if at least `N + 16` bytes was received;
  this may be difficult depending on DMA hardware, so can be skipped
* compute CRC of bytes `2` until `N + 16` of data;
  if it doesn't match the CRC in the first two bytes, signal error and stop processing this packet
* do control layer processing of packet (next section)

### Transmission

Devices should maintain a queue of packets to send.
When either transmission or reception finishes, and the queue is non-empty
the device should set up a timer for a random period roughly between 100us and 200us.
If this timer fires before any reception starts, the device can start
transmitting as described here.

* disable reception interrupt
* if we are already in reception abort transmission
* as quickly as possible, probe the line and if it's high, pull it low
* if the line was low, abort transmission and start reception as if the reception interrupt triggered
* wait ~10us
* pull the line high
* wait ~40us
* configure UART for transmission and start it
* when transmission ends:
* disable UART
* pull the line high
* pull it low
* wait ~10us
* pull the line high
* setup line as input with pull up and reception interrupt
* possibly, start transmission timer (see above)

Wait times need to be determined experimentally, so that the following are observed:
* initial low pulse is 11-15us
* the gap between end of low pulse and start of UART transmission is 40-50us (TODO)
* spacing between characters in UART transmission should be never more than 9us,
  and on average less than 2us
* the gap after UART transmission and before final break is less than 10us
* the final break is 11-15us

The probe-and-pull operation can typically be performed in a few cycles
and is critical to do as quick as possible.
The time difference between probe and set is the collision window of the protocol.
This can be typically brought down to around 50-100ns depending on MCU speed and GPIO hardware.
Typically, you'll want to add a function that is forced to be non-inline
and takes all arguments it needs pre-computed (GPIO port address, pin mask etc.).
It's good idea to look at disassembly of that function.

Such collision window cause about 0.3% packet loss when two devices try to
write on the bus as quickly as possible.
The packet loss grows quadratically with number of devices, but typically
devices do not flood the bus, which results in much lower packet losses.

## Control layer

Every JACDAC device has a unique 64 bit identifier.
It may be generated from hardware unique ID (through hashing if needed;
we recommend [FNV1A](https://tools.ietf.org/html/draft-eastlake-fnv-14#section-3)).
Alternatively, it may be generated at first run using randomness and stored in flash or EPROM.

Another option is to flash a randomly generated 64 bit number during production.
If that is difficult, generate 48 bit (or so) random number, and assign identifiers
starting with that number with lowest bits incrementing.
Make sure to machine-generate the number, do not just bang on the keyboard.
You can use the line at the bottom of [CF2 patcher](https://microsoft.github.io/uf2/patcher/)
to generate random numbers.

While it's theoretically possible for a device ID collision to occur in a small network of say 100 devices,
it's very unlikely unless we get many trillions of such networks.
This, however, relies on IDs being evenly distributed (ie., random).

### Direction of packets

The JACDAC packets contain only one device identifier.
* if highest bit of service command is set, we call it a _command packet_ and the device identifier is the destination
* otherwise, it's a _report packet_ and the device identifier is the source

Additionally, for command packets with service number of `0x41` (_multicast commands_),
the low order 32 bits of device identifier contain service class.
The command is then directed to all services with that service class.

### Services

Services in JACDAC are similar to services like telnet, HTTP, or SMTP in TCP/IP.
Each device can expose zero or more of these.
A service instance is uniquely identified by device identifier and 6 bit service number.
This also maps to a 32 bit service class.

Devices periodically advertise services they provide.
This advertisement lists all service classes in order of their respective service numbers.

For example, a device A may advertise the that it exposes the following services:
* control (0)
* accelerometer (1)
* magnetometer (2)
One can then send command packets with device identifier of A and service number of 2 to reach
the magnetometer service on A, for example to enable streaming of readings.
The readings would then similarly arrive with device identifier of A and service
number of 2, but as reports.

When device advertises, the control service has to always reside at service number zero.

### Advertisement report packets

Advertisement are sent every 300-500ms (the actual time should be picked at random between every two
advertisement packets).

The service command is 0, and argument is reserved for future use (send as 0).
The payload is an array of unsigned 32 bit integers that represent service classes.
The position in the array is the service number.
For services that are missing or disabled use `0xffffffff`.

TODO: do we always want to leading zero for control service?

### ACKs

If the packet is received by the control layer, and is then routed correctly,
an ACK may need to be sent.
If in a command packet, device identifier equals our device identifier,
and service number field has the highest bit set, an ACK is to be sent.

ACK packet uses our device identifier, service number of 0,
service command of 1, and uses the CRC of the packet being acknowledged
as the service argument.
