/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the XMC MCU: CAN Receiver example
*              for ModusToolbox. The CAN node 1 is configured to receive a CAN 
*              message over the CAN bus. Successful message reception is
*              indicated by toggling the USER LED1. The USER LED2 is updated based
*              on the message received in the CAN frame.
*
* Related Document: See README.md
*
******************************************************************************
*
* Copyright (c) 2021, Infineon Technologies AG
* All rights reserved.                        
*                                             
* Boost Software License - Version 1.0 - August 17th, 2003
* 
* Permission is hereby granted, free of charge, to any person or organization
* obtaining a copy of the software and accompanying documentation covered by
* this license (the "Software") to use, reproduce, display, distribute,
* execute, and transmit the Software, and to prepare derivative works of the
* Software, and to permit third-parties to whom the Software is furnished to
* do so, all subject to the following:
* 
* The copyright notices in the Software and this entire statement, including
* the above license grant, this restriction and the following disclaimer,
* must be included in all copies of the Software, in whole or in part, and
* all derivative works of the Software, unless such copies or derivative
* works are solely in the form of machine-executable object code generated by
* a source language processor.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
* SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
* FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*                                                                              
*****************************************************************************/

#include "cybsp.h"
#include "cy_utils.h"
#include "xmc_can.h"
#include "xmc_scu.h"
#include "cy_retarget_io.h"
#include "platform.h"
#include <stdio.h>

/*******************************************************************************
* Defines
*******************************************************************************/

/* Define macro to enable/disable printing of debug messages */
#define ENABLE_XMC_DEBUG_PRINT              (0)

/* Define macro to set the loop count before printing debug messages */
#if ENABLE_XMC_DEBUG_PRINT
#define DEBUG_LOOP_COUNT_MAX                (1U)
#endif

#define BAUD_RATE                500000    /* CAN Node baud rate in bps */
#define SAMPLE_POINT             8000      /* Sample Point. Range = [0, 10000] with respect to [0%, 100%] of the total bit time. */
#define SJW                      1         /* Synchronization Jump Width. Range: 0 - 3 */
#define CAN_IDENTIFIER           0xFF      /* 11 bits/ 29 bits CAN identifier. Sets Arbitration register */
#define CAN_IDENTIFIER_MASK      0xFF      /* CAN identifier Mask. Sets mask bits of Acceptance Mask Register. */
#define CAN_IDE_MASK             1         /* Identifier Extension Bit of Message Object. Message object receives frames only with matching IDE bit */
#define DATA_LENGTH_BYTES        1         /* CAN data Length. Range: 0 - 8 */
#define NODE1                    1         /* Node1 */
#define MESSAGE_OBJECT           0         /* Message Object 0 */

/*******************************************************************************
* Variables
*******************************************************************************/
/* Variable to indicate that CAN frame is received */
volatile bool frame_received = false;

/* CAN Bit time configuration structure */
XMC_CAN_NODE_NOMINAL_BIT_TIME_CONFIG_t baud =
{
  .can_frequency   = CAN_FREQUENCY,       /* Frequency of the CAN module in Hz */
  .baudrate        = BAUD_RATE,           /* Node baud rate in bps */
  .sample_point    = SAMPLE_POINT,        /* Sample Point */
  .sjw             = SJW,                 /* Synchronization Jump Width */
};

/*CAN message CAN_MO0 */
XMC_CAN_MO_t CAN_message =
{
  .can_mo_ptr      = CAN_MO0,                                        /* Pointer to the Message Object CAN register */
  .can_priority    = XMC_CAN_ARBITRATION_MODE_IDE_DIR_BASED_PRIO_2,  /* Arbitration Mode/Priority */
  .can_identifier  = CAN_IDENTIFIER,                                 /* Standard Identifier */
  .can_id_mask     = CAN_IDENTIFIER_MASK,                            /* CAN Identifier of MO */
  .can_id_mode     = XMC_CAN_FRAME_TYPE_STANDARD_11BITS,             /* Standard identifier */
  .can_ide_mask    = CAN_IDE_MASK,                                   /* Identifier Extension Bit */
  .can_data_length = DATA_LENGTH_BYTES,                              /* Message data length */
  .can_mo_type     = XMC_CAN_MO_TYPE_RECMSGOBJ                       /* Receive Message Object */
};


