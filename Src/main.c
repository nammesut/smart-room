#pragma once

#include "liquidcrystal_i2c.c"
#include "stm32f10x_i2c.c"
#include "stm32f10x_gpio.c"

#define PORT_DS18B20    GPIOB
#define PIN_DS18B20     GPIO_PIN_9

#define ON              1
#define OFF             0

typedef enum {
    ENTER = 0,
    EXIT     ,
    NONE
}PersonStatusType;

typedef struct {
    int8_t controlByTemp;
    int8_t turnOnFan;
}SetUpFanType;

void I2C_LCD_Config();
void GPIO_Fan_Config();
void GPIO_CountPerson_Config();
void GPIO_SetState(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIOMode_TypeDef GPIO_Mode);
int8_t DS18B20_GetTemp(void);
int8_t DS18B20_Start(void);
uint8_t DS18B20_Read(void);
void DS18B20_Write(uint8_t data);
void HandleFan();
void ShowScreen();

PersonStatusType GetPersonStatus();
void HandleLight();

static SetUpFanType SetUpFan;
static int8_t s8Temp = 0;
static int32_t s32Per = 0;


int main()
{
    GPIO_Fan_Config();
    GPIO_CountPerson_Config();
    I2C_LCD_Config();
    HD44780_Init(2);
    HD44780_Clear();

    SetUpFan.controlByTemp = ON;
    SetUpFan.turnOnFan = 35;

    while(1)
    {
        s8Temp = DS18B20_GetTemp();
        
        HandleLight();
        HandleFan();
        ShowScreen();
    }

    return 0;
}

void I2C_LCD_Config()
{
    GPIO_InitTypeDef    GPIO_InitStructure;
    I2C_InitTypeDef     I2C_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    GPIO_StructInit(&GPIO_InitStructure);

    /*SCL: PB6*/
    /*SDA: PB7*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    I2C_StructInit(&I2C_InitStructure);
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000;

    I2C_Init(I2C1, &I2C_InitStructure);
    I2C_Cmd(I2C1, ENABLE);
}

void GPIO_Fan_Config()
{
    /* Config pin Fan */
    /* PC13 - Fan */
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}

void GPIO_CountPerson_Config()
{
    /* Config 2 pins sensor  */
    /* PB10 - Sensor Enter */
    /* PB11 - Sensor Exit */
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Config light  */
    /* PC14 - Light */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOC, GPIO_Pin_14);
}

void GPIO_SetState(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIOMode_TypeDef GPIO_Mode)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOx, &GPIO_InitStructure);
}

void HandleFan()
{
    if(SetUpFan.controlByTemp == ON)
    {
        /* Temp >= 35 turn on fan */
        if(s8Temp >= SetUpFan.turnOnFan)
        {
            GPIO_SetBits(GPIOC, GPIO_Pin_13);
        }
        else
        {
            GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        }
    }
    else
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
    }
}

void ShowScreen()
{
    char str[16];

    /* Display Temperature */
    HD44780_SetCursor(0, 0);
    HD44780_PrintStr("Temp: ");
    HD44780_SetCursor(8, 0);
    sprintf(&str[0], "%0.2d", s8Temp);
    HD44780_PrintStr(&str[0]);
    HD44780_SetCursor(10, 0);
    HD44780_PrintSpecialChar(0xDF);
    HD44780_PrintStr("C ");

    /* Display Person in room */
    HD44780_SetCursor(0, 1);
    HD44780_PrintStr("Per in rm: ");
    HD44780_SetCursor(8, 0);
    sprintf(&str[8], "%d", s32Per);
    HD44780_PrintStr(&str[8]);
}

int8_t DS18B20_Start()
{
	int8_t Response = 0;
	GPIO_SetState(PORT_DS18B20, PIN_DS18B20, GPIO_Mode_Out_OD);  // set the pin as output
	GPIO_ResetBits(PORT_DS18B20, PIN_DS18B20);  // pull the pin low
	HAL_Delay(480);   // delay according to datasheet

	GPIO_SetState(PORT_DS18B20, PIN_DS18B20, GPIO_Mode_IN_FLOATING);    // set the pin as input
	HAL_Delay(80);    // delay according to datasheet

	if (!(GPIO_ReadInputDataBit(PORT_DS18B20, PIN_DS18B20))) Response = 1;    // if the pin is low i.e the presence pulse is detected
	else Response = -1;

	HAL_Delay(400); // 480 us delay totally.

	return Response;
}

