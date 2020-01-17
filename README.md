# Rover-Holonomic-Drive
Holonomic Drive Controller for CalRover

This project was developed using PSoC creator to be run on PSoC 5lp micro-controllers
There are two projects. One runs on the 5lp that controls the steering servos, and the other controls the drive motors. 

Commands can be sent to 5lp using I2C  
The code is currently setup to accept I2C commands. 

To send a command open up bridge control panel and send them in the form of:
w 08 00 0x 0x p    
The w is for write, the 08 is the address of the I2C slave set up in the code and  0x represents a 2 digit (1 byte) hex number.

The holonomic servos/motors code is set up to take a velocity in x and y and a rotation (with respect to the rover body) .

The I2C commands are parsed, such that, the two data bytes represented by ux and yz in; w 08 ux yz p.
u=command, which delineates whether the subsequent data is vx, vx, omega, or a send to pwm command. ( 0=vx ; 1= vw; 2 = omega; f = send pwm )
x= the sign of the data ( 1= neg 0= pos ).
yz= the magnitude of the vx, vy , omega in hex 


For example: w 08 00 01 23 p would tell the rover to set vx to -23 in hex or -35 in decimal.

note : none of the commands are sent to the motors or servos until the command w 08 00 f0 00 p is sent. This allows a vx, vy and omega to be executed at the same time.

