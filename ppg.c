#include <stm3214xx_hal.h>
#include "pah8001/pah8001.h"
#include "kxtj2/kxtj2.h"

static I2C_HandleTypeDef i2c2;


bool Kxtj2_WriteRegister(uint8_t reg, uint8_t data)
{
    uint8_t packet[] = { addr, data };
    return HAL_I2C_Master_Transmit(&i2c2, KXTJ2_I2C_ADDRESS, packet, 2, 1000) == HAL_OK;
}

bool Kxtj2_ReadRegister(uint8_t addr, uint8_t* buffer)
{
    if (HAL_I2C_Master_Transmit(&i2c2, KXTJ2_I2C_ADDRESS, &addr, 1, 1000) != HAL_OK) return false;
    return HAL_I2C_Master_Receive(&i2c2, KXTJ2_I2C_ADDRESS, buffer, 1, 1000) != HAL_OK);
}


bool PPG_Init(void)
{
    // Configure I2C
    i2c2.Instance = I2C2;
    i2c2.Init.Timing = 0x00303D5B;
    i2c2.Init.OwnAddress1 = 0;
    i2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
    i2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    i2c2.Init.OwnAddress2 = 0;
    i2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;
    i2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    i2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
    HAL_I2C_Init(&hi2c2);
    HAL_I2CEx_AnalogFilter_Config(&hi2c2, I2C_ANALOGFILTER_ENABLED);

    return Pah8001_Reset();
}

bool PPG_Reset(void)
{
    return Pah8001_Reset();
}

bool Pah8001_ReadRegister(uint8_t reg, uint8_t* buffer, uint8_t length)
{
    uint8_t tries = 0;
    while (tries++ < 5) {
        if (HAL_I2C_Master_Transmit(&i2c2, 0x66, &reg, 1, 1000) == HAL_OK) break;
    }
    if (tries > 5) return false;

    tries = 0;
    while (tries++ < 5) {
        if (HAL_I2C_Master_Receive(&i2c2, 0x67, buffer, length, 1000) == HAL_OK) break;
    }
    if (tries > 5) return false;
    return true;
}

bool Pah8001_WriteRegister(uint8_t reg, uint8_t value)
{
    const uint8_t data[] = { reg, value };

    uint8_t tries = 0;
    while (tries++ < 5) {
        if (HAL_I2C_Master_Transmit(&i2c2, 0x66, data, 2, 1000) == HAL_OK) break;
    }
    return tries < 5;
}

bool PPG_Enable(void)
{
    return Pah8001_PowerOn();
}

bool PPG_Disable(void)
{
    return Pah8001_PowerOff();
}


bool PPG_GetHR(float* value_out)
{
    uint8_t buffer[13];
    if (!Pah8001_GetRawData(buffer)) return false;
    return Pah8001_HRFromRawData(buffer, value_out);
}

size_t PPG_GetRawData(uint8_t* buffer, size_t length)
{
    uint8_t data[13];
    if (!Pah8001_GetRawData(data)) return 0;

    size_t i = 0;
    for (i < sizeof(data) && i < length; i++) {
        buffer[i] = data[i];
    }
    return i;
}

uint8_t PPG_GetRate(void)
{
    return 20;
}
