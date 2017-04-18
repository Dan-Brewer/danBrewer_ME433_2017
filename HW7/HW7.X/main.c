#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "i2c_master_noint.h" //i2c functions
#include"ILI9163C.h" //LED functions
#include<stdio.h>

// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // free up secondary osc pins (unsure here)
#pragma config FPBDIV = DIV_1 // divide CPU freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1048576 // slowest wdt (pretty sure this is slowest)
#pragma config WINDIS = OFF // no wdt window
#pragma config FWDTEN = OFF // wdt off by default
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the CPU clock to 48MHz
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz (external clock is 8Mhz)
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV (get 96))
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz (get 48))
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiply by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module

//Constants
const char ADDRESS = 0b1101011; //address of accelerometer
const unsigned short BACKGROUND = 0xF800; //define background color

void initAcc(){
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

unsigned char getWAI(){
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

void I2C_read_multiple(unsigned char *data){ //arguments stripped for this assignment
    i2c_master_start(); //make the start bit
    i2c_master_send(((ADDRESS << 1) | 0)); //write
    i2c_master_send(0x20); //write to the OUT_TEMP_L register
    i2c_master_restart(); // make the restart bit
    i2c_master_send(((ADDRESS << 1) | 1)); //read
    int i = 0;
    for(i; i < 13; i++){
        data[i] = i2c_master_recv(); //save value returned
        i2c_master_ack(0); //keep reading
    }
    data[13] = i2c_master_recv(); //save value returned
    i2c_master_ack(1); //stop reading
    i2c_master_stop(); //make the stop bit
}

void drawAccBar(signed short length, int xy, unsigned short color){
    signed short dist;
    dist = (((float)length)/16384)*50;
    
    int i = 0;
    for(i; i < 129; i++){
        if(((dist < 0) && ((i > (dist + 60)) && (i < 60))) || ((dist > 0) &&((i < (dist + 60)) && (i > 60)))){ //checking where to draw line
            if(xy == 0){
                LCD_drawPixel(i, 60, color);
            }
            else{
                LCD_drawPixel(60, i, color);
            }
        }
        else{
            if(xy == 0){
                LCD_drawPixel(i, 60, 0xF800);
            }
            else{
                LCD_drawPixel(60, i, 0xF800);
            }
        }
    } 
}


int main() {
    
    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
//    TRISAbits.TRISA4 = 0; //Set A4 to output for LED
//    TRISBbits.TRISB4 = 1; //Set B4 to input for user button 
//    LATAbits.LATA4 = 1; //set A4 high to turn LED on at start
    
    SPI1_init(); //initialize SPI pins
    LCD_init(); //initialize LCD
    LCD_clearScreen(BACKGROUND); //test red
    
    //Set up i2c
    ANSELBbits.ANSB2 = 0; //turn SCL2 and SDA2 pins to be not analog inputs
    ANSELBbits.ANSB3 = 0; 
    i2c_master_setup();
    initAcc(); //set accelerometer registers
   
    __builtin_enable_interrupts();
        
    //Test who am i 
//    unsigned char r = 0;
//    r = getWAI();
//    if(r == 0b01101001){
//        drawChar(50, 50, 'Y', 0xFFFF);
//    }
//    
    
    unsigned char datain[14];
    unsigned char message[100];
    signed short dataP[7]; //processed data (temp, gX, gY, gZ, aX, aY, aZ)
    int i;
    while(1) {
	    I2C_read_multiple(datain);
        i = 0;
        for(i; i < 7; i++){
            dataP[i] = datain[2*i + 1];
            dataP[i] = (dataP[i] << 8) | datain[2*i];
            dataP[i] = dataP[i]; //divide by 2^8
        }
//        sprintf(message, "%d     ", dataP[4]);
//        drawString(50, 50, message, 0xFFFF); //draw aX for test
        drawAccBar(dataP[4], 0, 0xFFFF); //draw aX bar
        drawAccBar(dataP[5], 1, 0xFFFF); //draw aY bar
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < 4800000){ //5 hz wait
            ;
        }
    }
}