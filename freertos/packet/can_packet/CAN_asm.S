#include <xc.h>
#include <sys/asm.h>
#include "ISR_Support.h"

	.set	nomips16
 	.set 	noreorder

 	.extern CAN_ISR_Handler

	.extern xISRStackTop

 	.global CAN1_ISR_Wrapper
 	.global CAN2_ISR_Wrapper

/******************************************************************/
#ifdef _CAN1
 	.set		noreorder
	.set 		noat
 	.ent		CAN1_ISR_Wrapper

CAN1_ISR_Wrapper:

	portSAVE_CONTEXT

	jal 		CAN_ISR_Handler
	nop

	portRESTORE_CONTEXT

	.end CAN1_ISR_Wrapper
#endif
/******************************************************************/
#ifdef _CAN2
 	.set		noreorder
	.set 		noat
 	.ent		CAN2_ISR_Wrapper

CAN2_ISR_Wrapper:

	portSAVE_CONTEXT

	jal 		CAN_ISR_Handler
	nop

	portRESTORE_CONTEXT

	.end CAN2_ISR_Wrapper
#endif
/******************************************************************/