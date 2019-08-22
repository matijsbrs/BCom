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
#include "`$INSTANCE_NAME`_BCom.h"


//main processing routine.
void `$INSTANCE_NAME`_Runtime() {
    if ( `$INSTANCE_NAME`_Frame.State == Ready ) {
        if ( `$INSTANCE_NAME`_Frame.inputBuffer[0] == '#' ) {
            // Message
            `$INSTANCE_NAME`_MessageDecoder(`$INSTANCE_NAME`_Frame.inputBuffer, `$INSTANCE_NAME`_Frame.bufferPtr); 
        } else {
           `$INSTANCE_NAME`_Transport(`$INSTANCE_NAME`_Frame.inputBuffer, `$INSTANCE_NAME`_Frame.bufferPtr); 
        }
        `$INSTANCE_NAME`_Frame.State = Free;
    }
}

void `$INSTANCE_NAME`_MessageDecoder(uint8_t * buffer, uint8_t bufferSize) {
    
    if ( bufferSize <= 7 ) return;
    unsigned char input[`$BufferSize`];
    uint8 inpLength = b64_decode(&buffer[1],bufferSize,input);
    uint16_t CRC = (input[inpLength] << 8) | input[inpLength+1];
    input[inpLength-2] = '\0';
    
    `$INSTANCE_NAME`_Message.Subscriber = (input[0]<<8)|(input[1]);
    `$INSTANCE_NAME`_Message.Operator = input[2];
    `$INSTANCE_NAME`_Message.Payload = &input[3];
    
    if (  `$INSTANCE_NAME`_Config.MessageHandler != NULL ) {
        int8_t result = `$INSTANCE_NAME`_Config.MessageHandler(&`$INSTANCE_NAME`_Message);
        if ( `$INSTANCE_NAME`_AckReq(`$INSTANCE_NAME`_Message.Operator) ) {
            // Has AckReq
            if ( result > `$INSTANCE_NAME`_MSG_Ignore ) { 
                if ( result > 0 ) {
                    if ( result != `$INSTANCE_NAME`_MSG_Succes_ACK_Included ) {
                        // Ack    
                        `$INSTANCE_NAME`_MessageEncoder(`$INSTANCE_NAME`_Message.Subscriber,0x01,NULL, 0);
                    }
                } else {
                    if ( result != `$INSTANCE_NAME`_MSG_Failed_NACK_Included ) {
                        // Nack
                        `$INSTANCE_NAME`_MessageEncoder(`$INSTANCE_NAME`_Message.Subscriber,0x04,NULL, 0);
                    }
                }
            }
        } 
    }
}

enum `$INSTANCE_NAME`_result_e `$INSTANCE_NAME`_MessageEncoder(uint16_t subscriber, uint8_t operator, uint8_t * payload, uint8_t size) {
    unsigned char SerialOut[`$BufferSize`];
    unsigned char Buffer[`$BufferSize`];
    uint8_t ptr = 0;
    Buffer[ptr++] = (uint8_t) (subscriber>>8);
    Buffer[ptr++] = (uint8_t) (subscriber&0xFF);
    Buffer[ptr++] = operator;
    uint8_t i;
    for ( i = 0 ; i < size ; i++ ) {
        Buffer[ptr++] = payload[i];
    }
    uint16_t crc = crc_16(&Buffer[1],ptr-2);

    Buffer[ptr++] = (uint8_t) (crc & 0xFF);
    Buffer[ptr++] = (uint8_t) ((crc >> 8 ) & 0xFF);
    
    uint8_t length = b64_encode( &Buffer[0],ptr,&SerialOut[1]);
    SerialOut[0] = '#';
    SerialOut[length+1] = '\n';

    if ( `$INSTANCE_NAME`_Config.PutArray != NULL ) {
        `$INSTANCE_NAME`_Config.PutArray(SerialOut,length+2);
    }
    if ( `$INSTANCE_NAME`_Ack(operator) ) {
        return `$INSTANCE_NAME`_MSG_Succes_ACK_Included;
    }
    
    return `$INSTANCE_NAME`_MSG_Succes;
}

