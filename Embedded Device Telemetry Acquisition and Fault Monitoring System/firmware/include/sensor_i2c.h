#ifndef SENSOR_I2C_H
#define SENSOR_I2C_H

#include <stdbool.h>

void i2c_sensor_init(void);
bool i2c_sensor_read(float *x, float *y, float *z);

#endif /* SENSOR_I2C_H */
