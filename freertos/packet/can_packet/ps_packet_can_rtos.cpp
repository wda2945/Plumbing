//
//  ps_packet_can_rtos.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//
//#include "can_config.h"
#include "ps_packet_can_rtos.hpp"

ps_packet_can_module& the_can_module()
{
    static ps_packet_can_rtos can_channel(CAN1);
    return can_channel;
}

ps_packet_can_rtos::ps_packet_can_rtos(CAN_MODULE _can)
{
	myModule = _can;
    
    //configure the module
    
    CANEnableModule(CAN1, TRUE);
    
    /* Place the CAN module in Configuration mode. */
    CANSetOperatingMode(CAN1, CAN_CONFIGURATION);
    while (CANGetOperatingMode(CAN1) != CAN_CONFIGURATION);

	//allocate buffer memory
    CANAssignMemoryBuffer(CAN1, can_buffer, BUFFER_SIZE * 4);
    
    int i;
    int next_channel = 0;
    for (i=0; i<TRANSMIT_CHANNELS; i++)
    {
        CANConfigureChannelForTx(CAN1, (CAN_CHANNEL) next_channel, TRANSMIT_FIFO, CAN_TX_RTR_DISABLED, (CAN_TXCHANNEL_PRIORITY) i);
        next_channel++;
    }
    next_receive_channel = next_channel;
    for (i=0; i<RECEIVE_CHANNELS; i++)
    {
        CANConfigureChannelForRx(CAN1, (CAN_CHANNEL) next_channel, RECEIVE_FIFO, CAN_RX_FULL_RECEIVE);
        next_channel++;
    }	
    
    //set speed
    CAN_BIT_CONFIG bit_config;
    bit_config.phaseSeg1Tq = (CAN_BIT_TQ) (PHASE1_SEGMENT - 1);
    bit_config.phaseSeg2TimeSelect = TRUE;
    bit_config.phaseSeg2Tq = (CAN_BIT_TQ) (PHASE2_SEGMENT - 1);
    bit_config.propagationSegTq = (CAN_BIT_TQ) (PROPAGATION_SEGMENT - 1);
    bit_config.syncJumpWidth = (CAN_BIT_TQ) (SYNC_JUMP_WIDTH - 1);
    bit_config.sample3Time = TRUE;
    CANSetSpeed(CAN1, &bit_config, GetPeripheralClock(), CAN_BAUD);
    
	//set interrupts for received packets
    
    
    //normal operation
    CANSetOperatingMode(CAN1, CAN_NORMAL_OPERATION);
    while (CANGetOperatingMode(CAN1) != CAN_NORMAL_OPERATION);
}

ps_packet_can_rtos::~ps_packet_can_rtos()
{
	//disable the module
}

//specify the mask to apply to all filters

void ps_packet_can_rtos::can_set_filter_mask(can_address_t addr) {
    //load a message filter
}

//send a packet

ps_result_enum ps_packet_can_rtos::can_send_packet(const can_packet_t *msg, int priority) {
    
    //select TX FIFO based on priority
    CANTxMessageBuffer *buffer = CANGetTxMessageBuffer(CAN1, priority);
    
    if (buffer)
    {
        int i;
        for (i=0; i<4; i++)
        {
            buffer->messageWord[i] = msg->messageWord[i];
        }
        
        CANUpdateChannel(CAN1, priority);
        CANFlushTxChannel(CAN1, priority);
    }

    return PS_OK;
}

//add a filter
int ps_packet_can_rtos::add_can_filter(can_address_t address)
{
    return 0;
}

    //wait for received packet
    can_packet_t *get_next_can_packet()
    {
        
    }
    

//************************************************
//interrupt ISRs
//Assembly wrappers
#ifdef _CAN1
void __attribute__((interrupt(CAN_IPL), vector(_CAN1_VECTOR))) CAN1_ISR_Wrapper(void);
#endif

#ifdef _CAN2
void __attribute__((interrupt(CAN_IPL), vector(_CAN2_VECTOR))) CAN2_ISR_Wrapper(void);
#endif

//common interrupt handler
void CAN_ISR_Handler(int icode) {
    
}

//ISRs for each CAN Channel
#ifdef _CAN1
void CAN1_ISR_Handler() {
    portBASE_TYPE higherPriorityTaskWoken = pdFALSE;
    
    int icode = C1VECbits.icode;
    CAN_ISR_Handler(icode)
            
    INTClearFlag(INT_CAN1);
    
    portEND_SWITCHING_ISR(higherPriorityTaskWoken);
}
#endif

#ifdef _CAN2
void CAN2_ISR_Handler() {
    portBASE_TYPE higherPriorityTaskWoken = pdFALSE;
    
    int icode = C2VECbits.icode;
    CAN_ISR_Handler(icode)
            
    INTClearFlag(INT_CAN2);
    
    portEND_SWITCHING_ISR(higherPriorityTaskWoken);
}
#endif