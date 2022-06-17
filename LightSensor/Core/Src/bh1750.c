#include "bh1750.h"
#include <stdint.h>

BH1750_Status BH1750_Init(BH1750_t *sensor)
{
	if (BH1750_OK != BH1750_Reset(sensor))
		return BH1750_ERROR;

	if (BH1750_OK != BH1750_setMTreg(sensor, sensor->BH1750_MTreg))
		return BH1750_ERROR;

	if (BH1750_OK != BH1750_setMode(sensor, sensor->BH1750_Mode))
		return BH1750_ERROR;

	return BH1750_OK;
}

BH1750_Status BH1750_changePowerState(BH1750_t *sensor, BH1750_powerState powerState)
{
	if (HAL_OK != HAL_I2C_Master_Transmit(sensor->BH1750_i2c, sensor->BH1750_ADDR, &powerState, 1, TIMEOUT))
		return BH1750_ERROR;

	return BH1750_OK;
}

BH1750_Status BH1750_Reset(BH1750_t *sensor)
{
	uint8_t reset = BH1750_RESET;
	if (HAL_OK != HAL_I2C_Master_Transmit(sensor->BH1750_i2c, sensor->BH1750_ADDR, &reset, 1, TIMEOUT))
		return BH1750_ERROR;

	return BH1750_OK;
}

BH1750_Status BH1750_setMode(BH1750_t *sensor, BH1750_mode mode)
{
	if (!((mode >> 4) || (mode >> 5)))
		return BH1750_ERROR;
	if ((mode & 0x0F) > 3)
		return BH1750_ERROR;

	sensor->BH1750_Mode = mode;
	if (HAL_OK != HAL_I2C_Master_Transmit(sensor->BH1750_i2c, sensor->BH1750_ADDR, &mode, 1, TIMEOUT))
		return BH1750_ERROR;

	return BH1750_OK;
}

BH1750_Status BH1750_setMTreg(BH1750_t *sensor, uint8_t MTreg)
{
	// Range of MTreg
	if (MTreg < (uint8_t)31 || MTreg > (uint8_t)254)
		return BH1750_ERROR;

	sensor->BH1750_MTreg = MTreg;

	// Changing High bit of MTreg
	uint8_t hBit = (0x40 | (MTreg >> 5));
	if (HAL_OK != HAL_I2C_Master_Transmit(sensor->BH1750_i2c, sensor->BH1750_ADDR, &hBit, 1, TIMEOUT))
		return BH1750_ERROR;

	// Changing Low bit of MTreg
	uint8_t lBit = (0x60 | (MTreg & 0x1F));
	if (HAL_OK != HAL_I2C_Master_Transmit(sensor->BH1750_i2c, sensor->BH1750_ADDR, &lBit, 1, TIMEOUT))
		return BH1750_ERROR;

	return BH1750_OK;
}

/*
The below formula is to calculate illuminance per 1 count.
	H-reslution mode : Illuminance per 1 count ( lx / count ) = 1 / 1.2 *( 69 / X )
	H-reslution mode2 : Illuminance per 1 count ( lx / count ) = 1 / 1.2 *( 69 / X ) / 2
		1.2 : Measurement accuracy
		69 : Default value of MTreg ( dec )
		X : MTreg value
*/

BH1750_Status BH1750_Read(BH1750_t *sensor, float *result)
{
	uint8_t response[2];

	if (HAL_OK != HAL_I2C_Master_Receive(sensor->BH1750_i2c, sensor->BH1750_ADDR, response, 2, TIMEOUT))
		return BH1750_ERROR;

	// High and Low bytes
	*result = (response[0] << 8) | (response[1]);
	*result *= (float)((uint8_t)BH1750_DEFAULT_MTREG / (float)sensor->BH1750_MTreg);

	if (sensor->BH1750_Mode == ONE_TIME_H_RESOLUTION_MODE_2 || sensor->BH1750_Mode == CONTINUOUS_H_RESOLUTION_MODE_2)
		*result /= 2.0;

	*result /= (float)BH1750_MEASUREMENT_ACCURACY;
	return BH1750_OK;
}