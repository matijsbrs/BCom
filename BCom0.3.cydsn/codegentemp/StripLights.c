/* =========================================================
 *
 * Strip Light Controller
 * By Mark Hastings
 *
 * 05/27/2013  v1.0  Mark Hastings   Initial working version
 * 09/04/2013  v1.3  Mark Hastings   Add more features and color lookup.
 * 12/02/2013  v1.6  Mark Hastings   Added a couple more functions.
 *
 * This file contains the functions required to control each
 * lighting control channel.
 *
 * =========================================================
*/

#include <project.h>
#include "cytypes.h"
#include "stdlib.h"
#include "cyfitter.h"
#include "StripLights.h"
#include "StripLights_fonts.h"

uint8  StripLights_initvar = 0;

#if(StripLights_MEMORY_TYPE == StripLights_MEMORY_RGB)
uint32  StripLights_ledArray[StripLights_ARRAY_ROWS][StripLights_ARRAY_COLS];
#else
uint8   StripLights_ledArray[StripLights_ARRAY_ROWS][StripLights_ARRAY_COLS];
#endif

uint32  StripLights_ledIndex = 0;  
uint32  StripLights_row = 0;
uint32  StripLights_refreshComplete;

uint32  StripLights_DimMask;
uint32  StripLights_DimShift;

uint32 StripLights_Red_Shift = 0;
uint32 StripLights_Green_Shift = 8;
uint32 StripLights_Blue_Shift = 16;

