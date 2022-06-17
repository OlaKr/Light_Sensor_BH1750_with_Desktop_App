/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdint.h>
#include "helpers.h"
#include "ring_buffer.h"
#include "bh1750.h"
#include "usart.h"
#include "main.h"

// UART transmit buffer descriptor
extern RingBuffer UART_RingBuffer_Tx;
// UART transmit buffer memory pool
extern char RingBufferData_Tx[1024];
// UART receive buffer descriptor
extern RingBuffer UART_RingBuffer_Rx;
// UART receive buffer memory pool
extern char RingBufferData_Rx[1024];

extern char Response_Rx[1024];
extern uint8_t uart_rx_buffer;

// buffer read control flag
extern uint8_t readyToRead;
// flag controlling the sending of data from the sensor
extern uint8_t readyToSend;

int __io_putchar(int ch)
{
	if (ch == '\n')
	{
		uint8_t ch2 = '\r';
		HAL_UART_Transmit(&huart2, &ch2, 1, HAL_MAX_DELAY);
	}

	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return 1;
}

bool UART_PutChar(char c)
{
	if (RingBuffer_PutChar(&UART_RingBuffer_Tx, c))
	{

		char temp;
		if (RingBuffer_GetChar(&UART_RingBuffer_Tx, &temp))
		{
			HAL_UART_Transmit(&huart2, &temp, 1, 100);
			// HAL_UART_Transmit_IT(&huart2, &temp, 1);
		}
		else
			return false;

		return true;
	}
	return false;
}

size_t UART_WriteData(const void *data, size_t dataSize)
{
	size_t i = 0;
	char const *d = (char const *)data;
	while (dataSize > 0)
	{
		i++;
		UART_PutChar(*d++);
		dataSize--;
	}
	return i;
}

size_t UART_WriteString(const char *string)
{
	return UART_WriteData(string, strlen(string) + 2);
}

bool UART_GetChar(char *c)
{
	bool temp = RingBuffer_GetChar(&UART_RingBuffer_Rx, c);
	return temp;
}

size_t UART_ReadData(char *data, size_t maxSize)
{

	size_t i = 0;

	while (maxSize > i)
	{

		if (!UART_GetChar(data + i))
		{
			return i;
		}
		i++;
	}
	return i;
}

void char_append(uint8_t value)
{
	if (value == '\r' || value == '\n')
	{ // end of line
		readyToRead = 1;
	}
	else
	{
		RingBuffer_PutChar(&UART_RingBuffer_Rx, uart_rx_buffer);
	}
}

void UART_ReadResponse()
{
	if (!RingBuffer_IsEmpty(&UART_RingBuffer_Rx))
	{
		size_t responseSize = UART_ReadData(&Response_Rx, 10);
		Response_Rx[responseSize] = '\0';
		if (strcmp(Response_Rx, "readValue") == 0)
		{
			readyToSend = 1;
		}
		else if (strcmp(Response_Rx, "incSens") == 0)
		{
			readyToSend = 2;
		}
		else if (strcmp(Response_Rx, "decSens") == 0)
		{
			readyToSend = 3;
		}
		else if (strcmp(Response_Rx, "help") == 0)
		{
			readyToSend = 4;
		}
		else
		{
			readyToSend = 5;
		}
	}
}
