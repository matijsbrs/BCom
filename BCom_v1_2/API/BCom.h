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

#ifndef BCOM_H
#define BCOM_H
#include "project.h"
    
#define `$INSTANCE_NAME`_PUSH(x)    if ( (x->Operator >>5) == 0 )
#define `$INSTANCE_NAME`_PULL(x)    if ( (x->Operator >>5) == 1 )
#define `$INSTANCE_NAME`_CLEAR(x)   if ( (x->Operator >>5) == 2 )
#define `$INSTANCE_NAME`_EXECUTE(x) if ( (x->Operator >>5) == 3 )

#define `$INSTANCE_NAME`_AckReq(x) ((x & 0x02) == 0x02)    
#define `$INSTANCE_NAME`_Ack(x) ((x & 0x01) == 0x01)    
#define `$INSTANCE_NAME`_Nack(x) ((x & 0x04) == 0x04)    
    
enum `$INSTANCE_NAME`_result_e {
    // errors
    `$INSTANCE_NAME`_MSG_Unknown_subscriber = -127,
    `$INSTANCE_NAME`_MSG_Ignore = -126,
    `$INSTANCE_NAME`_MSG_Operator_Not_used = -1,
    `$INSTANCE_NAME`_MSG_Operator_Unknown = -2,
    `$INSTANCE_NAME`_MSG_Failed_NACK_Included = -3,
    `$INSTANCE_NAME`_MSG_Unknown_Input = -4,
    // succes
    `$INSTANCE_NAME`_MSG_Succes = 1,
    `$INSTANCE_NAME`_MSG_Succes_ACK_Included = 2
    
};    
 
struct `$INSTANCE_NAME`_Message_s {
    uint16_t Subscriber;
    uint8_t Operator;
    uint8_t * Payload;
} `$INSTANCE_NAME`_Message;
    
struct `$INSTANCE_NAME`_FrameState_s {
    enum frameState_e {Free, Start, Downloading, Ready, Error} State;
    unsigned char inputBuffer[`$BufferSize`];
    uint8_t bufferPtr;
    uint8_t frameReady;
    uint8_t BufferOverrun;
    uint8_t OverSized;
} `$INSTANCE_NAME`_Frame;    
    
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
    
typedef int8_t (*`$INSTANCE_NAME`_MessageHandler)(struct `$INSTANCE_NAME`_Message_s * msg);
typedef void (*`$INSTANCE_NAME`_CallBackFunction)(struct BComFrame_s * frame);
typedef void (*`$INSTANCE_NAME`_SerialOut)(const uint8_t * data, uint32_t count);    

struct PortHandle_s {
    uint8_t Port;
    `$INSTANCE_NAME`_CallBackFunction Handle;
    struct PortHandle_s * next;
};

struct `$INSTANCE_NAME`_GlobalConfig_s {
    struct Network_s {
        uint32 TypeIdentifier;
        uint32 UID;
        char Name[20];
        uint32 FrameCounter;
    } Network;
    struct Applications_s {
        `$INSTANCE_NAME`_CallBackFunction DCP;
        struct PortHandle_s * List;
    } Ports;
    `$INSTANCE_NAME`_SerialOut PutArray;
    `$INSTANCE_NAME`_MessageHandler MessageHandler;
    
} `$INSTANCE_NAME`_Config;


void `$INSTANCE_NAME`_Start();
void `$INSTANCE_NAME`_MessageDecoder(uint8_t * buffer, uint8_t bufferSize);
enum `$INSTANCE_NAME`_result_e `$INSTANCE_NAME`_MessageEncoder(uint16_t subscriber, uint8_t operator, uint8_t * payload, uint8_t size);
void `$INSTANCE_NAME`_Transport(uint8_t * buffer, uint8_t bufferSize);
void `$INSTANCE_NAME`_Transmit(uint32_t Target, uint8_t Port, unsigned char * PayloadIn, uint8_t Size);
void `$INSTANCE_NAME`_SerialLoader(uint32 data);    
void `$INSTANCE_NAME`_Runtime();
    
#endif

/* [] END OF FILE */
