/*******************************************************************************
* File Name: Display_LCDPort.h  
* Version 2.20
*
* Description:
*  This file contains the Alias definitions for Per-Pin APIs in cypins.h. 
*  Information on using these APIs can be found in the System Reference Guide.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_Display_LCDPort_ALIASES_H) /* Pins Display_LCDPort_ALIASES_H */
#define CY_PINS_Display_LCDPort_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"


/***************************************
*              Constants        
***************************************/
#define Display_LCDPort_0			(Display_LCDPort__0__PC)
#define Display_LCDPort_0_INTR	((uint16)((uint16)0x0001u << Display_LCDPort__0__SHIFT))

#define Display_LCDPort_1			(Display_LCDPort__1__PC)
#define Display_LCDPort_1_INTR	((uint16)((uint16)0x0001u << Display_LCDPort__1__SHIFT))

#define Display_LCDPort_2			(Display_LCDPort__2__PC)
#define Display_LCDPort_2_INTR	((uint16)((uint16)0x0001u << Display_LCDPort__2__SHIFT))

#define Display_LCDPort_3			(Display_LCDPort__3__PC)
#define Display_LCDPort_3_INTR	((uint16)((uint16)0x0001u << Display_LCDPort__3__SHIFT))

#define Display_LCDPort_4			(Display_LCDPort__4__PC)
#define Display_LCDPort_4_INTR	((uint16)((uint16)0x0001u << Display_LCDPort__4__SHIFT))

#define Display_LCDPort_5			(Display_LCDPort__5__PC)
#define Display_LCDPort_5_INTR	((uint16)((uint16)0x0001u << Display_LCDPort__5__SHIFT))

#define Display_LCDPort_6			(Display_LCDPort__6__PC)
#define Display_LCDPort_6_INTR	((uint16)((uint16)0x0001u << Display_LCDPort__6__SHIFT))

#define Display_LCDPort_INTR_ALL	 ((uint16)(Display_LCDPort_0_INTR| Display_LCDPort_1_INTR| Display_LCDPort_2_INTR| Display_LCDPort_3_INTR| Display_LCDPort_4_INTR| Display_LCDPort_5_INTR| Display_LCDPort_6_INTR))

#endif /* End Pins Display_LCDPort_ALIASES_H */


/* [] END OF FILE */
