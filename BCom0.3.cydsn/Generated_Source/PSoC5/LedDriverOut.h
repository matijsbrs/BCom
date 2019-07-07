/*******************************************************************************
* File Name: LedDriverOut.h  
* Version 2.20
*
* Description:
*  This file contains Pin function prototypes and register defines
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_LedDriverOut_H) /* Pins LedDriverOut_H */
#define CY_PINS_LedDriverOut_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "LedDriverOut_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 LedDriverOut__PORT == 15 && ((LedDriverOut__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    LedDriverOut_Write(uint8 value);
void    LedDriverOut_SetDriveMode(uint8 mode);
uint8   LedDriverOut_ReadDataReg(void);
uint8   LedDriverOut_Read(void);
void    LedDriverOut_SetInterruptMode(uint16 position, uint16 mode);
uint8   LedDriverOut_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the LedDriverOut_SetDriveMode() function.
     *  @{
     */
        #define LedDriverOut_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define LedDriverOut_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define LedDriverOut_DM_RES_UP          PIN_DM_RES_UP
        #define LedDriverOut_DM_RES_DWN         PIN_DM_RES_DWN
        #define LedDriverOut_DM_OD_LO           PIN_DM_OD_LO
        #define LedDriverOut_DM_OD_HI           PIN_DM_OD_HI
        #define LedDriverOut_DM_STRONG          PIN_DM_STRONG
        #define LedDriverOut_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define LedDriverOut_MASK               LedDriverOut__MASK
#define LedDriverOut_SHIFT              LedDriverOut__SHIFT
#define LedDriverOut_WIDTH              1u

/* Interrupt constants */
#if defined(LedDriverOut__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in LedDriverOut_SetInterruptMode() function.
     *  @{
     */
        #define LedDriverOut_INTR_NONE      (uint16)(0x0000u)
        #define LedDriverOut_INTR_RISING    (uint16)(0x0001u)
        #define LedDriverOut_INTR_FALLING   (uint16)(0x0002u)
        #define LedDriverOut_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define LedDriverOut_INTR_MASK      (0x01u) 
#endif /* (LedDriverOut__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define LedDriverOut_PS                     (* (reg8 *) LedDriverOut__PS)
/* Data Register */
#define LedDriverOut_DR                     (* (reg8 *) LedDriverOut__DR)
/* Port Number */
#define LedDriverOut_PRT_NUM                (* (reg8 *) LedDriverOut__PRT) 
/* Connect to Analog Globals */                                                  
#define LedDriverOut_AG                     (* (reg8 *) LedDriverOut__AG)                       
/* Analog MUX bux enable */
#define LedDriverOut_AMUX                   (* (reg8 *) LedDriverOut__AMUX) 
/* Bidirectional Enable */                                                        
#define LedDriverOut_BIE                    (* (reg8 *) LedDriverOut__BIE)
/* Bit-mask for Aliased Register Access */
#define LedDriverOut_BIT_MASK               (* (reg8 *) LedDriverOut__BIT_MASK)
/* Bypass Enable */
#define LedDriverOut_BYP                    (* (reg8 *) LedDriverOut__BYP)
/* Port wide control signals */                                                   
#define LedDriverOut_CTL                    (* (reg8 *) LedDriverOut__CTL)
/* Drive Modes */
#define LedDriverOut_DM0                    (* (reg8 *) LedDriverOut__DM0) 
#define LedDriverOut_DM1                    (* (reg8 *) LedDriverOut__DM1)
#define LedDriverOut_DM2                    (* (reg8 *) LedDriverOut__DM2) 
/* Input Buffer Disable Override */
#define LedDriverOut_INP_DIS                (* (reg8 *) LedDriverOut__INP_DIS)
/* LCD Common or Segment Drive */
#define LedDriverOut_LCD_COM_SEG            (* (reg8 *) LedDriverOut__LCD_COM_SEG)
/* Enable Segment LCD */
#define LedDriverOut_LCD_EN                 (* (reg8 *) LedDriverOut__LCD_EN)
/* Slew Rate Control */
#define LedDriverOut_SLW                    (* (reg8 *) LedDriverOut__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define LedDriverOut_PRTDSI__CAPS_SEL       (* (reg8 *) LedDriverOut__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define LedDriverOut_PRTDSI__DBL_SYNC_IN    (* (reg8 *) LedDriverOut__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define LedDriverOut_PRTDSI__OE_SEL0        (* (reg8 *) LedDriverOut__PRTDSI__OE_SEL0) 
#define LedDriverOut_PRTDSI__OE_SEL1        (* (reg8 *) LedDriverOut__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define LedDriverOut_PRTDSI__OUT_SEL0       (* (reg8 *) LedDriverOut__PRTDSI__OUT_SEL0) 
#define LedDriverOut_PRTDSI__OUT_SEL1       (* (reg8 *) LedDriverOut__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define LedDriverOut_PRTDSI__SYNC_OUT       (* (reg8 *) LedDriverOut__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(LedDriverOut__SIO_CFG)
    #define LedDriverOut_SIO_HYST_EN        (* (reg8 *) LedDriverOut__SIO_HYST_EN)
    #define LedDriverOut_SIO_REG_HIFREQ     (* (reg8 *) LedDriverOut__SIO_REG_HIFREQ)
    #define LedDriverOut_SIO_CFG            (* (reg8 *) LedDriverOut__SIO_CFG)
    #define LedDriverOut_SIO_DIFF           (* (reg8 *) LedDriverOut__SIO_DIFF)
#endif /* (LedDriverOut__SIO_CFG) */

/* Interrupt Registers */
#if defined(LedDriverOut__INTSTAT)
    #define LedDriverOut_INTSTAT            (* (reg8 *) LedDriverOut__INTSTAT)
    #define LedDriverOut_SNAP               (* (reg8 *) LedDriverOut__SNAP)
    
	#define LedDriverOut_0_INTTYPE_REG 		(* (reg8 *) LedDriverOut__0__INTTYPE)
#endif /* (LedDriverOut__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_LedDriverOut_H */


/* [] END OF FILE */