#if(StripLights_CHIP == StripLights_CHIP_WS2812)
const uint32 StripLights_CLUT[ ] = {
//xxBBRRGG (WS2812)
//     (24)   [ Index = 0 ]
0x0000FFFF,  // 0 Yellow
0x0000CCFF,
0x000099FF,
0x000033FF,
0x000000FF,  // 5  Green
0x006600B3,
0x00990099, 
0x00B30066, 
0x00CC0033,  // 9 Blue
0x00B31919, 
0x00993300, 
0x00994000, 
0x00996600, 
0x00999900, 
0x0099CC00, 
0x0066E600, 
0x0000FF00, 
0x0000FF33, 
0x0000FF66, 
0x0000FF80, 
0x0000FF99,  // 20 Orange
0x0000FFB2, 
0x0000FFCC, 
0x0000FFE5,
// #24

//xxBBRRGG  (64)  [ Index = 24 ]
0x00000000, 0x00550000, 0x00AA0000, 0x00FF0000,  // 0, Black,  LtBlue, MBlue, Blue
0x00000055, 0x00550055, 0x00AA0055, 0x00FF0055,  // 4, LtGreen
0x000000AA, 0x005500AA, 0x00AA00AA, 0x00FF00AA,  // 8, MGreen
0x000000FF, 0x005500FF, 0x00AA00FF, 0x00FF00FF,  // 12 Green

0x00005500, 0x00555500, 0x00AA5500, 0x00FF5500,  // 16, LtRed
0x00005555, 0x00555555, 0x00AA5555, 0x00FF5555,  // 20, LtYellow
0x000055AA, 0x005555AA, 0x00AA55AA, 0x00FF55AA,  // 24, 
0x000055FF, 0x005555FF, 0x00AA55FF, 0x00FF55FF,  // 28,

0x0000AA00, 0x0055AA00, 0x00AAAA00, 0x00FFAA00,  // 32, MRed
0x0000AA55, 0x0055AA55, 0x00AAAA55, 0x00FFAA55,  // 36, 
0x0000AAAA, 0x0055AAAA, 0x00AAAAAA, 0x00FFAAAA,  // 55, 
0x0000AAFF, 0x0055AAFF, 0x00AAAAFF, 0x00FFAAFF,  // 44, 

0x0000FF00, 0x0055FF00, 0x00AAFF00, 0x00FFFF00,  // 48, Red, ??, ??, Magenta
0x0000FF55, 0x0055FF55, 0x00AAFF55, 0x00FFFF55,  // 52,
0x0000FFAA, 0x0055FFAA, 0x00AAFFAA, 0x00FFFFAA,  // 56,
0x0000FFFF, 0x0055FFFF, 0x00AAFFFF, 0x00FFFFFF,  // 60, Yellow,??, ??, White

// Misc ( 16 )  [ Index = 88 ]
0x000080FF,  // SPRING_GREEN
0x008000FF,  // TURQUOSE
0x00FF00FF,  // CYAN
0x00FF0080,  // OCEAN
0x00FF8000,  // VIOLET
0x0080FF00,  // RASPBERRY
0x000000FF,  // GREEN
0x00202020,  // DIM WHITE
0x00200000,  // DIM BLUE
0x10000000,  // INVISIBLE
0x00000000,  // Spare
0x00000000,  // Spare
0x00000000,  // Spare
0x00000000,  // Spare
0x00000000,  // Spare
0x00000000,  // Spare

// Temperture Color Blue to Red (16) [ Index = 104 ]
0x00FF0000, 0x00F01000, 0x00E02000, 0x00D03000,
0x00C04000, 0x00B05000, 0x00A06000, 0x00907000,
0x00808000, 0x00709000, 0x0060A000, 0x0050B000,
0x0040C000, 0x0030D000, 0x0020E000, 0x0000FF00
};
#else  //xxBBGGRR (WS2811)
const uint32 StripLights_CLUT[ ] = {
//     (24)   [ Index = 0 ]
0x0000FFFF,  // 0 Yellow
0x0000FFCC,
0x0000FF99,
0x0000FF33,
0x0000FF00,  // 5  Green
0x0066B300,
0x00999900, 
0x00B36600, 
0x00CC3300,  // 9 Blue
0x00B31919, 
0x00990033, 
0x00990040, 
0x00990066, 
0x00990099, 
0x009900CC, 
0x006600E6, 
0x000000FF, 
0x000033FF, 
0x000066FF, 
0x000080FF, 
0x000099FF,  // 20 Orange
0x0000B2FF, 
0x0000CCFF, 
0x0000E5FF,
// #24

//xxBBRRGG  (64)  [ Index = 24 ]
0x00000000, 0x00550000, 0x00AA0000, 0x00FF0000,  // 0, Black,  LtBlue, MBlue, Blue
0x00005500, 0x00555500, 0x00AA5500, 0x00FF5500,  // 4, LtGreen
0x0000AA00, 0x0055AA00, 0x00AAAA00, 0x00FFAA00,  // 8, MGreen
0x0000FF00, 0x0055FF00, 0x00AAFF00, 0x00FFFF00,  // 12 Green

0x00000055, 0x00550055, 0x00AA0055, 0x00FF0055,  // 16, LtRed
0x00005555, 0x00555555, 0x00AA5555, 0x00FF5555,  // 20, LtYellow
0x0000AA55, 0x0055AA55, 0x00AAAA55, 0x00FFAA55,  // 24, 
0x0000FF55, 0x0055FF55, 0x00AAFF55, 0x00FFFF55,  // 28,

0x000000AA, 0x005500AA, 0x00AA00AA, 0x00FF00AA,  // 32, MRed
0x000055AA, 0x005555AA, 0x00AA55AA, 0x00FF55AA,  // 36, 
0x0000AAAA, 0x0055AAAA, 0x00AAAAAA, 0x00FFAAAA,  // 55, 
0x0000FFAA, 0x0055FFAA, 0x00AAFFAA, 0x00FFFFAA,  // 44, 

0x000000FF, 0x005500FF, 0x00AA00FF, 0x00F00FFF,  // 48, Red, ??, ??, Magenta
0x000055FF, 0x005555FF, 0x00AA55FF, 0x00FF55FF,  // 52,
0x0000AAFF, 0x0055AAFF, 0x00AAAAFF, 0x00FFAAFF,  // 56,
0x0000FFFF, 0x0055FFFF, 0x00AAFFFF, 0x00FFFFFF,  // 60, Yellow,??, ??, White

// Misc ( 16 )  [ Index = 88 ]
0x0000FF80,  // SPRING_GREEN
0x0080FF00,  // TURQUOSE
0x00FFFF00,  // CYAN
0x00FF8000,  // OCEAN
0x00FF0080,  // VIOLET
0x008000FF,  // RASPBERRY
0x0000FF00,  // GREEN
0x00202020,  // DIM WHITE
0x00200000,  // DIM BLUE
0x10000000,  // INVISIBLE
0x00000000,  // Spare
0x00000000,  // Spare
0x00000000,  // Spare
0x00000000,  // Spare
0x00000000,  // Spare
0x00000000,  // Spare

// Temperture Color Blue to Red (16) [ Index = 104 ]
0x00FF0000, 0x00F00010, 0x00E00020, 0x00D00030,
0x00C00040, 0x00B00050, 0x00A00060, 0x00900070,
0x00800080, 0x00700090, 0x006000A0, 0x005000B0,
0x004000C0, 0x003000D0, 0x002000E0, 0x000000FF
};
#endif

