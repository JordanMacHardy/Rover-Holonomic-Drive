/* ========================================
 *Code Writen By Jordan MacHardy, feb 2018
 *
*/

// Header Files //
#include "project.h"
#include <stdio.h>
#include <math.h>


// Update these dimensions //
#define wb1 1.0f // Wheel base 1
#define wb2  1.0f // Wheel base 2
#define tw  0.5f // 1/2 Track Width

// Initialize Functions //
void ind_wheel_spd_ang(float vx, float vy, float rx, float ry, float w);
void body2wheel_vel(float vx, float vy, float w);

// Initialize Dynamics Variables  //
float spd_ang[2]; // Initialize temporary speed and angle calculation
float wheel_vel[12]; /* Initialize array of all wheel speeds and angles. 
Alternating speed and angle references starting from front left wheel and counting counter-clockwise */
// x and y locations of all wheels //
float rx_all[6] = {wb1,0,-wb2,-wb2,0,wb1};
float ry_all[6] = {tw,tw,tw,-tw,-tw,-tw};

 ////////////////// In itialize Com/PWM Variables ////////////////////////////// 
char str1[100]; // For printing to uart
float VxIn=0.0;     // Temparary Velocity in x
float VyIn=0.0;     // Temparary Velocity in y
float OmegaIn=0.0;  // Temp rotation

// Debug Variables ///
int vx_debug=0;
int vy_debug=0;
int w_debug=0;

uint8 compare=0x05;   // First Byte of transfered data
uint8 compare2=0xdc; // second byte of transfered data
uint8 command;       // stores the command variable
uint8 commandData;   // stores the data asociated with command

uint8 i2cbuf[2];    // initilize buffer array
uint8 dataSign;     // holds sign of data
float motorSpeedScale=1; // Scales motor speen ;)
float mpsOffset=0; // offset for motor speed scaling
int dataReady=0;  // flag variable keeps PWM from being continualy writen to firmware modules

// Initialize PWM Arrays ////
int pwmServos[6]={1507,1507,1507,1507,1507,1507}; // 1507 is the zero angle of servos (midle of range)
int pwmMotors[6]={1500,1500,1500,1500,1500,1500}; // pwm of 1500 is arbatrarily set to be 0 vel.
float servoOffset[6]={0,-20,0,0,0,0}; // servo offsets to manualy ajust servos that look out of wack.   
      

