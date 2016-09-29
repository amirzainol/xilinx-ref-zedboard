#include <stdio.h>
#include "platform.h"
#include "xaxidma.h"
#include "xparameters.h"
#include "xfunc_hls_core.h"

#define DMA_DEV_ID0		XPAR_AXI_DMA_0_DEVICE_ID
#define DMA_DEV_ID1		XPAR_AXI_DMA_1_DEVICE_ID
#define DMA_DEV_ID2		XPAR_AXI_DMA_2_DEVICE_ID // Loopback DMA
#define DMA_DEV_ID3		XPAR_AXI_DMA_3_DEVICE_ID

#define MEM_BASE_ADDR		(XPAR_PS7_DDR_0_S_AXI_BASEADDR + 0x10100000)

#define TX_BUFFER_BASE0		(MEM_BASE_ADDR + 0x00100000)
#define TX_BUFFER_BASE1		(MEM_BASE_ADDR + 0x02100000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x04100000) // max 8 MB axi dma
#define TMP_BUFFER			(MEM_BASE_ADDR + 0x06100000) // Loopback DMA

#define MAX_PKT_LEN_WORDS	1048576
#define MAX_PKT_LEN		MAX_PKT_LEN_WORDS*4

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
XAxiDma AxiDma2; // Loopback DMA
XAxiDma AxiDma3;

// the HW Acc instance
XFunc_hls_core XAxilite_add_HW_dev;
XFunc_hls_core_Config XAxilite_add_HW_config = { 0, XPAR_FUNC_HLS_CORE_0_S_AXI_CONTROL_BUS_BASEADDR };

union ufloat
{
   float f;
   unsigned u;
} x;