// Gamma Correction Table NeoPixel / WS2812
const uint8_t CYCODE gamma[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };




/*******************************************************************************
* Function Name: StripLights_Start
********************************************************************************
* Summary:
*  Initialize the hardware. 
*
* Parameters:  
*  void  
*
* Return: 
*  void
*
*******************************************************************************/
void StripLights_Start()
{
    extern uint8 StripLights_initvar;
    extern uint32 StripLights_refreshComplete;

    StripLights_UseGammaCorrection = 1;
    StripLights_ACTL0_REG = StripLights_DISABLE_FIFO;
    
    StripLights_Period   = StripLights_PERIOD-1;
    StripLights_Compare0 = StripLights_DATA_ZERO;
    StripLights_Compare1 = StripLights_DATA_ONE;
    
    StripLights_CONTROL = StripLights_ENABLE;
    StripLights_MemClear(StripLights_OFF);
    StripLights_CONTROL = StripLights_DISABLE;
    
    StripLights_SetFont( StripLights_FONT_5X7);
    
    /* Set no dimming, full brightness */
    StripLights_Dim(0); 

    
    if(StripLights_initvar == 0)
    {

        StripLights_initvar = 1;
 
         /* Start and set interrupt vector */

#if(StripLights_TRANSFER == StripLights_TRANSFER_ISR)
       {
           StripLights_cisr_StartEx(StripLights_CISR);
		   StripLights_fisr_StartEx(StripLights_FISR);
       }
#endif       
       if(StripLights_TRANSFER == StripLights_TRANSFER_FIRMWARE)
       {
           StripLights_CONTROL = StripLights_ENABLE;    
       }
    }
    StripLights_refreshComplete = 1;
}


/*******************************************************************************
* Function Name: StripLights_Trigger
********************************************************************************
* Summary:
*   Update the LEDs with graphics memory.
*
* Parameters:  
*  void  
*
* Return: 
*  void
*
*******************************************************************************/
void StripLights_Trigger(uint32 rst)
{
    uint32 color;
    
    if(rst) 
    {
        StripLights_row = 0;  /* Reset the row */
        StripLights_Channel = 0;
    }
    
    #if(StripLights_MEMORY_TYPE == StripLights_MEMORY_RGB)
       color = StripLights_ledArray[StripLights_row][0];
    #else  /* Else use lookup table */
       color = StripLights_CLUT[ (StripLights_ledArray[StripLights_row][0]) ];
    #endif
    
    StripLights_WriteColor(color);
    
    StripLights_ledIndex = 1;
 //   StripLights_CONTROL = StripLights_ENABLE | StripLights_FIFO_IRQ_EN; 
	
	StripLights_CONTROL = StripLights_ENABLE | StripLights_XFRCMPT_IRQ_EN | StripLights_FIFO_IRQ_EN; 
    StripLights_refreshComplete = 0;
}

/*******************************************************************************
* Function Name: StripLights_Ready
********************************************************************************
* Summary:
*  Checks to see if transfer is complete.
*
* Parameters:  
*  none  
*
* Return: 
*  Zero if not complete, non-zero if transfer complete.
*
*******************************************************************************/
uint32 StripLights_Ready(void)
{
    if(StripLights_refreshComplete )
    {
        StripLights_CONTROL = StripLights_DISABLE;
        return((uint32)1);
    }
    else
    {
         return((uint32)0);
    }
}


/*******************************************************************************
* Function Name: StripLights_Stop
********************************************************************************
* Summary:
*  Stop all transfers.
*
* Parameters:  
*  None 
*
* Return: 
*  void
*
*******************************************************************************/
void StripLights_Stop()
{

    StripLights_CONTROL = StripLights_DISABLE;
}


