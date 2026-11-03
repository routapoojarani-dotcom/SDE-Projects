#include <stdio.h>
#include <assert.h>

#include "can.h"

void test_uint16_roundtrip(void) {
    unsigned char data[2];
    can_encode_uint16(3200, data);
    int decoded = can_decode_uint16(data);
    assert(decoded == 3200);
    printf("PASS: test_uint16_roundtrip\n");
}

void test_uint16_zero(void) {
    unsigned char data[2];
    can_encode_uint16(0, data);
    assert(can_decode_uint16(data) == 0);
    printf("PASS: test_uint16_zero\n");
}

void test_uint16_max_realistic(void) {
    unsigned char data[2];
    can_encode_uint16(6000, data);  // max RPM used by Engine ECU
    assert(can_decode_uint16(data) == 6000);
    printf("PASS: test_uint16_max_realistic\n");
}

void test_uint16_byte_order(void) {
    // Confirms big-endian layout specifically: high byte first
    unsigned char data[2];
    can_encode_uint16(0x1234, data);
    assert(data[0] == 0x12);
    assert(data[1] == 0x34);
    printf("PASS: test_uint16_byte_order\n");
}

void test_uint8_roundtrip(void) {
    unsigned char data[1];
    can_encode_uint8(87, data);
    assert(can_decode_uint8(data) == 87);
    printf("PASS: test_uint8_roundtrip\n");
}

void test_uint8_boundary_max(void) {
    unsigned char data[1];
    can_encode_uint8(255, data);
    assert(can_decode_uint8(data) == 255);
    printf("PASS: test_uint8_boundary_max\n");
}

void test_uint8_overflow_wraps(void) {
    // 256 doesn't fit in a byte — confirms our encode masks correctly rather than corrupting memory
    unsigned char data[1];
    can_encode_uint8(256, data);
    assert(can_decode_uint8(data) == 0);  // 256 & 0xFF == 0
    printf("PASS: test_uint8_overflow_wraps\n");
}

void test_validate_dlc_valid_values(void) {
    assert(can_validate_dlc(0) == 1);
    assert(can_validate_dlc(4) == 1);
    assert(can_validate_dlc(8) == 1);
    printf("PASS: test_validate_dlc_valid_values\n");
}

void test_validate_dlc_invalid_value(void) {
    assert(can_validate_dlc(9) == 0);
    assert(can_validate_dlc(255) == 0);
    printf("PASS: test_validate_dlc_invalid_value\n");
}

int main(void) {
    test_uint16_roundtrip();
    test_uint16_zero();
    test_uint16_max_realistic();
    test_uint16_byte_order();
    test_uint8_roundtrip();
    test_uint8_boundary_max();
    test_uint8_overflow_wraps();
    test_validate_dlc_valid_values();
    test_validate_dlc_invalid_value();

    printf("\nAll 9 unit tests passed.\n");
    return 0;
}