int main()
{
	int Status;

    init_platform();

	Status = XAxiDma_SimplePollExample();

	if (Status != XST_SUCCESS) {

		xil_printf("XAxiDma_SimplePollExample: Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("XAxiDma_SimplePollExample: Passed!\r\n");

	return XST_SUCCESS;
}

int XAxiDma_SimplePollExample()
{
	int Status;
	u32 Index;
	u32 *TxBufferPtr0;
	u32 *TxBufferPtr1;
	u32 *RxBufferPtr;
	u32 *TmpBufferPtr; // Loopback DMA
	float Value;
	int i;
	int sim_time = 100;

	TxBufferPtr0 = (u32 *)TX_BUFFER_BASE0;
	TxBufferPtr1 = (u32 *)TX_BUFFER_BASE1;
	RxBufferPtr  = (u32 *)RX_BUFFER_BASE;
	TmpBufferPtr = (u32 *)TMP_BUFFER; // Loopback DMA

	// Run the HW Accelerator
	Status = XFunc_hls_core_CfgInitialize(&XAxilite_add_HW_dev,
										&XAxilite_add_HW_config);
	if(Status != XST_SUCCESS){
		xil_printf("Error: example setup failed\r\n");
		return XST_FAILURE;
	}

	// The interrupt is not connected
	XFunc_hls_core_InterruptGlobalDisable(&XAxilite_add_HW_dev);
	XFunc_hls_core_InterruptDisable(&XAxilite_add_HW_dev, 1);

	// Start the accelerator
	XFunc_hls_core_Start(&XAxilite_add_HW_dev);

	// Enable auto restart the accelerator,
	// in such a way that it is ready for the next transaction
	XFunc_hls_core_EnableAutoRestart(&XAxilite_add_HW_dev);

	/* Initialize the XAxiDma device.
	 */
	Status = init_dma(DMA_DEV_ID0, &AxiDma0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = init_dma(DMA_DEV_ID1, &AxiDma1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Loopback DMA
	Status = init_dma(DMA_DEV_ID2, &AxiDma2);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = init_dma(DMA_DEV_ID3, &AxiDma3);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

    for (i = sim_time; i !=0; i--)
    {
    	Xil_DCacheFlushRange((u32)TxBufferPtr0, MAX_PKT_LEN);
    	Xil_DCacheFlushRange((u32)TxBufferPtr1, MAX_PKT_LEN);

		Status = XAxiDma_SimpleTransfer(&AxiDma3,(u32) RxBufferPtr,
					MAX_PKT_LEN, XAXIDMA_DEVICE_TO_DMA);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XAxiDma_SimpleTransfer(&AxiDma0,(u32) TxBufferPtr0,
					MAX_PKT_LEN, XAXIDMA_DMA_TO_DEVICE);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XAxiDma_SimpleTransfer(&AxiDma1,(u32) TxBufferPtr1,
					MAX_PKT_LEN, XAXIDMA_DMA_TO_DEVICE);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		while (XAxiDma_Busy(&AxiDma0,XAXIDMA_DMA_TO_DEVICE)) {}

		while (XAxiDma_Busy(&AxiDma1,XAXIDMA_DMA_TO_DEVICE)) {}

		while (XAxiDma_Busy(&AxiDma3,XAXIDMA_DEVICE_TO_DMA)) {}

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// tmp = temp
		Xil_DCacheFlushRange((u32)TxBufferPtr0, MAX_PKT_LEN);

		Status = XAxiDma_SimpleTransfer(&AxiDma2,(u32) TmpBufferPtr,
				MAX_PKT_LEN, XAXIDMA_DEVICE_TO_DMA);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XAxiDma_SimpleTransfer(&AxiDma2,(u32) TxBufferPtr0,
				MAX_PKT_LEN, XAXIDMA_DMA_TO_DEVICE);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		while (XAxiDma_Busy(&AxiDma2,XAXIDMA_DMA_TO_DEVICE)) {}

		while (XAxiDma_Busy(&AxiDma2,XAXIDMA_DEVICE_TO_DMA)) {}

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// temp = out
		Xil_DCacheFlushRange((u32)RxBufferPtr, MAX_PKT_LEN);

		Status = XAxiDma_SimpleTransfer(&AxiDma2,(u32) TxBufferPtr0,
			MAX_PKT_LEN, XAXIDMA_DEVICE_TO_DMA);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XAxiDma_SimpleTransfer(&AxiDma2,(u32) RxBufferPtr,
			MAX_PKT_LEN, XAXIDMA_DMA_TO_DEVICE);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		while (XAxiDma_Busy(&AxiDma2,XAXIDMA_DMA_TO_DEVICE)) {}

		while (XAxiDma_Busy(&AxiDma2,XAXIDMA_DEVICE_TO_DMA)) {}

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// out = tmp
		Xil_DCacheFlushRange((u32)TmpBufferPtr, MAX_PKT_LEN);

		Status = XAxiDma_SimpleTransfer(&AxiDma2,(u32) RxBufferPtr,
			MAX_PKT_LEN, XAXIDMA_DEVICE_TO_DMA);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XAxiDma_SimpleTransfer(&AxiDma2,(u32) TmpBufferPtr,
			MAX_PKT_LEN, XAXIDMA_DMA_TO_DEVICE);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		while (XAxiDma_Busy(&AxiDma2,XAXIDMA_DMA_TO_DEVICE)) {}

		while (XAxiDma_Busy(&AxiDma2,XAXIDMA_DEVICE_TO_DMA)) {}

		xil_printf("Done sim_time: %d\r\n", i);
    }
		/*
		Status = CheckData();
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		*/

	return XST_SUCCESS;
}

static int CheckData(void)
{
	u32 *RxPacket;
	u32 Index = 0;

	RxPacket = (u32 *) RX_BUFFER_BASE;
	Xil_DCacheInvalidateRange((u32)RxPacket, MAX_PKT_LEN);

	for(Index = 0; Index < MAX_PKT_LEN_WORDS; Index++) {
		x.u = (unsigned int)RxPacket[Index];
		printf("%.6f\r\n", x.f);
	}

	xil_printf("\r\n");

	return XST_SUCCESS;
}

int init_dma(u16 DeviceId, XAxiDma * DMAptr) {

	XAxiDma_Config *CfgPtr;
	int Status;

	CfgPtr = XAxiDma_LookupConfig(DeviceId);
	if (!CfgPtr) {
		xil_printf("No config found for %d\r\n", DeviceId);
		return XST_FAILURE;
	}

	Status = XAxiDma_CfgInitialize(DMAptr, CfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	if(XAxiDma_HasSg(DMAptr)){
		xil_printf("Device configured as SG mode \r\n");
		return XST_FAILURE;
	}

	XAxiDma_IntrDisable(DMAptr, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(DMAptr, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DMA_TO_DEVICE);

	return XST_SUCCESS;
}