/*******************************************************************************
* Function Name: StripLights_ColorInc
********************************************************************************
* Summary:
*  Increment color throught the color lookup table.
*
* Parameters:  
*  uint32 incValue: Increment through color table by incValue. 
*
* Return: Color at next location.
*  
*
*******************************************************************************/
uint32 StripLights_ColorInc(uint32 incValue)
{
    uint32 color;
    extern const uint32 StripLights_CLUT[];
    static uint32 colorIndex = 0;
    
    colorIndex += incValue;
    colorIndex = colorIndex % StripLights_COLOR_WHEEL_SIZE;

    #if(StripLights_MEMORY_TYPE == StripLights_MEMORY_RGB)
       color = StripLights_CLUT[ colorIndex ];
    #else
        color = colorIndex;
    #endif


    return(color);
}

void   StripLights_SetBitShifts(uint32 Red, uint32 Green, uint32 Blue) {
    StripLights_Red_Shift = Red;
    StripLights_Green_Shift = Green;
    StripLights_Blue_Shift = Blue;
}


/*****************************************************************************
* Function Name: StripLights_FISR
******************************************************************************
*
* Summary:
*  Interrupt service handler for data transfer for each LED 
*
* Parameters:  
*  void
*
* Return: 
*  void 
*
* Reentrant: 
*  No
*
*****************************************************************************/
CY_ISR( StripLights_FISR)
{
    extern uint32  StripLights_DimMask;
    extern uint32  StripLights_DimShift;
    uint32 static color;

    if(StripLights_ledIndex < StripLights_ARRAY_COLS)
    {
        #if(StripLights_MEMORY_TYPE == StripLights_MEMORY_RGB)
            color = StripLights_ledArray[StripLights_row][StripLights_ledIndex++];
        #else  /* Else use lookup table */
            color = StripLights_CLUT[ (StripLights_ledArray[StripLights_row][StripLights_ledIndex++]) ];
        #endif

        StripLights_WriteColor(color);
    }
    else 
    {
         StripLights_CONTROL = StripLights_ENABLE | StripLights_XFRCMPT_IRQ_EN; 
    }

}

/*****************************************************************************
* Function Name: StripLights_CISR
******************************************************************************
*
* Summary:
*  Interrupt service handler after each row is complete.
*
* Parameters:  
*  void
*
* Return: 
*  void 
*
* Reentrant: 
*  No
*
*****************************************************************************/
CY_ISR( StripLights_CISR)
{
    extern uint32  StripLights_DimMask;
    extern uint32  StripLights_DimShift;
    uint32 static color;
    extern uint32 StripLights_refreshComplete;

	StripLights_CONTROL = StripLights_ENABLE |StripLights_NEXT_ROW;
    StripLights_row++;
    if( StripLights_row < StripLights_ARRAY_ROWS)  /* More Rows to do  */
    {
        StripLights_Channel = StripLights_row;  

		#if(StripLights_MEMORY_TYPE == StripLights_MEMORY_RGB)
             color = StripLights_ledArray[StripLights_row][0];
        #else  /* Else use lookup table */
             color = StripLights_CLUT[ (StripLights_ledArray[StripLights_row][0]) ];
        #endif

        StripLights_WriteColor(color);

        StripLights_ledIndex = 1;
		StripLights_CONTROL = StripLights_ENABLE | StripLights_FIFO_IRQ_EN; 
		
    }
    else
    {
        StripLights_refreshComplete = 1u;
        if (StripLights_Done != NULL ) {
            CyDelay(1);
            StripLights_Done();
        }
    }
  
}


/*******************************************************************************
* Function Name: StripLights_DisplayClear
********************************************************************************
* Summary:
*   Clear memory with a given value and update the display.
*
* Parameters:  
*  uint32 color: Color to clear display. 
*
* Return: 
*  void
*
*******************************************************************************/
void StripLights_DisplayClear(uint32 color)
{   
    StripLights_MemClear(color);
    StripLights_Trigger(1);
}