/*******************************************************************************
* Function Name: CAN_IRQ_HANDLER
********************************************************************************
* Summary:
* This is the interrupt handler function for the CAN node
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void CAN_IRQ_HANDLER(void)
{
    /* Receive the message in the CAN_message MO */
    XMC_CAN_MO_Receive(&CAN_message);

    /* Toggle LED1 to indicate that the message is received */
    XMC_GPIO_ToggleOutput(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);

    /* Set the frame received flag to true */
    frame_received = true;
}

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function. This function performs
*  - initial setup of device
*  - initialize CAN Node and receive message object
*  - prints the received CAN message in the serial terminal and turns the
*    USER LED2 based on the command received.
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize retarget-io to use the debug UART port */
    cy_retarget_io_init(CYBSP_DEBUG_UART_HW);

    #if ENABLE_XMC_DEBUG_PRINT
    printf("Initialization done\r\n");
    #endif

    printf("\r\n*********************************\r\n");
    printf("CAN Receiver Example project\n\r");
    printf("*********************************\r\n");

    /* Configure the CAN module timer's clock rate */
    XMC_CAN_InitEx(CAN, CAN_CLOCK_SOURCE, CAN_FREQUENCY);

    /* Configure CAN Node baud rate*/
    XMC_CAN_NODE_NominalBitTimeConfigureEx(CAN_NODE, &baud);

    /* NODE 1 initialization */
    /* Allow to change the configuration of the CAN node 1 */
    XMC_CAN_NODE_EnableConfigurationChange(CAN_NODE);

    /* Disable CAN node participation in CAN traffic */
    XMC_CAN_NODE_SetInitBit(CAN_NODE);

    /* Set CAN input receive pin */
    XMC_CAN_NODE_SetReceiveInput(CAN_NODE, XMC_CAN_NODE_RECEIVE_INPUT);

    /* Connect CAN TX pin to the CAN Node 1 */
    XMC_GPIO_SetMode(CYBSP_CAN_TX_PORT, CYBSP_CAN_TX_PIN, CAN_TX_PIN_MODE);

    /* Initializes CAN Message Object 0 */
    XMC_CAN_MO_Config(&CAN_message);

    /* Allocate Message object 0 to Node 1 */
    XMC_CAN_AllocateMOtoNodeList(CAN, NODE_NUMBER, MESSAGE_OBJECT);

    /* Enable receive interrupt for message */
    XMC_CAN_MO_EnableEvent(&CAN_message, XMC_CAN_MO_EVENT_RECEIVE);

    /* Configure Message Object event node pointer with service_request number */
    XMC_CAN_MO_SetEventNodePointer(&CAN_message, XMC_CAN_MO_POINTER_EVENT_RECEIVE, CAN_SERVICE_REQUEST);

#if(UC_device == XMC14)
    /* Interrupt Multiplexer configuration */
    XMC_SCU_SetInterruptControl(IRQ_NUMBER, XMC_SCU_IRQCTRL_CAN0_SR3_IRQ7);
#endif

    /* Enable NVIC node */
    NVIC_EnableIRQ(IRQ_NUMBER);

    /* Lock the configuration of CAN node 1 */
    XMC_CAN_NODE_DisableConfigurationChange(CAN_NODE);

    /* Enable CAN node 1 participation in CAN traffic */
    XMC_CAN_NODE_ResetInitBit(CAN_NODE);
    for(;;)
    {
        /* CAN Frame is received */
        if(frame_received)
        {
            /* Print the received frame in serial terminal */
            printf("Received CAN frame\n\r");
            printf( "Button State: %x\n\r", CAN_message.can_data_byte[0]);

#ifdef CYBSP_USER_LED2_PIN

            /* Update USER LED2 based on command received */
            if(CAN_message.can_data_byte[0] == 0)
            {
                XMC_GPIO_SetOutputLow(CYBSP_USER_LED2_PORT, CYBSP_USER_LED2_PIN);
            }
            else
            {
                XMC_GPIO_SetOutputHigh(CYBSP_USER_LED2_PORT, CYBSP_USER_LED2_PIN);
            }
#endif
            /* Reset flag */
            frame_received = false;
        }
    }
}

/* [] END OF FILE */
