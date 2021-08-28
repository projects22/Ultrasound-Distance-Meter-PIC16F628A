
// OLED ULTRASONIC DISTANCE .  
// By moty22.co.uk
// PIC16F628A

#include <htc.h>
#include "oled_font.c"

#pragma config WDTE=OFF, MCLRE=OFF, BOREN=OFF, FOSC=HS, CP=OFF, CPD=OFF, LVP=OFF

#define _XTAL_FREQ 8000000
#define __delay_us(x) _delay((unsigned long)((x)*(_XTAL_FREQ/4000000.0)))
#define __delay_ms(x) _delay((unsigned long)((x)*(_XTAL_FREQ/4000.0)))

#define trig RA0	//pin17
#define echo RA1	//pin18
#define SCL RA3 //pin2
#define SDA RA2 //pin1

//prototypes
void cmnd (unsigned char track);
void command( unsigned char comm);
void oled_init();
void clrScreen();
void sendData(unsigned char dataB);
void startBit(void);
void stopBit(void);
void clock(void);
void drawChar2(char fig, unsigned char y, unsigned char x);

unsigned char addr=0x78;  //0b1111000

void main(void){
	unsigned char d[4];
    unsigned int count;
    
    	// PIC I/O init
    CMCON = 0b111;		//comparator off
    TRISA = 0b110010;   //SCL, SDA, busy
    SCL=1;
    SDA=1;
    trig=0;
    T1CON = 0x00;        //TMR1 prescale 1:1, osc=off, timer off

    __delay_ms(1000);
    oled_init();

    clrScreen();       // clear screen
    drawChar2(10, 1, 7);  //c
    drawChar2(11, 1, 8);  //m
    drawChar2(13, 1, 4);  //.
    
	while(1) {
        TMR1L=0;
        TMR1H=0;
        trig=1; __delay_us(100); trig=0;
        while(!echo);
        TMR1ON=1;
        while(echo);
        TMR1ON=0;
        count=((TMR1H << 8) + TMR1L)/12;    //137

        d[0]=(count/1000) %10; 	//digit on left
        d[1]=(count/100) %10;	
        d[2]=(count/10) %10; 	//digit on left
        d[3]=count %10;

        drawChar2(d[0], 1, 1); 
        drawChar2(d[1], 1, 2);
        drawChar2(d[2], 1, 3);
        drawChar2(d[3], 1, 5);

        __delay_ms(500);
	}
	
}

   //size 2 chars
void drawChar2(char fig, unsigned char y, unsigned char x)
{
    unsigned char i, line, btm, top;    //
    
    command(0x20);    // vert mode
    command(0x01);

    command(0x21);     //col addr
    command(13 * x); //col start
    command(13 * x + 9);  //col end
    command(0x22);    //0x22
    command(y); // Page start
    command(y+1); // Page end
    
    startBit();
    sendData(addr);            // address
    sendData(0x40);

    for (i = 0; i < 5; i++){
        line=font[5*(fig)+i];
        btm=0; top=0;
            // expend char    
        if(line & 64) {btm +=192;}
        if(line & 32) {btm +=48;}
        if(line & 16) {btm +=12;}           
        if(line & 8) {btm +=3;}

        if(line & 4) {top +=192;}
        if(line & 2) {top +=48;}
        if(line & 1) {top +=12;}        

         sendData(top); //top page
         sendData(btm);  //second page
         sendData(top);
         sendData(btm);
    }
    stopBit();
        
    command(0x20);      // horizontal mode
    command(0x00);    
        
}

void clrScreen()    //fill screen with 0
{
    unsigned char y, i;
    
    for ( y = 0; y < 8; y++ ) {
        command(0x21);     //col addr
        command(0); //col start
        command(127);  //col end
        command(0x22);    //0x22
        command(y); // Page start
        command(y+1); // Page end    
        startBit();
        sendData(addr);            // address
        sendData(0x40);
        for (i = 0; i < 128; i++){
             sendData(0x00);
        }
        stopBit();
    }    
}

//Software I2C
void sendData(unsigned char dataB)
{
    for(unsigned char b=0;b<8;b++){
       SDA=(dataB >> (7-b)) % 2;
       clock();
    }
    TRISA2=1;   //SDA input
    clock();
    __delay_us(5);
    TRISA2=0;   //SDA output

}

void clock(void)
{
   __delay_us(1);
   SCL=1;
   __delay_us(5);
   SCL=0;
   __delay_us(1);
}

void startBit(void)
{
    SDA=0;
    __delay_us(5);
    SCL=0;

}

void stopBit(void)
{
    SCL=1;
    __delay_us(5);
    SDA=1;

}

void command( unsigned char comm){
    
    startBit();
    sendData(addr);            // address
    sendData(0x00);
    sendData(comm);             // command code
    stopBit();
}

void oled_init() {
    
    command(0xAE);   // DISPLAYOFF
    command(0x8D);         // CHARGEPUMP *
    command(0x14);     //0x14-pump on
    command(0x20);         // MEMORYMODE
    command(0x0);      //0x0=horizontal, 0x01=vertical, 0x02=page
    command(0xA1);        //SEGREMAP * A0/A1=top/bottom 
    command(0xC8);     //COMSCANDEC * C0/C8=left/right
    command(0xDA);         // SETCOMPINS *
    command(0x22);   //0x22=4rows, 0x12=8rows
    command(0x81);        // SETCONTRAST
    command(0xFF);     //0x8F
    //next settings are set by default
//    command(0xD5);  //SETDISPLAYCLOCKDIV 
//    command(0x80);  
//    command(0xA8);       // SETMULTIPLEX
//    command(0x3F);     //0x1F
//    command(0xD3);   // SETDISPLAYOFFSET
//    command(0x0);  
//    command(0x40); // SETSTARTLINE  
//    command(0xD9);       // SETPRECHARGE
//    command(0xF1);
//    command(0xDB);      // SETVCOMDETECT
//    command(0x40);
//    command(0xA4);     // DISPLAYALLON_RESUME
 //   command(0xA6);      // NORMALDISPLAY
    command(0xAF);          //DISPLAYON

}

   