/*******************************************************************************
* Function Name: StripLights_MemClear
********************************************************************************
* Summary:
*   Clear LED memory with given color, but do not update display.
*
* Parameters:  
*  uint32 color: Color to clear display.  
*
* Return: 
*  void
*
*******************************************************************************/
void StripLights_MemClear(uint32 color)
{
    uint32  row, col;
    
    for(row=0; row < StripLights_ARRAY_ROWS; row++)
    {
        for(col=0; col < StripLights_ARRAY_COLS; col++)
        {
            StripLights_ledArray[row][col] = color;
            #if(StripLights_MEMORY_TYPE == StripLights_MEMORY_RGB)
                StripLights_ledArray[row][col] = color;
            #else  /* Else use lookup table */
                 StripLights_ledArray[row][col] = (uint8)color;
            #endif
        }
    }
}



/*******************************************************************************
* Function Name: StripLights_WriteColor
********************************************************************************
* Summary:
*   Write given color directly to output register.
*
* Parameters:  
*  uint32 color: Color to write to display. 
*
* Return: 
*  void
*
*******************************************************************************/
void StripLights_WriteColor(uint32 color)
{
    while( (StripLights_STATUS & StripLights_FIFO_EMPTY) == 0); 

    
    
    #if(StripLights_UseGammaCorrection == 0 ) 
    
    StripLights_DATA = (uint8)(color & 0x000000FF);  // Write Red
    color = color >> 8;
    StripLights_DATA = (uint8)(color & 0x000000FF);  // Write green
    color = color >> 8;
    StripLights_DATA = (uint8)(color & 0x000000FF);  // Write green
    color = color >> 8;
    StripLights_DATA = (uint8)(color & 0x000000FF);  // Write green
    
    #else 
        
    StripLights_DATA = gamma[(uint8)(color & 0x000000FF)];  // Write Green
    color = color >> 8;
    StripLights_DATA = gamma[(uint8)(color & 0x000000FF)];  // Write Red
    color = color >> 8;
    StripLights_DATA = gamma[(uint8)(color & 0x000000FF)];  // Write Blue
    color = color >> 8;
    StripLights_DATA = (uint8)(color & 0x000000FF);  // Write green
    
    #endif 
}




void StripLights_RGBtoHSV( float r, float g, float b, float *h, float *s, float *v )
{
	float min, max, delta;
	min = MIN( r, g, b );
	max = MAX( r, g, b );
	*v = max;				// v
	delta = max - min;
	if( max != 0 )
		*s = delta / max;		// s
	else {
		// r = g = b = 0		// s = 0, v is undefined
		*s = 0;
		*h = -1;
		return;
	}
	if( r == max )
		*h = ( g - b ) / delta;		// between yellow & magenta
	else if( g == max )
		*h = 2 + ( b - r ) / delta;	// between cyan & yellow
	else
		*h = 4 + ( r - g ) / delta;	// between magenta & cyan
	*h *= 60;				// degrees
	if( *h < 0 )
		*h += 360;
}

/*!
	@fn void HSVtoRGB( float *r, float *g, float *b, float h, float s, float v )
	@brief convert HSV to RGB
*/
void StripLights_HSVtoRGB( float *r, float *g, float *b, float h, float s, float v )
{
	int i;
	float f, p, q, t;
	if( s == 0 ) {
		// achromatic (grey)
		*r = *g = *b = v;
		return;
	}
	h /= 60;			// sector 0 to 5
	i = (int) h;
	f = h - i;			// factorial part of h
	p = v * ( 1 - s );
	q = v * ( 1 - s * f );
	t = v * ( 1 - s * ( 1 - f ) );
	switch( i ) {
		case 0:
			*r = v;
			*g = t;
			*b = p;
			break;
		case 1:
			*r = q;
			*g = v;
			*b = p;
			break;
		case 2:
			*r = p;
			*g = v;
			*b = t;
			break;
		case 3:
			*r = p;
			*g = q;
			*b = v;
			break;
		case 4:
			*r = t;
			*g = p;
			*b = v;
			break;
		default:		// case 5:
			*r = v;
			*g = p;
			*b = q;
			break;
	}
}

void StripLights_HSV_Pixel(int32 x, int32 y, float h, float s, float v) {
    float R,G,B;
    StripLights_HSVtoRGB( &R, &G, &B,  h,  s,  v );    
    //uint32 colour = ((int) (R * 255)) | (((int) (G * 255))<<8) | (((int) (B * 255)) << 16 );
    uint32 colour = ((int) (R * 255)<<StripLights_Red_Shift) | (((int) (B * 255))<<StripLights_Blue_Shift) | (((int) (G * 255))<<StripLights_Green_Shift );
    StripLights_Pixel(x,y,colour);
}

