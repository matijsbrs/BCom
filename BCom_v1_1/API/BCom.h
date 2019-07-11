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
    #define BUFFER_SIZE 64
    
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
    
typedef void (*`$INSTANCE_NAME`_CallBackFunction)(struct BComFrame_s * frame);
typedef void (*`$INSTANCE_NAME`_SerialOut)(const uint8_t * data, uint32_t count);    

struct PortHandle_s {
    uint8_t Port;
    `$INSTANCE_NAME`_CallBackFunction Handle;
    struct PortHandle_s * next;
};

struct GlobalConfig_s {
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
    
} Config;


void `$INSTANCE_NAME`_Start();
void `$INSTANCE_NAME`_Transport(uint8_t * buffer, uint8_t bufferSize);
void `$INSTANCE_NAME`_Transmit(uint32_t Target, uint8_t Port, unsigned char * PayloadIn, uint8_t Size);
    
    
#endif

/* [] END OF FILE */
