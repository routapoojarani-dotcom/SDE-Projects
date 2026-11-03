#ifndef CAN_IDS_H
#define CAN_IDS_H

// one CAN id per signal 
// because each signal type gets its own identifier so receivers can filter
// for what they care about 

#define ID_ENGINE_RPM     0x100
#define ID_VEHICLE_SPEED  0x101
#define ID_ENGINE_TEMP    0x102
#define ID_FUEL_LEVEL     0x103
#define ID_BRAKE_STATUS   0x104
#define ID_TURN_SIGNAL    0x105
#define ID_ACCELERATOR    0x106  // data[0]: 0 = released, 1 = pressed 

#endif