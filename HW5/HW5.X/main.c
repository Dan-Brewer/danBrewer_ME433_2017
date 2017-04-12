#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "i2c_master_noint.h" //i2c functions

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
const char ADDRESS = 0x20; //address of i2c2 I/O expander chip

void initExpander(){
    //Set IO for pins
    i2c_master_start(); //make the start bit
    i2c_master_send(((ADDRESS << 1) | 0)); //write
    i2c_master_send(0x00); //write to the IODIR pin
    i2c_master_send(0xF8); //set G0-3 as outputs, G4-7 as inputs
    i2c_master_stop(); //make the stop bit
}

void setExpander(char pin, char level){
    i2c_master_start(); //make the start bit
    i2c_master_send(((ADDRESS << 1) | 0));
    i2c_master_send(0x09); //write to the GPIO register
    i2c_master_send(level << pin); //write the desired level to the desired pin
    i2c_master_stop(); //make the stop bit
}

unsigned char getExpander(){
    i2c_master_start(); //make the start bit
    i2c_master_send(((ADDRESS << 1) | 0)); //write
    i2c_master_send(0x09); //write to the GPIO register
    i2c_master_restart(); //make the restart bit
    i2c_master_send(((ADDRESS << 1) | 1)); //read
    unsigned char r = i2c_master_recv(); //save the value returned
    i2c_master_ack(1); //make the ack so the slave knows we got it
    i2c_master_stop(); //make the stop bit
    return r;
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
    
    //Set up i2c
    ANSELBbits.ANSB2 = 0; //turn SCL2 and SDA2 pins to be not analog inputs
    ANSELBbits.ANSB3 = 0; 
    i2c_master_setup();
    initExpander(); //set Expander IO pins
    
    __builtin_enable_interrupts();
    
    unsigned char r; //read byte from expander
    setExpander(0, 1); //start LED pin high
    while(1) {
	    r = getExpander();
        if(((r >> 7) & 0x01) == 0){
            setExpander(0, 0); //set LED pin low
        }
        else{
            setExpander(0, 1); //set LED pin high
        }
    }
}