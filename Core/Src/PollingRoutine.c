/*
 * PollingRoutine.c
 *
 *  Created on: Oct 24, 2023
 *      Author: karl.yamashita
 *
 *
 *      Template for projects.
 *
 *      The object of this PollingRoutine.c/h files is to not have to write code in main.c which already has a lot of generated code.
 *      It is cumbersome having to scroll through all the generated code for your own code and having to find a USER CODE section so your code is not erased when CubeMX re-generates code.
 *      
 *      Direction: Call PollingInit before the main while loop. Call PollingRoutine from within the main while loop
 * 
 *      Example;
        // USER CODE BEGIN WHILE
        PollingInit();
        while (1)
        {
            PollingRoutine();
            // USER CODE END WHILE

            // USER CODE BEGIN 3
        }
        // USER CODE END 3

 */


#include "main.h"

extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart2;

extern TimerCallbackStruct timerCallback;


UART_DMA_QueueStruct uart2_msg =
{
	.huart = &huart2,
	.rx.queueSize = UART_DMA_QUEUE_SIZE,
	.tx.queueSize = UART_DMA_QUEUE_SIZE
};

ADC_Data_t adc1_msg = {0};

Interpolation_t interpol =
{
	.x1 = 10,
	.x2 = 4090,
	.y1 = 30,
	.y2 = 0
};

void PollingInit(void)
{
	// enable UART interrupt
	UART_DMA_EnableRxInterrupt(&uart2_msg);

	// calibrate ADC
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);

	// Blink LED every 1 second
	TimerCallbackRegisterOnly(&timerCallback, LED_Toggle);
	TimerCallbackTimerStart(&timerCallback, LED_Toggle, 500, true);

	// Check every 10ms to see if ADC conversion is done.
	TimerCallbackRegisterOnly(&timerCallback, Volume_Check);
	TimerCallbackTimerStart(&timerCallback, Volume_Check, 10, true);

	// Check every 10ms to see if ADC conversion is done.
	TimerCallbackRegisterOnly(&timerCallback, ADC_Check);
	TimerCallbackTimerStart(&timerCallback, ADC_Check, 1000, true);

	// start the DMA to get ADC value(s)
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc1_msg.data, 2);
}

void PollingRoutine(void)
{
	TimerCallbackCheck(&timerCallback);

}


void Volume_Check(void)
{
	char str[32] = {0};

	if(adc1_msg.adcRdy)
	{
		adc1_msg.adcRdy = false;

		// Since the ADC value will fluctuate, we will get the interpolation value instead.
		// The interpolation values are lower resolution, which will be easier to compare
		// without false jitters.
		Interpolation_Set_X(&interpol, adc1_msg.data[0]);
		Interpolation_Get_Y(&interpol);

		// convert float to uint16 prior to compare
		if((uint16_t)interpol.y_last != (uint16_t)interpol.y)
		{
			interpol.y_last = interpol.y; // save new value

			sprintf(str, "Volume=%d ", (uint8_t)interpol.y);
			UART_DMA_NotifyUser(&uart2_msg, str, strlen(str), true);
		}
	}
}

void ADC_Check(void)
{
	char str[32] = {0};
	uint16_t voltage = 0;

	voltage = (uint16_t)(adc1_msg.data[1] * ADC_RESOLUTION * 1000);
	sprintf(str, "ADC_V=%d", voltage);
	UART_DMA_NotifyUser(&uart2_msg, str, strlen(str), true);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if(hadc == &hadc1)
	{
		adc1_msg.adcRdy = true;
	}
}

void LED_Toggle(void)
{
	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart == uart2_msg.huart)
	{
		RingBuff_Ptr_Input(&uart2_msg.rx.ptr, uart2_msg.rx.queueSize);
		UART_DMA_EnableRxInterrupt(&uart2_msg);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == uart2_msg.huart)
	{
		uart2_msg.tx.txPending = false;
		UART_DMA_SendMessage(&uart2_msg);
	}
}



