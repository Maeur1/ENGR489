/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include <string.h>
#include "xbasic_types.h"
#include "xtime_l.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xparameters.h"	/* SDK generated parameters */
#include "xsdps.h"		/* SD device driver */
#include "xil_printf.h"
#include "ff.h"

#include "Timer.h"

#define INTC_INTERRUPT_ID 	XPAR_FABRIC_TIMER_0_INTERRUPT_OUT_INTR
#define INTC_DEVICE_ID 		XPAR_PS7_SCUGIC_0_DEVICE_ID

#define NS_PER_TICK 4
#define PROCESSING_TIME_NS 448
#define RUN_TIME_USEC 20000000
#define FILE_BUFFER_SIZE 200

XScuGic InterruptController;
static XScuGic_Config *GicConfig;/* The configuration parameters of the controller */
static FIL fil;		/* File object */
static FATFS fatfs;
static char FileName[32] = "0:/test.csv";
static char *SD_File;
static uint32_t dataPoint = 0;

static char fileBuffer[FILE_BUFFER_SIZE];

static void TIMR_Intr_Handler(void *information);
static int ScuGicInterrupt_Init();

Xuint32 *baseaddr_p = (Xuint32 *)XPAR_TIMER_0_S00_AXI_BASEADDR;

void TIMR_Intr_Handler(void *information){
	UINT NumBytesWritten;
	xil_printf("Packet Detected ");
	uint32_t timeTaken = ((*(baseaddr_p+1)) * NS_PER_TICK) - PROCESSING_TIME_NS;
	xil_printf("Time Taken: %uns\n", timeTaken);
	sprintf(fileBuffer, "%6lu,%6lu\n", dataPoint, timeTaken);
	f_write(&fil, (const void*)fileBuffer, 14, &NumBytesWritten);
	dataPoint++;
}

int ScuGicInterrupt_Init(){
	int Status;

	Xil_ExceptionInit();

	GicConfig = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);


	if (NULL == GicConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, GicConfig,
			GicConfig->CpuBaseAddress);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the Interrupt System
	 * */

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
			(Xil_ExceptionHandler) XScuGic_InterruptHandler,
			(void *) &InterruptController);

	/*
	 * Connect a device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler performs
	 * the specific interrupt processing for the device
	 */
	Status = XScuGic_Connect(&InterruptController,XPAR_FABRIC_TIMER_0_INTERRUPT_OUT_INTR,
			(Xil_ExceptionHandler)TIMR_Intr_Handler,
			(void *) &InterruptController);

	XScuGic_Enable(&InterruptController, XPAR_FABRIC_TIMER_0_INTERRUPT_OUT_INTR);

	/*
	 * Enable interrupts in the ARM
	 */
	Xil_ExceptionEnable();

	//Only used for edge sensitive Interrupts
	XScuGic_SetPriorityTriggerType(&InterruptController, XPAR_FABRIC_TIMER_0_INTERRUPT_OUT_INTR,
						0x0, 3);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
}

int setupSDCard(){
	FRESULT Res;
	TCHAR *Path = "0:/";

	Res = f_mount(&fatfs, Path, 0);
	if (Res != FR_OK) {
		return XST_FAILURE;
	}

	Res = f_mkfs(Path, 0, 0);
	if (Res != FR_OK) {
		return XST_FAILURE;
	}

	SD_File = (char *)FileName;

	Res = f_open(&fil, SD_File, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	if (Res) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

int closeSDCard(){
	FRESULT Res;
	/*
	 * Close file.
	 */
	Res = f_close(&fil);
	if (Res) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

uint64_t getTimeDifferenceUSec(XTime start, XTime end){
	return (uint64_t)(1.0 * (end - start) / (COUNTS_PER_SECOND/1000000));
}

int main()
{
    XTime tStart, tEnd;

    int status;

    print("Packet Timing Program Begin\n\r");

    status = ScuGicInterrupt_Init();
    if (status != XST_SUCCESS){
    	printf("Interrupt Setup Failed\n");
		return XST_FAILURE;
    }

    status = setupSDCard();
    if (status != XST_SUCCESS){
		printf("SD Setup Failed\n");
		return XST_FAILURE;
	}

    XTime_GetTime(&tStart);
    XTime_GetTime(&tEnd);

    while(getTimeDifferenceUSec(tStart, tEnd) < RUN_TIME_USEC){XTime_GetTime(&tEnd);}

    printf("Finished Program\n");
    Xil_ExceptionDisable();
    closeSDCard();
    return 0;
}
