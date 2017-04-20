/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
 */

APP_DATA appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
 */

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
 */


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

//Constants
const char ADDRESS = 0b1101011; //address of accelerometer
const unsigned short BACKGROUND = 0xF800; //define background color

void initAcc() {
    //Set IO for pins
    i2c_master_start(); //make the start bit
    i2c_master_send(((ADDRESS << 1) | 0)); //write
    i2c_master_send(0x10); //write to the CTRL1_XL register
    i2c_master_send(0b10000010); //set sample rate 1.66 kHz, 2g sensitivity, 100 Hz filter
    i2c_master_stop(); //make the stop bit
    i2c_master_start(); //make the start bit
    i2c_master_send(((ADDRESS << 1) | 0)); //write
    i2c_master_send(0x11); //write to the CTRL2_G register
    i2c_master_send(0b10001000); //set sample rate 1.66 kHz, 1000dps sensitivity
    i2c_master_stop(); //make the stop bit
}

unsigned char getWAI() {
    i2c_master_start(); //make the start bit
    i2c_master_send(((ADDRESS << 1) | 0)); //write
    i2c_master_send(0x0F); //write to the WHO_AM_I register
    i2c_master_restart(); //make the restart bit
    i2c_master_send(((ADDRESS << 1) | 1)); //read
    unsigned char r = i2c_master_recv(); //save the value returned
    i2c_master_ack(1); //make the ack so the slave knows we got it
    i2c_master_stop(); //make the stop bit
    return r;
}

void I2C_read_multiple(unsigned char *data) { //arguments stripped for this assignment
    i2c_master_start(); //make the start bit
    i2c_master_send(((ADDRESS << 1) | 0)); //write
    i2c_master_send(0x20); //write to the OUT_TEMP_L register
    i2c_master_restart(); // make the restart bit
    i2c_master_send(((ADDRESS << 1) | 1)); //read
    int i = 0;
    for (i; i < 13; i++) {
        data[i] = i2c_master_recv(); //save value returned
        i2c_master_ack(0); //keep reading
    }
    data[13] = i2c_master_recv(); //save value returned
    i2c_master_ack(1); //stop reading
    i2c_master_stop(); //make the stop bit
}

void drawAccBar(signed short length, int xy, unsigned short color) {
    signed short dist;
    dist = (((float) length) / 16384)*50;

    int i = 0;
    for (i; i < 129; i++) {
        if (((dist < 0) && ((i > (dist + 60)) && (i < 60))) || ((dist > 0) &&((i < (dist + 60)) && (i > 60)))) { //checking where to draw line
            if (xy == 0) {
                LCD_drawPixel(i, 60, color);
            } else {
                LCD_drawPixel(60, i, color);
            }
        } else {
            if (xy == 0) {
                LCD_drawPixel(i, 60, 0xF800);
            } else {
                LCD_drawPixel(60, i, 0xF800);
            }
        }
    }
}

void APP_Initialize(void) {
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;

    SPI1_init(); //initialize SPI pins
    LCD_init(); //initialize LCD
    LCD_clearScreen(BACKGROUND); //test red

    //Set up i2c
    ANSELBbits.ANSB2 = 0; //turn SCL2 and SDA2 pins to be not analog inputs
    ANSELBbits.ANSB3 = 0;
    i2c_master_setup();
    initAcc(); //set accelerometer registers

    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}

/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks(void) {

    /* Check the application's current state. */
    switch (appData.state) {
            /* Application's initial state. */
        case APP_STATE_INIT:
        {
            bool appInitialized = true;


            if (appInitialized) {

                appData.state = APP_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
            static unsigned char datain[14];
            static unsigned char message[100];
            static signed short dataP[7]; //processed data (temp, gX, gY, gZ, aX, aY, aZ)
            static int i;
            I2C_read_multiple(datain);
            i = 0;
            for (i; i < 7; i++) {
                dataP[i] = datain[2 * i + 1];
                dataP[i] = (dataP[i] << 8) | datain[2 * i];
                dataP[i] = dataP[i]; //divide by 2^8
            }
            //        sprintf(message, "%d     ", dataP[4]);
            //        drawString(50, 50, message, 0xFFFF); //draw aX for test
            drawAccBar(dataP[4], 0, 0xFFFF); //draw aX bar
            drawAccBar(dataP[5], 1, 0xFFFF); //draw aY bar
            _CP0_SET_COUNT(0);
            while (_CP0_GET_COUNT() < 4800000) { //5 hz wait
                ;
            }
            break;
        }

            /* TODO: implement your application state machine.*/


            /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}



/*******************************************************************************
 End of File
 */
