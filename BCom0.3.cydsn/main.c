/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include "BCom_1_checksum.h"
#include "BCom_1_base64.h"
#include "BCom_1_BCom.h"
#include <stdio.h>
#include "common.h"

extern uint32  StripLights_ledArray[StripLights_ARRAY_ROWS][StripLights_ARRAY_COLS];
extern uint32 StripLights_CLUT[];

/* Add an explicit reference to the floating point printf library to allow
the usage of floating point conversion specifier */
#if defined (__GNUC__)
    asm (".global _printf_float");
#endif
#define UART_PRINTF_ENABLED         ENABLED

#define _MESSAGE_SIZE 128

uint8 errorStatus = 0u;
unsigned char inputBuffer[_MESSAGE_SIZE];
uint8_t bufferPtr = 0;
uint8_t frameReady = 0;
uint8_t BufferOverrun = 0;
CY_ISR(RxIsr)
{
    isr_rx_ClearPending();
    int i;
    uint16 data;
    if ( frameReady > 0 ) {
        BufferOverrun++;
        UART_ClearRxBuffer();
    } else {
        for ( i = 0 ; (i <= UART_GetRxBufferSize())  ; i++ ) {
            data = (UART_GetByte());
            unsigned char chr = (unsigned char) (data & 0x000000FF);
            if (chr == ':' ) {
                bufferPtr = 0;
            } else if ((chr == '\r') || (chr == '\n')) {
                frameReady = 1;  
            } else {
                if ( bufferPtr < _MESSAGE_SIZE) {
                    inputBuffer[bufferPtr++] = chr;
                }
            } 
        }
    }
}


void setup() {
    uint32 UID[2] = {0x00,0x00};
    CyGetUniqueId(UID); // test for it.. Might be the best source for the Random generator. 
    Config.Network.UID = (UID[0] ^ UID[1]) & 0x3FFFFFF;  
    
    UART_Start();
    isr_rx_StartEx(RxIsr);
    CyGlobalIntEnable; /* Enable global interrupts. */
    
}

volatile uint active = 0;
volatile uint requestLedUpdate = 0;

void Port_10(struct BComFrame_s * frame) {
    StatusLed_Write(frame->Payload[0]);
    if ( !active) {
        active = 1;
        uint offset = 0;
        uint8_t * tmp = (uint8_t *) &StripLights_ledArray[0][0];
        uint i = 0;
        uint skip = 0;
        switch (frame->Payload[0]) {
            case 1: 
                // Use with offset
                offset = (frame->Payload[1]<<8) | (frame->Payload[2]) ;
                break;
            case 0: 
                offset = 0;
                break;
                
            default: 
                // clear all
                for ( i = 0 ; i < 300 ; i++ ) {
                    tmp[i] = 0;
                }
                skip = 1;
            break;
            
        }
    
        if ( !skip ) {
            for ( i = 0 ; i < frame->Lenght-4 ; i++ ) {
                tmp[i+offset] = frame->Payload[i+4];
            }
        }
        requestLedUpdate = 1;
        
        active = 0;
    }
}

void DCP_Handler(struct BComFrame_s * frame) {
    switch (frame->Payload[0]) {
        
    }
    
}


void updateString() {

}

int main(void)
{
    setup();
    struct PortHandle_s Port_10_Handle;
    Port_10_Handle.Handle = &Port_10;
    Port_10_Handle.Port = 10;
    Port_10_Handle.next = NULL;
    Config.Ports.List = &Port_10_Handle;
    
     StripLights_Start();  
	StripLights_Dim(0);
	// Clear all memory to black
	StripLights_MemClear(StripLights_BLACK);
    StripLights_Done = &updateString;
    StripLights_Trigger(1);
    
    uint16 FrameCounter = 0;
    uint32 interval = 0;
    
    for(;;)
    { 
        if ( frameReady  ) {
            Transport(inputBuffer, bufferPtr);
            frameReady = 0; // release frame. 
        }
        if ( BufferOverrun > 0 ) {
            UART_Stop();
            BufferOverrun = 0;
            UART_Start();
        }
        
        CyDelay(1);
        if ( interval >= 15000 ) {
            interval = 0;
            
            Transmit(0x12345678, (unsigned char *) "Hello, World!",13);
        }
        if ( !Button_Read() ) {
            Transmit(0x12345678, (unsigned char *) "Button: pressed",15);
             
            uint i;
                StripLights_ledArray[0][0] = 0x000000FF;
                StripLights_ledArray[0][1] = 0x0000FF00;
                StripLights_ledArray[0][2] = 0x00FF0000;
                StripLights_ledArray[0][3] = 0x00FFFFFF;
            for ( i = 4 ; i < 300 ; i++ ) {
                StripLights_ledArray[0][i] = 0xFF000000;
            }
            StripLights_Trigger(1);
            
            while (!Button_Read()) CyDelay(10);   
            Transmit(0x12345678, (unsigned char *) "Button: released",16);
            StripLights_ledArray[0][0] = 0x00000000;
            
            for ( i = 0 ; i < 300 ; i++ ) {
                //StripLights_ledArray[0][0] = 0x000000FF;
                //StripLights_ledArray[0][1] = 0x0000FF00;
                //StripLights_ledArray[0][2] = 0x00FF0000;
                StripLights_ledArray[0][i] = 0x00000000;
            }
            StripLights_Trigger(1);
        }
        
        if ( requestLedUpdate != 0 ) {
            StripLights_Trigger(1);
            requestLedUpdate = 0;
        }
        interval++;
    }
}

/* [] END OF FILE */