void DS18B20_Write(uint8_t data)
{
    uint8_t i = 0;
	GPIO_SetState(PORT_DS18B20, PIN_DS18B20, GPIO_Mode_Out_OD);  // set as output

	for (i=0; i<8; i++)
	{

		if ((data & (1<<i))!=0)  // if the bit is high
		{
			// write 1

			GPIO_SetState(PORT_DS18B20, PIN_DS18B20, GPIO_Mode_Out_OD);  // set as output
			GPIO_ResetBits(PORT_DS18B20, PIN_DS18B20);  // pull the pin LOW
			HAL_Delay(1);  // wait for 1 us

			GPIO_SetState(PORT_DS18B20, PIN_DS18B20, GPIO_Mode_IN_FLOATING);  // set as input
			HAL_Delay(50);  // wait for 60 us
		}

		else  // if the bit is low
		{
			// write 0

			GPIO_SetState(PORT_DS18B20, PIN_DS18B20, GPIO_Mode_Out_OD);
			GPIO_ResetBits(PORT_DS18B20, PIN_DS18B20);  // pull the pin LOW
			HAL_Delay(50);  // wait for 60 us

			GPIO_SetState(PORT_DS18B20, PIN_DS18B20, GPIO_Mode_IN_FLOATING);
		}
	}
}

uint8_t DS18B20_Read()
{
	uint8_t value = 0;
    uint8_t i = 0;
	GPIO_SetState(PORT_DS18B20, PIN_DS18B20, GPIO_Mode_IN_FLOATING);

	for (i=0;i<8;i++)
	{
		GPIO_SetState(PORT_DS18B20, PIN_DS18B20, GPIO_Mode_Out_OD);   // set as output

		GPIO_ResetBits(PORT_DS18B20, PIN_DS18B20);  // pull the data pin LOW
		HAL_Delay(2);;  // wait for 2 us

		GPIO_SetState(PORT_DS18B20, PIN_DS18B20, GPIO_Mode_IN_FLOATING);  // set as input
		if (GPIO_ReadInputDataBit(PORT_DS18B20, PIN_DS18B20))  // if the pin is HIGH
		{
			value |= 1<<i;  // read = 1
		}
		HAL_Delay(60);  // wait for 60 us
	}
	return value;
}

int8_t DS18B20_GetTemp()
{
    uint8_t Temp_byte1, Temp_byte2;
    (void)DS18B20_Start();
    HAL_Delay(1);
    DS18B20_Write (0xCC);  // skip ROM
    DS18B20_Write (0x44);  // convert t
    HAL_Delay (800);

    (void)DS18B20_Start();
    HAL_Delay(1);
    DS18B20_Write (0xCC);  // skip ROM
    DS18B20_Write (0xBE);  // Read Scratch-pad

    Temp_byte1 = DS18B20_Read();
    Temp_byte2 = DS18B20_Read();

    return (int8_t)(((Temp_byte2 << 8) | Temp_byte1)/16);
}

PersonStatusType GetPersonStatus()
{
    PersonStatusType status = NONE;

    if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == 0)
    {
        DelayUS(80);
        if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == 0)
        {
            status = ENTER;
        }
    }
    else if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)
    {
        DelayUS(80);
        if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)
        {
            status = EXIT;
        }
    }

    return status;
}

void HandleLight()
{
    if(ENTER == GetPersonStatus())
    {
        while(ENTER == GetPersonStatus())
        {

        }
        if(EXIT == GetPersonStatus())
        {
            while(EXIT == GetPersonStatus())
            {

            }

            s32Per++;
        }
    }

    if(EXIT == GetPersonStatus())
    {
        while(EXIT == GetPersonStatus())
        {

        }
        if(ENTER == GetPersonStatus())
        {
            while(ENTER == GetPersonStatus())
            {

            }

            s32Per--;
            if(s32Per <= 0)
            {
                s32Per = 0;
            }
        }
    }

    if(s32Per == 0)
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_14);
    }
    else
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_14);
    }
}