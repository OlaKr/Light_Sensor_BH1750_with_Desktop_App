#ifndef BH1750_H_
#define BH1750_H_

#include <stdint.h>
#include "i2c.h"

#define	BH1750_RESET                    0x07
#define	BH1750_DEFAULT_MTREG	        69
#define BH1750_MEASUREMENT_ACCURACY     1.2

#define TIMEOUT                         10

typedef enum {
    BH1750_POWER_DOWN   = 0x00,
    BH1750_POWER_ON	    = 0x01
} BH1750_powerState;

typedef enum {
    CONTINUOUS_H_RESOLUTION_MODE  	= 0x10,
    CONTINUOUS_H_RESOLUTION_MODE_2 	= 0x11,
    CONTINUOUS_L_RESOLUTION_MODE 	= 0x13,
    ONE_TIME_H_RESOLUTION_MODE 		= 0x20,
    ONE_TIME_H_RESOLUTION_MODE_2 	= 0x21,
    ONE_TIME_L_RESOLUTION_MODE 		= 0x23
}BH1750_mode;

typedef enum {
	BH1750_OK		= 1,
	BH1750_ERROR	= 0
} BH1750_Status;

typedef struct {
    I2C_HandleTypeDef 	*BH1750_i2c;	// I2C interface handler
    BH1750_mode 		BH1750_Mode;	// Current BH1750 mode
    uint8_t				BH1750_MTreg;   // Current BH1750 MTreg value
    uint16_t            BH1750_ADDR;    // BH1750 address
} BH1750_t;

/**
 * Initializes the given sensor
 *
 * @param sensor pointer to a BH1750_t structure
 * @return BH1750_OK if sensor initialized successfully, BH1750_ERROR otherwise
*/
BH1750_Status BH1750_Init(BH1750_t *sensor);
/**
 * Changes the BH1750 power state
 *
 * @param sensor pointer to a BH1750_t structure
 * @param powerState power state
 * @return BH1750_OK if power state was changed successfully, BH1750_ERROR otherwise
*/
BH1750_Status BH1750_changePowerState(BH1750_t *sensor, BH1750_powerState powerState);

/**
 * Reset of the BH1750
 *
 * @param sensor pointer to a BH1750_t structure
 * @return BH1750_OK if the reset was successful, BH1750_ERROR otherwise
*/
BH1750_Status BH1750_Reset(BH1750_t *sensor);

/**
 * Changes the BH1750 mode
 *
 * @param sensor pointer to a BH1750_t structure
 * @param mode selected mode of BH1750
 * @return BH1750_OK if mode was changed successfully, BH1750_ERROR otherwise
*/
BH1750_Status BH1750_setMode(BH1750_t *sensor, BH1750_mode mode);

/**
 * Changes the MTreg value
 *
 * @param sensor pointer to a BH1750_t structure
 * @param MTreg value of MTreg
 * @return BH1750_OK if the MTreg value was changed successfully, BH1750_ERROR otherwise
*/
BH1750_Status BH1750_setMTreg(BH1750_t *sensor, uint8_t MTreg);

/**
 * Read value from BH1750
 *
 * @param sensor pointer to a BH1750_t structure
 * @param result result pointer
 * @return BH1750_OK if the reading of the value was successful, BH1750_ERROR otherwise
*/
BH1750_Status BH1750_Read(BH1750_t *sensor, float *result);

#endif /* BH1750_H_ */