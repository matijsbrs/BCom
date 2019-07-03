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
#include "checksum.h"
#include "base64.h"
#include <stdio.h>
#include "common.h"

/* Add an explicit reference to the floating point printf library to allow
the usage of floating point conversion specifier */
#if defined (__GNUC__)
    asm (".global _printf_float");
#endif
#define UART_PRINTF_ENABLED         ENABLED

#define _MESSAGE_SIZE 128

struct GlobalConfig_s {
    struct Network_s {
        uint32 TypeIdentifier;
        uint32 UID;
        char Name[20];
        uint32 FrameCounter;
        uint8_t RFU[128];
    } Network;
    
} Config;

union _system_u {
    struct System_s {
        uint8 AckReq:1;
        uint8 Ack:1;
        uint8 Nack:1;
        uint8 Encrypted:1;
        uint8 Ping:1;
        uint8 RFU:3;   
    } System;
    uint8 byteVal;
} System_u;

struct FrameHeader_s { 
    uint32 Target;
    uint32 Source;
    struct Parameter_s {
        uint8 System;
        uint8 Port;
        uint16 FrameNr;
    } Parameter;
};

struct BComFrame_s {
    struct FrameHeader_s * Header;   
    uint16_t CRC;
    uint8_t Lenght;
    uint8_t * Payload;
};
   
//:RDMiEd3Mu6roCnsAQkNvbeRq
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

struct DeviceControlProtocolFrame_s {
    enum Command_e {
        SC_GET,
        SC_SET,
        SC_PUSH, 
        SC_STORE, 
        SC_RESTORE,
        SC_WIPE,
        SC_RESTART,
        SC_UNKOWN
    } Command;
    uint16_t Offset;
    uint8_t Length;
};

void setup() {
    uint32 UID[2] = {0x00,0x00};
    CyGetUniqueId(UID); // test for it.. Might be the best source for the Random generator. 
    Config.Network.UID = (UID[0] ^ UID[1]) & 0x3FFFFFF;  
    
    UART_Start();
    isr_rx_StartEx(RxIsr);
    CyGlobalIntEnable; /* Enable global interrupts. */
    
}


void Port_10(struct BComFrame_s * frame) {
    StatusLed_Write(frame->Payload[0]);
           
   
}

void DCP_Handler(struct BComFrame_s * frame) {
    switch (frame->Payload[0]) {
        
    }
    
}


// @180219 ^MB Added Transport layer function
void Transport(uint8_t * buffer, uint8_t bufferSize) {
    if ( bufferSize <= 14 ) return;
    
    unsigned char input[128];
    uint8 inpLength = b64_decode(buffer,bufferSize,input);
    
    uint16_t CRC = (input[inpLength] << 8) | input[inpLength+1];
    input[inpLength-2] = '\0';
    struct BComFrame_s Frame;
    Frame.Header = ( struct FrameHeader_s * ) &input[0];
    Frame.CRC = CRC;
    Frame.Lenght = inpLength-14;
    Frame.Payload = &input[12];
    
    uint8_t accept = 0x00;
    switch ((Frame.Header->Target >> 30)) {
        case 0: // Unicast 
            if ( ( Frame.Header->Target & 0x3FFFFFFF ) == Config.Network.UID ) 
                accept = 0xFF;
        break;
        case 1: // Multicast
                
        break;
        case 2: // Reserved
                
        break;
        case 3: // Broadcast
            accept = 0xFF;                
        break;
    }
    
    if ( accept ) {
        //if ( Frame.Header->Parameter.Ack ) {
            
        //}
        if ( Frame.Header->Parameter.Port == 1 ) {
            DCP_Handler(&Frame);   
        } else {
            switch(Frame.Header->Parameter.Port) {
                case 10:
                    Port_10(&Frame);
                    break;
                default:
                break;    
            }
        }    
    }
}

void Transmit(uint32_t Target, unsigned char * PayloadIn, uint8_t Size) {
    uint8 length = 12, ptr; // 20
        unsigned char output[128];
        unsigned char SerialOut[128];
        unsigned char * payload = &output[12];
        struct FrameHeader_s * Frame = ( struct FrameHeader_s * ) &output[0];

        Frame->Target = Target;
        Frame->Source = Config.Network.UID;
        
        System_u.System.AckReq = 0;
        System_u.System.Ack = 0;
        System_u.System.Nack = 0;
        System_u.System.Encrypted = 0;
        System_u.System.Ping = 0;
        System_u.System.RFU = 0;
        
        Frame->Parameter.System = System_u.byteVal;
        Frame->Parameter.Port = 10;
        Frame->Parameter.FrameNr = (uint16) Config.Network.FrameCounter++;

        for ( ptr = 0 ; ptr < Size ; ptr++ ) {
        payload[ptr] = PayloadIn[ptr];
        }
        length += Size; 

        uint16_t crc = crc_16(output,length);

        payload[ptr++] = (uint8_t) (crc & 0xFF);
        payload[ptr++] = (uint8_t) ((crc >> 8 ) & 0xFF);
        length += 2;

        length= b64_encode( &output[0],length,&SerialOut[1]);
        SerialOut[0] = ':';
        SerialOut[length+1] = '\n';

        UART_PutArray(SerialOut,length+2);
    
}


int main(void)
{
    setup();
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
            while (!Button_Read()) CyDelay(10);   
            Transmit(0x12345678, (unsigned char *) "Button: released",16);
        }
        interval++;
    }
}

/* [] END OF FILE */
