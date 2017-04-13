#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include"ILI9163C.h"
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


const unsigned short BACKGROUND = 0xF800; //define background color

void drawChar(unsigned short x, unsigned short y, char c, unsigned short color){
    if(((x + 5) > 128) || ((y+5) > 128)){ //make sure character wont overflow screen
        return;
    }
    int i = 0;
    int j = 0;
    for(i; i < 5; i++){
        j = 0;
        for(j; j < 8; j++){
            if((((ASCII[c - 0x20][i]) >> j) & 0x1) == 1){
                LCD_drawPixel((x+i), (y+j), color);
            }
        }
    }
}

void drawString(unsigned short x, unsigned short y, char *message, unsigned short color){
    int i = 0;
    unsigned short xinc = x;
    while(message[i]){ //while string isn't fully printed
        drawChar(xinc, y, message[i], color);
        i++;
        xinc = xinc + 5; //draw next character after current
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
//    
    SPI1_init(); //initialize SPI pins
    __builtin_enable_interrupts();
    LCD_init(); //initialize LCD
    LCD_clearScreen(BACKGROUND); //test red
    char message[10];
    sprintf(message, "hyraxe");
    drawString(10, 10, message, 0xFFFF); //test string
    while(1) {
	    // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
		  // remember the core timer runs at half the CPU speed
        ;
    }
}