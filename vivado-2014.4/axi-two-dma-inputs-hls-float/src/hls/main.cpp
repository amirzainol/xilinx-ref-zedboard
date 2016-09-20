#include "matrixmul.h"

using namespace hls;

void matrixmul(
		stream<AXI_VALUE> &in_stream_0,
		stream<AXI_VALUE> &in_stream_1,
		result_t res[MAT_DIM][MAT_DIM]
) {

#pragma HLS INLINE off

	AXI_VALUE aValue, bValue;
	mat_a_t mat_a[MAT_DIM][MAT_DIM];
	mat_b_t mat_b[MAT_DIM][MAT_DIM];
	result_t accum;

  Row: for(int i = 0; i < MAT_DIM; i++) {
    Col: for(int j = 0; j < MAT_DIM; j++) {
#pragma HLS PIPELINE II=1

    	in_stream_0.read(aValue);
    	in_stream_1.read(bValue);
		union {	unsigned int ival; mat_a_t oval; } converterA, converterB;
		converterA.ival = aValue.data;
		mat_a[i][j] = converterA.oval;
		converterB.ival = bValue.data;
		mat_b[i][j] = converterB.oval;
        accum = mat_a[i][j] + mat_b[i][j];
        res[i][j] = accum;
    }
  }
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

    result_t mat_res[MAT_DIM][MAT_DIM];
    AXI_VALUE aValue;
    int i, j;

	// do Matrix multiplication
	matrixmul(in_stream_0, in_stream_1, mat_res);

	// stream out result matrix
	write_res1: for(i=0; i< MAT_DIM; i++){
		write_res2: for(j=0; j< MAT_DIM; j++){
#pragma HLS PIPELINE
			union {	unsigned int oval; result_t ival; } converter;
			converter.ival = mat_res[i][j];
			aValue.data = converter.oval;
			aValue.last = ((i==MAT_DIM-1)&&(j==MAT_DIM-1))? 1 : 0;
			aValue.strb = -1;
			aValue.keep = 15; //e.strb;
			aValue.user = 0;
			aValue.id = 0;
			aValue.dest = 0;
			out_stream.write(aValue);
		}
	}
}