/*******************************************************************************
* Function Name: StripLights_Pixel
********************************************************************************
*
* Summary:
*  Draw Pixel  
*
* Parameters:  
*  x,y:    Location to draw the pixel
*  color:  Color of the pixel
*
* Return: 
*  None 
*******************************************************************************/
void StripLights_Pixel(int32 x, int32 y, uint32 color)
{

	if((x >= StripLights_MIN_X) && (y >= StripLights_MIN_Y) && (x <= StripLights_MAX_X) && (y <= StripLights_MAX_Y))
    {

    #if(StripLights_MEMORY_TYPE == StripLights_MEMORY_RGB)
       StripLights_ledArray[y][x] = color;
    #else  /* Else use lookup table */
       StripLights_ledArray[y][x] = (uint8)color;
    #endif
    }
  
}

/*******************************************************************************
* Function Name: StripLights_GetPixel
********************************************************************************
*
* Summary:
*  Get Pixel Color  
*
* Parameters:  
*  x,y:    Location to get pixel color
*
* Return: 
*  None 
*******************************************************************************/
uint32 StripLights_GetPixel(int32 x, int32 y)
{
    uint32 color;
    if((x>=0) && (y>=0) && (x < StripLights_ARRAY_COLS) && (y < StripLights_ARRAY_ROWS))
    {
    #if(StripLights_MEMORY_TYPE == StripLights_MEMORY_RGB)
       color = StripLights_ledArray[y][x];
    #else  /* Else use lookup table */
       color = (uint32)StripLights_ledArray[y][x];
    #endif
    }
    return(color);
}

