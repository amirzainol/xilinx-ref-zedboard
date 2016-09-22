#include "matrixmul.h"

float kernel[MAT_DIM*MAT_DIM] = {
		1, 0, 1,
		0, 0, 0,
		1, 0, 1,
};

#define HORIZONTAL_PIXEL_WIDTH 3
// 3x3 kernel
#define KERNEL_DIM 3

using namespace hls;

float sumWindow(hls::Window<KERNEL_DIM,KERNEL_DIM,float> *window);

void matrixmul(
		stream<AXI_VALUE> &in_stream_0,
		stream<AXI_VALUE> &in_stream_1,
		stream<AXI_VALUE> &out_stream
) {

#pragma HLS INLINE off

	AXI_VALUE aValue, bValue;
	int i,j;

	float y_buf[2][HORIZONTAL_PIXEL_WIDTH];
#pragma HLS array_partition variable=y_buf block factor=2 dim=1
#pragma HLS resource variable=y_buf core=RAM_2P

	// Defining the line buffer and setting the inter dependency to false through pragmas
	hls::LineBuffer<KERNEL_DIM,MAT_DIM,float> lineBuff;
	hls::Window<KERNEL_DIM,KERNEL_DIM,float> windowX;
	// Index used to keep track of row,col
	int idxCol = 0;
	int idxRow = 0;
	int pixConvolved = 0;
	// Calculate delay to fix line-buffer offset
	int waitTicks = (MAT_DIM*(KERNEL_DIM-1)+KERNEL_DIM)/2;// 241;
	int countWait = 0;
	int sentPixels = 0;
	int operation = 0;

	  Row: for(i = 0; i < MAT_DIM; i++) {
	    Col: for(j = 0; j < MAT_DIM; j++) {

// the HLS PIPELINE improves our latency
#pragma HLS PIPELINE

		// Read and cache (Block here if FIFO sender is empty)
		//aValue = in_stream_0.read();

    	in_stream_0.read(aValue);
    	in_stream_1.read(bValue);
		union {	unsigned int ival; mat_a_t oval; } converterA, converterB, convC;
		converterA.ival = aValue.data;
		converterB.ival = bValue.data;

		// Put data on the LineBuffer
		lineBuff.shift_up(idxCol);
		lineBuff.insert_top(converterA.oval,idxCol); // Will put in val[2] of line buffer (Check Debug)

		// Put data on the window and multiply with the kernel
		for (int idxWinRow = 0; idxWinRow < KERNEL_DIM; idxWinRow++)
		{
			for (int idxWinCol = 0; idxWinCol < KERNEL_DIM; idxWinCol++)
			{
				// idxWinCol + pixConvolved, will slide the window ...
				float val = (float)lineBuff.getval(idxWinRow,idxWinCol+pixConvolved);

				// Multiply kernel by the sampling window
				val = (float)kernel[(idxWinRow*KERNEL_DIM) + idxWinCol ] * val;
				windowX.insert(val,idxWinRow,idxWinCol);

			}

		}

		// Avoid calculate out of the image boundaries
		float valOutput = 0;
		if ((idxRow >= KERNEL_DIM-1) && (idxCol >= KERNEL_DIM-1))
		{
				valOutput = sumWindow(&windowX);
			pixConvolved++;
		}

		// Calculate row and col index
		if (idxCol < MAT_DIM-1)
		{
			idxCol++;
		}
		else
		{
			// New line
			idxCol = 0;
			idxRow++;
			pixConvolved = 0;
		}

		countWait++;
		if (countWait > waitTicks)
		{
			//bValue.data = valOutput;
			union {	unsigned int oval; result_t ival; } converter;
			converter.ival = valOutput;
			bValue.data = converter.oval;
			bValue.keep = 15;
			bValue.strb = -1;
			bValue.user = 0;
			bValue.last = 0;
			bValue.id = 0;
			bValue.dest = 0;
			//cout << "res " << valOutput << endl; // OK
			out_stream.write(bValue);
			sentPixels++;
		}
	}

} // end of for i and j

	// Now send the remaining zeros (Just the (Number of delayed ticks)
	for (countWait = 0; countWait < waitTicks; countWait++)
	{
		bValue.data = 0;
		//cout << "res " << 0 << endl; // OK
		bValue.keep = 15;
		bValue.strb = -1;
		bValue.user = 0;
		// Send last on the last item
		if (countWait < waitTicks - 1)
			bValue.last = 0;
		else
			bValue.last = 1;
		bValue.id = 0;
		bValue.dest = 0;
		out_stream.write(bValue);
	}
} // end of function

// Sum all values inside window (Already multiplied by the kernel)
float sumWindow(hls::Window<KERNEL_DIM,KERNEL_DIM,float> *window)
{
	float accumulator = 0;

	// Iterate on the window multiplying and accumulating the kernel and sampling window
	for (int idxRow = 0; idxRow < KERNEL_DIM; idxRow++)
	{
		for (int idxCol = 0; idxCol < KERNEL_DIM; idxCol++)
		{
			accumulator = accumulator + (float)window->getval(idxRow,idxCol);
		}
	}
	return accumulator;
}

// --------------------------------------------------------
// main accelerator function, interfaces with AXI-S channels
void func_hls_core (
	stream<AXI_VALUE> &in_stream_0,
	stream<AXI_VALUE> &in_stream_1,
	stream<AXI_VALUE> &out_stream)
{

	// Map HLS ports to AXI interfaces
#pragma HLS RESOURCE variable=in_stream_0  core=AXIS metadata="-bus_bundle INPUT_STREAM_0"
#pragma HLS RESOURCE variable=in_stream_1  core=AXIS metadata="-bus_bundle INPUT_STREAM_1"
#pragma HLS RESOURCE variable=out_stream core=AXIS metadata="-bus_bundle OUTPUT_STREAM"
#pragma HLS RESOURCE variable=return core=AXI4LiteS metadata="-bus_bundle CONTROL_BUS"

	// do Matrix multiplication
	matrixmul(in_stream_0, in_stream_1, out_stream);

}