// @180219 ^MB Added Transport layer function
void `$INSTANCE_NAME`_Transport(uint8_t * buffer, uint8_t bufferSize) {
    if ( bufferSize <= 14 ) return;
    
    unsigned char input[`$BufferSize`];
    uint8 inpLength = b64_decode(&buffer[1],bufferSize,input);
    
    uint16_t CRC = (input[inpLength] << 8) | input[inpLength+1];
    input[inpLength-2] = '\0';
    struct BComFrame_s Frame;
    Frame.Header = ( struct FrameHeader_s * ) &input[0];
    Frame.CRC = CRC;
    Frame.Lenght = inpLength-14;
    Frame.Payload = &input[12];
    
    uint8_t accept = 0x00;
    switch (buffer[0]) {
        case ':': // Unicast 
            if (  Frame.Header->Target == `$INSTANCE_NAME`_Config.Network.UID ) 
                accept = 0xFF;
        break;
        case ';': // Multicast
                
        break;
        case '!': // Broadcast
            accept = 0xFF;                
        break;
    }
    
    if ( accept ) {
        //if ( Frame.Header->Parameter.Ack ) {
            
        //}
        if ( Frame.Header->Parameter.Port == 1 ) {
            //DCP_Handler(&Frame); 
            if ( `$INSTANCE_NAME`_Config.Ports.DCP ) `$INSTANCE_NAME`_Config.Ports.DCP(&Frame);
        } else {
            struct PortHandle_s * Entry = `$INSTANCE_NAME`_Config.Ports.List;
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

void `$INSTANCE_NAME`_Transmit(uint32_t Target, uint8_t Port, unsigned char * PayloadIn, uint8_t Size) {
    uint8 length = 12, ptr; // 20
    if ( Size == 0 ) Size = strlen((const char *)PayloadIn);
    unsigned char output[`$BufferSize`];
    unsigned char SerialOut[`$BufferSize`];
    unsigned char * payload = &output[12];
    struct FrameHeader_s * Frame = ( struct FrameHeader_s * ) &output[0];

    Frame->Target = Target;
    Frame->Source = `$INSTANCE_NAME`_Config.Network.UID;
    
    System_u.System.AckReq = 0;
    System_u.System.Ack = 0;
    System_u.System.Nack = 0;
    System_u.System.Encrypted = 0;
    System_u.System.Ping = 0;
    System_u.System.RFU = 0;
    
    Frame->Parameter.System = System_u.byteVal;
    Frame->Parameter.Port = Port;
    Frame->Parameter.FrameNr = (uint16) `$INSTANCE_NAME`_Config.Network.FrameCounter++;

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

    if ( `$INSTANCE_NAME`_Config.PutArray != NULL ) {
        `$INSTANCE_NAME`_Config.PutArray(SerialOut,length+2);
    }

}

void `$INSTANCE_NAME`_SerialLoader(uint32 data) {
    unsigned char chr = (unsigned char) (data & 0x000000FF);
    if ((chr == ':') || (chr == ';') || (chr == '!')  || (chr == '#')) {
        `$INSTANCE_NAME`_Frame.bufferPtr = 0;
        `$INSTANCE_NAME`_Frame.OverSized = 0;
        `$INSTANCE_NAME`_Frame.inputBuffer[`$INSTANCE_NAME`_Frame.bufferPtr++] = chr;
        `$INSTANCE_NAME`_Frame.State = Start;
    } else if ((chr == '\r') || (chr == '\n')) {
        if ( !`$INSTANCE_NAME`_Frame.OverSized ) 
            `$INSTANCE_NAME`_Frame.State = Ready;
    } else {
        if ( `$INSTANCE_NAME`_Frame.bufferPtr < `$BufferSize`) {
            `$INSTANCE_NAME`_Frame.inputBuffer[`$INSTANCE_NAME`_Frame.bufferPtr++] = chr;
            `$INSTANCE_NAME`_Frame.State = Downloading;
        } else {
            `$INSTANCE_NAME`_Frame.State = Error;
            if (`$INSTANCE_NAME`_Frame.OverSized < 0xFE ) 
                `$INSTANCE_NAME`_Frame.OverSized++;   
        }
    }   
}


void `$INSTANCE_NAME`_Start() {
    // Clear the ports
    `$INSTANCE_NAME`_Config.Ports.List = NULL;
    `$INSTANCE_NAME`_Config.Ports.DCP = NULL;
    `$INSTANCE_NAME`_Config.PutArray = NULL;
}

/* [] END OF FILE */
