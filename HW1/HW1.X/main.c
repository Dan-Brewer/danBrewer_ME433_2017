#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro

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

    // do your TRIS and LAT commands here MODIFIED FOR MOTOR TEST
    TRISBbits.TRISB2 = 0; //Set B2 to output for Motor 2
    TRISBbits.TRISB3 = 0; //Set B3 to output for Motor 2
    TRISBbits.TRISB14 = 0; //Set A4 to output for Motor 1
    TRISBbits.TRISB15 = 0; //Set B4 to input for Motor 1
    
    
    LATBbits.LATB14 = 1; //set A4 high to turn LED on at start
    LATBbits.LATB15 = 0; //get motor 1 running
    
    LATBbits.LATB2 = 1; //get motor 2 running
    LATBbits.LATB3 = 1; //get motor 2 running
    
    TRISAbits.TRISA4 = 0; //led
    TRISBbits.TRISB4 = 1; // user button
    
    LATAbits.LATA4 = 1; //led on

    __builtin_enable_interrupts();

    while(1) {
	    // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
		  // remember the core timer runs at half the CPU speed
        _CP0_SET_COUNT(0); //set count to zero
        if(LATAbits.LATA4 == 1){ //toggle LED
            LATAbits.LATA4 = 0;
        }
        else{
            LATAbits.LATA4 = 1;
        }
        while(_CP0_GET_COUNT() < 12000){ //wait 0.5 ms
            ; //do nothing
        }
        while(PORTBbits.RB4 == 0){ //while user button is pressed
            LATAbits.LATA4 = 0; //turn off LED
        }
    }
}
