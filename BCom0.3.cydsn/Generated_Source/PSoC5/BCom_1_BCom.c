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
#include "BCom_1_BCom.h"

// @180219 ^MB Added Transport layer function
void Transport(uint8_t * buffer, uint8_t bufferSize) {
    if ( bufferSize <= 14 ) return;
    
    unsigned char input[BUFFER_SIZE];
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
            //DCP_Handler(&Frame); 
            if ( Config.Ports.DCP ) Config.Ports.DCP(&Frame);
        } else {
            struct PortHandle_s * Entry = Config.Ports.List;
            while ( Entry != NULL ) {
                if ( Entry->Port == Frame.Header->Parameter.Port ) {
                    if ( Entry->Handle != NULL ) {
                        Entry->Handle(&Frame);
                    }
                    break;
                } else {
                    Entry = Entry->next;    
                }
            }
        }    
    }
}

void Transmit(uint32_t Target, unsigned char * PayloadIn, uint8_t Size) {
    uint8 length = 12, ptr; // 20
        unsigned char output[BUFFER_SIZE];
        unsigned char SerialOut[BUFFER_SIZE];
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

        if ( Config.PutArray != NULL ) {
            Config.PutArray(SerialOut,length+2);
        }
    
}

void BCom_1_Start() {
    // Clear the ports
    Config.Ports.List = NULL;
    Config.Ports.DCP = NULL;
    Config.PutArray = NULL;
}

/* [] END OF FILE */