/*******************************************************************************
* Function Name: StripLights_DrawCircle
********************************************************************************
*
* Summary:
*  Draw a circle on the display given a start point and radius.  
*
*  This code uses Bresenham's Circle Algorithm. 
*
* Parameters:  
*  x0, y0: Center of circle
*  radius: Radius of circle
*  color:  Color of circle
*
* Return: 
*  None 
*  
*******************************************************************************/
void StripLights_DrawCircle (int32 x0, int32 y0, int32 radius, uint32 color)
{
	int32 f = 1 - radius;
	int32 ddF_x = 0;
	int32 ddF_y = -2 * radius;
	int32 x = 0;
	int32 y = radius;

	StripLights_Pixel(x0, y0 + radius, color);
	StripLights_Pixel(x0, y0 - radius, color);
	StripLights_Pixel( x0 + radius, y0, color);
	StripLights_Pixel( x0 - radius, y0, color);

	while(x < y)
	{
		if(f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;

		StripLights_Pixel(x0 + x, y0 + y, color);
		StripLights_Pixel(x0 - x, y0 + y, color);
		StripLights_Pixel( x0 + x, y0 - y, color);
		StripLights_Pixel( x0 - x, y0 - y, color);
		StripLights_Pixel( x0 + y, y0 + x, color);
		StripLights_Pixel( x0 - y, y0 + x, color);
		StripLights_Pixel( x0 + y, y0 - x, color);
		StripLights_Pixel( x0 - y, y0 - x, color);
	}
}


/*******************************************************************************
* Function Name: StripLights_DrawLine
********************************************************************************
*
* Summary:
*  Draw a line on the display.  
*
* Parameters:  
*  x0, y0:  The beginning endpoint
*  x1, y1:  The end endpoint.
*  color:   Color of the line.
*
* Return: 
*  None 
*  
*******************************************************************************/
void StripLights_DrawLine(int32 x0, int32 y0, int32 x1, int32 y1, uint32 color)
{
	int32 dy = y1 - y0; /* Difference between y0 and y1 */
	int32 dx = x1 - x0; /* Difference between x0 and x1 */
	int32 stepx, stepy;

	if (dy < 0)
	{
		dy = -dy;
		stepy = -1;
	}
	else
	{
		stepy = 1;
	}

	if (dx < 0)
	{
		dx = -dx;
		stepx = -1;
	}
	else
	{
		stepx = 1;
	}

	dy <<= 1; /* dy is now 2*dy  */
	dx <<= 1; /* dx is now 2*dx  */
	StripLights_Pixel(x0, y0, color);

	if (dx > dy) 
	{
		int fraction = dy - (dx >> 1);
		while (x0 != x1)
		{
			if (fraction >= 0)
			{
				y0 += stepy;
				fraction -= dx;
			}
			x0 += stepx;
			fraction += dy;
			StripLights_Pixel(x0, y0, color);
		}
	}
	else
	{
		int fraction = dx - (dy >> 1);
		while (y0 != y1)
		{
			if (fraction >= 0)
			{
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;
			StripLights_Pixel( x0, y0, color);
		}
	}
}

/*******************************************************************************
* Function Name: StripLights_DrawRect
********************************************************************************
*
* Summary:
*  Draw a rectangle, filled or not.  
*
* Parameters:  
*  x0, y0:  The upper lefthand corner.
*  x1, y1:  The lower right corner.
*  fill:    Non-Zero if retangle is to be filled.
*  color:   Color for rectangle, border and fill.
*
* Return: 
*  None 
*  
*******************************************************************************/
void StripLights_DrawRect(int32 x0, int32 y0, int32 x1, int32 y1, int32 fill, uint32 color)
{	
     int xDiff;
	/* Check if the rectangle is to be filled    */
	if (fill != 0)
	{	
        /* Find the difference between the x vars */
		if(x0 > x1)
		{
			xDiff = x0 - x1; 
		}
		else
		{
			xDiff = x1 - x0;
		}
	
	    /* Fill it with lines  */
		while(xDiff >= 0)
		{
			StripLights_DrawLine(x0, y0, x0, y1, color);
		
			if(x0 > x1)
				x0--;
			else
				x0++;
		
			xDiff--;
		}

	}
	else 
	{
		/* Draw the four sides of the rectangle */
		StripLights_DrawLine(x0, y0, x1, y0, color);
		StripLights_DrawLine(x0, y1, x1, y1, color);
		StripLights_DrawLine(x0, y0, x0, y1, color);
		StripLights_DrawLine(x1, y0, x1, y1, color);
	}
}

/*******************************************************************************
* Function Name: StripLights_Dim
********************************************************************************
*
* Summary:
*  Dim all output by a specific level (0,1,2,3,4)  
*
* Parameters:  
*  dimLevel:  Dim level 1 to 4, 0 => No dimming.
*
* Return: 
*  None 
*
@14092015^MB + dim mask 0..7 0:Min Dim 7:Max Dim

*******************************************************************************/
void StripLights_Dim(uint32 dimLevel) 
{
extern uint32  StripLights_DimMask;
extern uint32  StripLights_DimShift;

    switch(dimLevel)
    {           
        case 1:
           StripLights_DimMask = 0x007F7F7F;
           StripLights_DimShift = 1;
           break;
           
        case 2:
           StripLights_DimMask = 0x003F3F3F;
           StripLights_DimShift = 2;
           break;
           
        case 3:
           StripLights_DimMask = 0x001F1F1F;
           StripLights_DimShift = 3;
           break;
       
        case 4:
           StripLights_DimMask = 0x000F0F0F;
           StripLights_DimShift = 4;
           break;
       
        case 5:
           StripLights_DimMask = 0x00070707;
           StripLights_DimShift = 5;
           break;
       
        case 6:
           StripLights_DimMask = 0x00030303;
           StripLights_DimShift = 6;
           break;
        
        case 7:
           StripLights_DimMask = 0x00010101;
           StripLights_DimShift = 7;
           break;
        
        
       default:
           StripLights_DimMask = 0x00FFFFFF;
           StripLights_DimShift = 0;
           break;
        
    }
}

void StripLights_bplot( int32 x, int32 y, uint8 * bitMap, int32 update)
{
    int32 dx, dy;
    int32 aindex = 0;
    int32 maxX, maxY;

    maxX = x + (int32)bitMap[aindex++];
    maxY = y  + (int32)bitMap[aindex++];

	for(dy = y; dy < maxY; dy++)
    {
		for(dx = x; dx < maxX; dx++)
        {
            StripLights_Pixel(dx, dy, bitMap[aindex++]);
        }
    }
	if(update) StripLights_Trigger(1);
}

/* [] END OF FILE */
