#include "i2c.h"
#include "light_sensor.h"

// The light sensor address is defined by this value
const uint8_t LIGHT_SENSOR_ADDRESS = ADDR_SEL_FLOATS;

// Based on the datasheet we have to write 0x03 to the CONTROL register to switch on the sensor
const uint8_t POWER_ON = 0x03;

// Of the modes described in the light_sensor.h file, this function will show the write sequence
HAL_StatusTypeDef Light_Sensor_Power_On_Simple()
{
	uint8_t bytes_to_send[2] = { CONTROL, POWER_ON };	// datas to send
	HAL_StatusTypeDef write_status = HAL_I2C_Master_Transmit(&hi2c2, LIGHT_SENSOR_ADDRESS, &bytes_to_send[0], 2, 100); 	// sending
	//a return corresponding to the success of the data transmission
	return write_status;
}

// Of the modes described in the light_sensor.h file, this function will hide the write sequence
HAL_StatusTypeDef Light_Sensor_Power_On_Memory()
{
	uint8_t control_reg_content = POWER_ON;			// data to the register
	// register writing
	HAL_StatusTypeDef write_status = HAL_I2C_Mem_Write(&hi2c2,
			LIGHT_SENSOR_ADDRESS, CONTROL, I2C_MEMADD_SIZE_8BIT,
			&control_reg_content, 1, 100);
	//a return corresponding to the success of the data transmission
	return write_status;
}

// Implementation of the function declared in the light_sensor.h file
// It will write the POWER_ON value to the CONTROL register based on the received mode parameter
void Light_Sensor_Power_On(enum I2C_USAGE_MODE mode)
{
	HAL_StatusTypeDef write_status = HAL_ERROR;

	switch (mode)
	{
	case SIMPLE_MODE:
		write_status = Light_Sensor_Power_On_Simple();
		break;
	case MEMORY_MODE:
		write_status = Light_Sensor_Power_On_Memory();
		break;
	}
	// Error handling if the value writing fails
	if (write_status != HAL_OK)
	{
		Error_Handler();
	}
}

// Of the modes described in the light_sensor.h file, this function will represent the multi-byte read sequence
HAL_StatusTypeDef Light_Sensor_Read_Value_Simple(uint16_t *output_value)
{
	uint8_t byte_to_send = { DATA0LOW };		// data to send, in that case the register to be read
	HAL_StatusTypeDef write_status = HAL_ERROR;
	write_status = HAL_I2C_Master_Transmit(&hi2c2, LIGHT_SENSOR_ADDRESS, &byte_to_send, 1, 100);	//send

	//a return corresponding to the success of the data transmission
	if(write_status != HAL_OK) {
		return write_status;
	}

	uint8_t register_content[2] = { 0 };		// create an array of the data to read
	HAL_StatusTypeDef read_status = HAL_I2C_Master_Receive(&hi2c2, LIGHT_SENSOR_ADDRESS, &register_content[0], 2, 100);		//data receive function

	*output_value = register_content[1] << 8 | register_content[0];		//adding array values to a variable

	return read_status;
}

// Of the modes described in the light_sensor.h file, this function will hide the multi-byte read sequence
HAL_StatusTypeDef Light_Sensor_Read_Value_Memory(uint16_t *output_value)
{
	uint8_t register_content[2] = { 0 };		// create an array of the data to read

	// memory read from DATA0LOW
	HAL_StatusTypeDef read_status = HAL_I2C_Mem_Read(&hi2c2,
			LIGHT_SENSOR_ADDRESS, DATA0LOW, I2C_MEMADD_SIZE_8BIT,
			&register_content[0], 2, 100);

	*output_value = register_content[1] << 8 | register_content[0];		//adding array values to a variable

	return read_status;
}

// Implementation of the function declared in the light_sensor.h file
// It will perform a multi-byte read from registers DATA0LOW and DATA0HIGH based on the received mode parameter
uint16_t Light_Sensor_Read_Value(enum I2C_USAGE_MODE mode)
{
	HAL_StatusTypeDef read_status = HAL_ERROR;

	uint16_t light_sensor_value = 0;
	switch (mode)
	{
	case SIMPLE_MODE:
		read_status = Light_Sensor_Read_Value_Simple(&light_sensor_value);
		break;
	case MEMORY_MODE:
		read_status = Light_Sensor_Read_Value_Memory(&light_sensor_value);
		break;
	}
	// Error handling if the value writing fails
	if (read_status != HAL_OK)
	{
		Error_Handler();
	}

	return light_sensor_value;
}
