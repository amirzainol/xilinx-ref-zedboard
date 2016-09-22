// ==============================================================
// File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2014.4
// Copyright (C) 2014 Xilinx Inc. All rights reserved.
// 
// ==============================================================

/***************************** Include Files *********************************/
#include "xfunc_hls_core.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XFunc_hls_core_CfgInitialize(XFunc_hls_core *InstancePtr, XFunc_hls_core_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_bus_BaseAddress = ConfigPtr->Control_bus_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XFunc_hls_core_Start(XFunc_hls_core *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFunc_hls_core_ReadReg(InstancePtr->Control_bus_BaseAddress, XFUNC_HLS_CORE_CONTROL_BUS_ADDR_AP_CTRL) & 0x80;
    XFunc_hls_core_WriteReg(InstancePtr->Control_bus_BaseAddress, XFUNC_HLS_CORE_CONTROL_BUS_ADDR_AP_CTRL, Data | 0x01);
}

u32 XFunc_hls_core_IsDone(XFunc_hls_core *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFunc_hls_core_ReadReg(InstancePtr->Control_bus_BaseAddress, XFUNC_HLS_CORE_CONTROL_BUS_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XFunc_hls_core_IsIdle(XFunc_hls_core *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFunc_hls_core_ReadReg(InstancePtr->Control_bus_BaseAddress, XFUNC_HLS_CORE_CONTROL_BUS_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XFunc_hls_core_IsReady(XFunc_hls_core *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFunc_hls_core_ReadReg(InstancePtr->Control_bus_BaseAddress, XFUNC_HLS_CORE_CONTROL_BUS_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XFunc_hls_core_EnableAutoRestart(XFunc_hls_core *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFunc_hls_core_WriteReg(InstancePtr->Control_bus_BaseAddress, XFUNC_HLS_CORE_CONTROL_BUS_ADDR_AP_CTRL, 0x80);
}

void XFunc_hls_core_DisableAutoRestart(XFunc_hls_core *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFunc_hls_core_WriteReg(InstancePtr->Control_bus_BaseAddress, XFUNC_HLS_CORE_CONTROL_BUS_ADDR_AP_CTRL, 0);
}

void XFunc_hls_core_InterruptGlobalEnable(XFunc_hls_core *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFunc_hls_core_WriteReg(InstancePtr->Control_bus_BaseAddress, XFUNC_HLS_CORE_CONTROL_BUS_ADDR_GIE, 1);
}

void XFunc_hls_core_InterruptGlobalDisable(XFunc_hls_core *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFunc_hls_core_WriteReg(InstancePtr->Control_bus_BaseAddress, XFUNC_HLS_CORE_CONTROL_BUS_ADDR_GIE, 0);
}

void XFunc_hls_core_InterruptEnable(XFunc_hls_core *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XFunc_hls_core_ReadReg(InstancePtr->Control_bus_BaseAddress, XFUNC_HLS_CORE_CONTROL_BUS_ADDR_IER);
    XFunc_hls_core_WriteReg(InstancePtr->Control_bus_BaseAddress, XFUNC_HLS_CORE_CONTROL_BUS_ADDR_IER, Register | Mask);
}

void XFunc_hls_core_InterruptDisable(XFunc_hls_core *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XFunc_hls_core_ReadReg(InstancePtr->Control_bus_BaseAddress, XFUNC_HLS_CORE_CONTROL_BUS_ADDR_IER);
    XFunc_hls_core_WriteReg(InstancePtr->Control_bus_BaseAddress, XFUNC_HLS_CORE_CONTROL_BUS_ADDR_IER, Register & (~Mask));
}

void XFunc_hls_core_InterruptClear(XFunc_hls_core *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFunc_hls_core_WriteReg(InstancePtr->Control_bus_BaseAddress, XFUNC_HLS_CORE_CONTROL_BUS_ADDR_ISR, Mask);
}

u32 XFunc_hls_core_InterruptGetEnabled(XFunc_hls_core *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XFunc_hls_core_ReadReg(InstancePtr->Control_bus_BaseAddress, XFUNC_HLS_CORE_CONTROL_BUS_ADDR_IER);
}

u32 XFunc_hls_core_InterruptGetStatus(XFunc_hls_core *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XFunc_hls_core_ReadReg(InstancePtr->Control_bus_BaseAddress, XFUNC_HLS_CORE_CONTROL_BUS_ADDR_ISR);
}
