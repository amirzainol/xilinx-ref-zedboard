/*
 * Copyright (c) 2009-2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

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
#include "platform.h"
#include "xaxidma.h"
#include "xparameters.h"

/*
 * Device hardware build related constants.
 */

#define DMA0_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID
#define DMA1_DEV_ID		XPAR_AXIDMA_1_DEVICE_ID

#define MEM_BASE_ADDR		(XPAR_PS7_DDR_0_S_AXI_BASEADDR + 0x10100000)

#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x02100000) // max 8 MB axi dma

// The maximum MAX_PKT_LEN_WORDS is 2097151 words (or 8388604 Bytes).
// As the width of buffer length register (in AXI DMA) is set to 23.
#define MAX_PKT_LEN_WORDS	1048576
#define MAX_PKT_LEN		MAX_PKT_LEN_WORDS*4
#define OFFSET			2097152

#define NUMBER_OF_TRANSFERS	10

/************************** Function Prototypes ******************************/

int XAxiDma_SimplePollExample();
static int CheckData(void);
int init_dma(u16 DeviceId, XAxiDma * DMAptr);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XAxiDma AxiDma0;
XAxiDma AxiDma1;

int main()
{
	int Status;

    init_platform();

	/* Run the poll example for simple transfer */
	Status = XAxiDma_SimplePollExample();

	if (Status != XST_SUCCESS) {
		xil_printf("XAxiDma_SimplePollExample: Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("XAxiDma_SimplePollExample: Passed\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* The example to do the simple transfer through polling. The constant
* NUMBER_OF_TRANSFERS defines how many times a simple transfer is repeated.
*
* @param	DeviceId is the Device Id of the XAxiDma instance
*
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if error occurs
*
* @note		None
*
*
******************************************************************************/
int XAxiDma_SimplePollExample()
{

	int Status;
	int Tries = NUMBER_OF_TRANSFERS;
	u32 Index;
	u32 *TxBufferPtr;
	u32 *RxBufferPtr;
	u32 Value;

	TxBufferPtr = (u32 *)TX_BUFFER_BASE;
	RxBufferPtr = (u32 *)RX_BUFFER_BASE;

	Status = init_dma(DMA0_DEV_ID, &AxiDma0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = init_dma(DMA1_DEV_ID, &AxiDma1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Value = 11;
	for(Index = 0; Index < MAX_PKT_LEN_WORDS; Index ++) {
			TxBufferPtr[Index] = Value;
			Value++;
	}
	// Flush the SrcBuffer before the DMA transfer, in case the Data Cache is enabled
	Xil_DCacheFlushRange((u32)TxBufferPtr, MAX_PKT_LEN);

	Value = 22;
	for(Index = OFFSET; Index < MAX_PKT_LEN_WORDS+OFFSET; Index ++) {
			TxBufferPtr[Index] = Value;
			Value++;
	}
	// Flush the SrcBuffer before the DMA transfer, in case the Data Cache is enabled
	Xil_DCacheFlushRange((u32)(TxBufferPtr+OFFSET), MAX_PKT_LEN);

	for(Index = 0; Index < Tries; Index ++) {

		Status = XAxiDma_SimpleTransfer(&AxiDma0,(u32) (RxBufferPtr+0),
					MAX_PKT_LEN, XAXIDMA_DEVICE_TO_DMA);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XAxiDma_SimpleTransfer(&AxiDma1,(u32) (RxBufferPtr+OFFSET),
					MAX_PKT_LEN, XAXIDMA_DEVICE_TO_DMA);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XAxiDma_SimpleTransfer(&AxiDma0,(u32) (TxBufferPtr+0),
					MAX_PKT_LEN, XAXIDMA_DMA_TO_DEVICE);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XAxiDma_SimpleTransfer(&AxiDma1,(u32) (TxBufferPtr+OFFSET),
					MAX_PKT_LEN, XAXIDMA_DMA_TO_DEVICE);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		while (XAxiDma_Busy(&AxiDma0,XAXIDMA_DMA_TO_DEVICE)) {}

		while (XAxiDma_Busy(&AxiDma1,XAXIDMA_DMA_TO_DEVICE)) {}

		while (XAxiDma_Busy(&AxiDma0,XAXIDMA_DEVICE_TO_DMA)) {}

		while (XAxiDma_Busy(&AxiDma1,XAXIDMA_DEVICE_TO_DMA)) {}

		Status = CheckData();
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

	}

	/* Test finishes successfully
	 */
	return XST_SUCCESS;
}



/*****************************************************************************/
/*
*
* This function checks data buffer after the DMA transfer is finished.
*
* @param	None
*
* @return
*		- XST_SUCCESS if validation is successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int CheckData(void)
{
	u32 *RxPacket;
	u32 Index = 0;

	RxPacket = (u32 *) RX_BUFFER_BASE;

	//RxPacket[MAX_PKT_LEN_WORDS-1] = MAX_PKT_LEN_WORDS-1;
	/* Invalidate the DestBuffer before receiving the data, in case the
	 * Data Cache is enabled
	 */
	Xil_DCacheInvalidateRange((u32)(RxPacket+0), MAX_PKT_LEN);
	for(Index = 0; Index < MAX_PKT_LEN_WORDS; Index++) {
		if((Index >= 0) && (Index < 5))
			xil_printf("%d ", (unsigned int)RxPacket[Index]);
		if((Index >= MAX_PKT_LEN_WORDS-5) && (Index < MAX_PKT_LEN_WORDS))
			xil_printf("%d ", (unsigned int)RxPacket[Index]);
	}
	xil_printf("\r\n");

	Xil_DCacheInvalidateRange((u32)(RxPacket+OFFSET), MAX_PKT_LEN);
	for(Index = OFFSET; Index < MAX_PKT_LEN_WORDS+OFFSET; Index++) {
		if((Index >= OFFSET) && (Index < (OFFSET+5)))
			xil_printf("%d ", (unsigned int)RxPacket[Index]);
		if((Index >= MAX_PKT_LEN_WORDS+OFFSET-5) && (Index < MAX_PKT_LEN_WORDS+OFFSET))
			xil_printf("%d ", (unsigned int)RxPacket[Index]);
	}
	xil_printf("\r\n");

	return XST_SUCCESS;
}

int init_dma(u16 DeviceId, XAxiDma * DMAptr) {

	XAxiDma_Config *CfgPtr;
	int Status;

	/* Initialize the XAxiDma device.
	 */
	CfgPtr = XAxiDma_LookupConfig(DeviceId);
	if (!CfgPtr) {
		xil_printf("No config found for %d\r\n", DeviceId);
		return XST_FAILURE;
	}

	//Status = XAxiDma_CfgInitialize(&AxiDma0, CfgPtr);
	Status = XAxiDma_CfgInitialize(DMAptr, CfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	if(XAxiDma_HasSg(DMAptr)){
		xil_printf("Device configured as SG mode \r\n");
		return XST_FAILURE;
	}

	/* Disable interrupts, we use polling mode
	 */
	XAxiDma_IntrDisable(DMAptr, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(DMAptr, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DMA_TO_DEVICE);

	return XST_SUCCESS;
}
