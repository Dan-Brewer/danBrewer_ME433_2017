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

    // do your TRIS and LAT commands here
//    TRISAbits.TRISA4 = 0; //Set A4 to output for LED
//    TRISBbits.TRISB4 = 1; //Set B4 to input for user button 
//    LATAbits.LATA4 = 1; //set A4 high to turn LED on at start
//    
    //Assign functions to SPI pins
    RPA0Rbits.RPA0R = 0x3; //Assign A0 to slave select (ss1)
    RPA1Rbits.RPA1R = 0x3; //Assign A1 to SDO1
    SDI1Rbits.SDI1R = 0x2; //Assign B1 to SDI1
    
    __builtin_enable_interrupts();

    
    //SPI Initialization function
    void SP1init(){
    //Set slave select to output 1
    TRISAbits.TRISA0 = 0; //set to output
    LATAbits.LATA0 = 1; //set to high
    
    //Initialize SPI1
    SPI1CON = 0;              // turn off the spi module and reset it
    SPI1BUF;                  // clear the rx buffer by reading from it
    SPI1BRG = 0x1;            // baud rate to 12 MHz [SPI4BRG = (48000000/(2*desired))-1]
    SPI1STATbits.SPIROV = 0;  // clear the overflow bit
    SPI1CONbits.CKE = 1;      // data changes when clock goes from hi to lo (since CKP is 0)
    SPI1CONbits.MSTEN = 1;    // master operation
    SPI1CONbits.ON = 1;       // turn on  1
    }
    
    
    //SPI Communication functions
    char *packageData(char AB, char data[]){
//        pdata[14] = 1; //Buffer
//        pdata[13] = 1; // 1x gain
//        pdata[12] = 1; //Active mode operation
//        int i = 0;
//        for(i; i < 7; i++){ //load data into pAdata
//            pdata[4 + i] = data[i];
//        }
 
        char pdata[16] = 0; //initialize packaged data
        pdata = (pdata | data) << 4; //add data to pdata
        pdata = pdata | 0xF000; //set all special bits to 1 initially
        pdata[15] = AB; //0 = A, 1 = B
        return pdata;
    }
    
    void writeSPI(char data[]){
        LATAbits.LATA0 = 0; //CS needs to be set low to send data
        SPI1BUF = data[]; //send data
        while(!SPI1STATbits.SPIRBF){ //wait to receive data
            ;
        }
        LATAbits.LATA0 = 0; //CS needs to be set high to finish sending data
    }
    
    void setVoltage(char channel, char voltage[]){ //channel = 0 means A, 1 means B
        writeSPI((packageData(channel, voltage[]) & 0xFF00) >> 8); //send most significant byte
        writeSPI(packageData(channel, voltage[]) & 0x00FF); //send least significant byte
    }
    
    //Signal initialization functions
    char *sineTable(void){
        char sT[100] = 0;
        int i = 0;
        for(i; i < 100; i++){
            sT[i] = 
        }
    }
    
    while(1) {
        
        
	    // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
		  // remember the core timer runs at half the CPU speed
//        _CP0_SET_COUNT(0); //set count to zero 
//        if(LATAbits.LATA4 == 1){ //toggle LED
//            LATAbits.LATA4 = 0;
//        }
//        else{
//            LATAbits.LATA4 = 1;
//        }
//        while(_CP0_GET_COUNT() < 12000){ //wait 0.5 ms 
//            ; //do nothing
//        }
//        while(PORTBbits.RB4 == 0){ //while user button is pressed
//            LATAbits.LATA4 = 0; //turn off LED
//        }
    }
}