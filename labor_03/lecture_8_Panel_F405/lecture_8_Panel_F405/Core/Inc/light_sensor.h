#ifndef INC_LIGHT_SENSOR_H_
#define INC_LIGHT_SENSOR_H_

// These are the registers of the light sensor
// Based on the datasheet adding 0x80 to the register address is needed
// Example:
//    DATA0LOW register address: 0x0A
//    The actual address to write: 0x0A | 0x80 --> 0x8A
enum I2C_LIGHT_SENSOR_REGISTERS
{
	CONTROL = 0x80,
	TIMING,
	THRESHLOWLOW,
	THRESHLOWHIGH,
	THRESHHIGHLOW,
	THRESHHIGHHIGH,
	INTERRUPT,
	ID = 0x8A,
	DATA0LOW = 0x8C,
	DATA0HIGH,
	DATA1LOW,
	DATA1HIGH
};

// The possible I2C adresses based on the ADDRSEL pin
// 1 bit left shifting is needed for the right communication
// Example
//    Based on the datasheet if the ADDRSEL pin is grounded, then the address is: 0b00101001
//    The actual adress will be: 0b00101001 << 1, so 0b01010010
enum I2C_LIGHT_SENSOR_ADDRESS
{
	ADDR_SEL_ON_GROUND = 0x52,
	ADDR_SEL_FLOATS = 0x72,
	ADDR_SEL_ON_VDD = 0x92,
};

// There is 2 usage mode for the sensor
// SIMPLE_MODE will show you how to formulate the sequence learned in this chapter as code
// MEMORY_MODE can be used to cover the sequence, write shorter code, and improve the readability of the code
enum I2C_USAGE_MODE
{
	SIMPLE_MODE,
	MEMORY_MODE
};

// These are the functions to switch on the light sensor and read the ADC0 channel
// The functions get as a parameter how to perform the operation from the upper mentioned two usage modes
void Light_Sensor_Power_On(enum I2C_USAGE_MODE mode);
uint16_t Light_Sensor_Read_Value(enum I2C_USAGE_MODE mode);

#endif /* INC_LIGHT_SENSOR_H_ */