// Begin main program .... Not main loop //
int main(void)
{
    // Init Values In buffer //
    i2cbuf[0]=0;
    i2cbuf[1]=0;
    
   CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    PWM1_Start(); PWM2_Start(); PWM3_Start(); PWM4_Start(); PWM5_Start(); PWM6_Start();   
    I2C_Start();
    I2C_SetBuffer1(2,2, i2cbuf);
    UART_Start();
    
    // Initialize servos //
    PWM1_WriteCompare(pwmServos[0]);CyDelay(10); PWM2_WriteCompare(pwmServos[1]);CyDelay(10); PWM3_WriteCompare(pwmServos[2]); CyDelay(10);
    PWM4_WriteCompare(pwmServos[3]); CyDelay(10); PWM5_WriteCompare(pwmServos[4]);CyDelay(10); PWM6_WriteCompare(pwmServos[5]);CyDelay(10);
    UART_PutString(" Servos Initialized \n");

   
   for(;;){

   /*sprintf(str1, " buf1 %x  buf2  %x  val  %d  \n" ,i2cbuf[0],i2cbuf[1], pwmServos[5]);
    UART_PutString(str1 ); */
    
      
    // Checks if there is new data in the buffer //
    if(compare != i2cbuf[0] || compare2 != i2cbuf[1]){
    CyDelay(10);
        
    // Packaging Incoming Data //
    // breaks the recieved 16bit hex number into command data and sign //
    compare = i2cbuf[0];  // // sets compare values to buffer.. this makes it so the "new" data only gets writen once
    compare2 = i2cbuf[1]; 
    command = (compare & 0xf0)>>4;
    commandData= compare2;
    dataSign = (compare & 0x0f);
        
    sprintf(str1, " command %x \n sign neg %x \n Data %x \n " , command,  dataSign, commandData);
    UART_PutString(str1 );
     
    if( command != 0xf)   // if the comand isnt being sent to the PWMs  (0xf sends data to pwm) 
    {
 // Check The command puts data in apropriate variable //
       // Velocity in X
        if( command == 0) 
        { 
            if( dataSign == 1)
            {
                VxIn = -commandData;
                vx_debug= -commandData;
            }
            else 
            {
                VxIn = commandData;
                vx_debug= commandData;
            }
            
        }   
        // Velocity In y
        else if( command == 1) 
        { 
            if( dataSign == 1)
            {
                VyIn = -commandData;
                vy_debug= -commandData;
            }
            else 
            { 
                VyIn = commandData;
                vy_debug= commandData;
            }
        }
        
        // Rotation Counter Clockwise
        else if( command == 2)  
        { 
            if( dataSign == 1)
            {
                OmegaIn = -commandData;
                w_debug= -commandData;
            }
            else
            {
                OmegaIn = commandData;
                w_debug = commandData;
            }
        }
     
        dataReady=1; // if data ready=0 command 0xf will not send pwms. this  prevents continual writing of pwms.
     // UART_PutString(" Data Ready");
    }
        
    }
    
// If commands is 0xf and there is new data
 if ( command == 0xf && dataReady == 1 )
    {
    
    sprintf(str1, " VX %d \n VY %d \n w %d \n", vx_debug, vy_debug, w_debug);
    UART_PutString(str1);
    
    // Dynamics Math ( see functions bellow//
    body2wheel_vel(VxIn, VyIn, OmegaIn);
    
    
 
    
     // Write PWM to Motors and Print to UART //
    PWM1_WriteCompare(pwmMotors[0]); PWM2_WriteCompare(pwmMotors[1]); PWM3_WriteCompare(pwmMotors[2]);
    PWM4_WriteCompare(pwmMotors[3]); PWM5_WriteCompare(pwmMotors[4]); PWM6_WriteCompare(pwmMotors[5]); 
    sprintf(str1,"motor1  %d \n motor2  %d \n motor3  %d \n motor4  %d \n motor5  %d \n motor6 %d  \n ",pwmMotors[0],pwmMotors[1],pwmMotors[2],pwmMotors[3],pwmMotors[4], pwmMotors[5]);
    UART_PutString(str1 );

    
    UART_PutString(" PWM Received \n");
    dataReady=0; // the new data has been sent... wait for new data
    CyDelay(10);
    
    }
     
  }
}
   
// Function definitions ////
void body2wheel_vel(float vx, float vy, float w){

    // Calculate wheel spends and angles
    // Wheels are numbered 1-6 starting from front
    // left and rotation counter-clockwise
    for (int i = 0; i<6; i++) {
        ind_wheel_spd_ang(vx,vy,rx_all[i],ry_all[i],w);
        pwmMotors[i]= roundf(spd_ang[0] * motorSpeedScale + mpsOffset); // can comment out for servos
        pwmServos[i]= roundf(1507 - spd_ang[1]* 250.93); // for the 1050-1950 range the offset is 900/288 +1500
    }
}
// Called by function above. These could be put into one function
void ind_wheel_spd_ang(float vx, float vy, float rx, float ry, float w){
    float v_wheelx = vx + ry*w;
    float v_wheely = vy - rx*w;
    spd_ang[0] = sqrtf( powf(v_wheelx,2)+powf(v_wheely,2)); // Wheel Speed -- can comment out for servos
    spd_ang[1] = atanf(v_wheely/v_wheelx); // Wheel Angle
    
    // this deals with undefined angles when vx=0
    if( v_wheelx > 0.01 ){
        spd_ang[1] = atanf(v_wheely/v_wheelx); // Wheel Angle
    }
    else if (v_wheely<0){
        spd_ang[1]= -1.570796;
    }
    else{
        spd_ang[1]= 1.570796;
    }

}

/* [] END OF FILE */
