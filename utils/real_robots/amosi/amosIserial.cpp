/***************************************************************************
 *   Copyright (C) 2012 by Robot Group Goettingen                          *
 *                                    									   *
 *    fhesse@physik3.gwdg.de     			                               *
 *    xiong@physik3.gwdg.de                  	                           *
 *    poramate@physik3.gwdg.de                                             *
 *   LICENSE:                                                              *
 *   This work is licensed under the Creative Commons                      *
 *   Attribution-NonCommercial-ShareAlike 2.5 License. To view a copy of   *
 *   this license, visit http://creativecommons.org/licenses/by-nc-sa/2.5/ *
 *   or send a letter to Creative Commons, 543 Howard Street, 5th Floor,   *
 *   San Francisco, California, 94105, USA.                                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 *   AMOSII v2 has now only 18 sensors                                     *
 ***************************************************************************/

// include header file
#include "amosIserial.h"




namespace lpzrobots {

AmosIISerialV2::AmosIISerialV2(const char *port)
: AbstractRobot("AmosISerial", "$Id: main.cpp,v 0.1 2011/14/07 18:00:00 fhesse $"),
  port(port) {

	fd1=open(port, O_RDWR | O_NOCTTY | O_NDELAY);//make sure your account in PC can have access to serial port


	if(fd1 == -1)
	{
		std::cout<<std::endl<<"unable to open"<<port<<std::endl;
		std::cout<<"check provided port (hint: \"/dev/ttyS0\" or \"/dev/ttyUSB0\" required?)"<<std::endl;
		assert (fd1 != -1);
	}
	else
	{
		fcntl(fd1, F_SETFL, 0);
		printf("port is open");

		memset(&tio,0,sizeof(tio));
		tio.c_iflag=0;
		tio.c_oflag=0;
		tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
		tio.c_lflag=0;
		tio.c_cc[VMIN]=1;
		tio.c_cc[VTIME]=5;


		cfsetospeed(&tio,B57600);            // 57600 baud
		cfsetispeed(&tio,B57600);            // 57600 baud

		tcsetattr(fd1,TCSANOW,&tio);


		printf("port finish configuration \n");
	}
	comByte=2;
	end=0;
	index=0;
	sensor1=1;


	t=0;  // global step counter

	sensornumber = AMOSII_SENSOR_MAX;
	motornumber = AMOSII_MOTOR_MAX;

	//Setting Motors
	for (int t=0;t<33;t++)
	{
		serialPos[t]=128; //Setting all motor to middle position as initialization
	}

}


AmosIISerialV2::~AmosIISerialV2(){


	//Close COM0
	if(fd1 != -1){
		close(fd1);
		std::cout<<"closed the serial communication port"<<std::endl;
	}
}

// robot interface
/** returns actual sensorvalues
  @param sensors sensors scaled to [-1,1]
  @param sensornumber length of the sensor array
  @return number of actually written sensors
 */
int AmosIISerialV2::getSensors(sensor* sensors, int sensornumber){

	assert(sensornumber >= this->sensornumber);

	for(int i=0; i<=AMOSII_SENSOR_MAX;i++){
		sensors[i]=0;
	}

	comByte=2;
	end=0;

	char serial_msg[COMMAND_BUFFER_NUM];
	do{

		//Sending "getSensors" command to the board
		sprintf(serial_msg, "%c%c",comByte,end);

		wr = write(fd1, serial_msg,sizeof(serial_msg));

		// --- Reading the potentiometer values
		for (int i=1;i<SENSOR_BUFFER_NUM;i++){
			do{

				rd = read(fd1, &chBuff, 1);
				if (rd){
					potValue[i]=(unsigned char)(chBuff);// potvalue are AMOS sensor data
				}
			}while(!rd);

		}

	}while((unsigned char)(chBuff)!=0);// "0" is sync byte


	// LpzRobot <-- AMOS
	//Foot sensors (FS,Group 1)

	//Change here
	/*R0_fs*/sensors[R0_fs]=potValue[R0_fs_real]; //[min = 7 (off ground), max =  207 (touch ground)]
	/*R1_fs*/sensors[R1_fs]=potValue[R1_fs_real]; //[min = 15 (off ground), max = 196 (touch ground)]
	/*R2_fs*/sensors[R2_fs]=potValue[R2_fs_real]; //[min = 20 (off ground), max = 200 (touch ground)]

	/*L0_fs*/sensors[L0_fs]=potValue[L0_fs_real]; //[min = 28 (off ground), max = 196 (touch ground)]
	/*L1_fs*/sensors[L1_fs]=potValue[L1_fs_real]; //[min = 27 (off ground), max = 195 (touch ground)]
	/*L2_fs*/sensors[L2_fs]=potValue[L2_fs_real]; //[min = 20 (off ground), max = 200 (touch ground)]

	//US sensor (US, Group 2) // UNDK30U6112 range = 6 cm-40cm
	/*FR_us*/sensors[FR_us]=potValue[FR_us_real];//[min = 1 (detect object ~ 6 cm at front), max 135 (detect object at 40 cm),max 220 (detect object at 62 cm), max = 255 (not detect very close object)]
	/*FL_us*/sensors[FL_us]=potValue[FL_us_real];//[min = 1 (detect object ~ 6 cm at front), max 135 (detect object at 40 cm),max 220 (detect object at 62 cm), max = 255 (not detect very close object)]

	//Reflex ir sensors at leg (IRS, Group 3)
	/*R0_irs*/sensors[R0_irs]=potValue[R0_irs_real];//[min = 2 (not detect object), max 75 (detect object ~ 3mm at front)
	/*R1_irs*/sensors[R1_irs]=potValue[R1_irs_real];//[min = 2 (not detect object), max 75 (detect object ~ 3mm at front)
	/*R2_irs*/sensors[R2_irs]=potValue[R2_irs_real];//[min = 2 (not detect object), max 75 (detect object ~ 3mm at front)
	/*L0_irs*/sensors[L0_irs]=potValue[L0_irs_real];//[min = 2 (not detect object), max 75 (detect object ~ 3mm at front)
	/*L1_irs*/sensors[L1_irs]=potValue[L1_irs_real];//[min = 2 (not detect object), max 75 (detect object ~ 3mm at front)
	/*L2_irs*/sensors[L2_irs]=potValue[L2_irs_real];//[min = 2 (not detect object), max 75 (detect object ~ 3mm at front)

	//Photo sensors (PS, group 4)
	/*M_ps*/sensors[M_ps]=potValue[M_ps_real];//[min = 1 (very dark), max 250 (detect light (very bright))
	/*R_ps*/sensors[R_ps]=potValue[R_ps_real];//[min = 1 (very dark), max 250 (detect light (very bright))
	/*L_ps*/sensors[L_ps]=potValue[L_ps_real];//[min = 1 (very dark), max 250 (detect light (very bright))

	//Average current sensors (ACS, group 5)
	//[LTS 6-NP sensor, return voltage ~ current Ip, conversion => 0.0195312*sensors[A_cs] = volt output=> I = (volt output-2.5)*6/0.625, P = I*V = I*5 v, Joule = P*t(second)]
	//[ZAP 25] (ac_board*(5.0/256)-2.5)/0.037
	/*A_cs*/sensors[A_cs]=potValue[A_cs_real];// [ZAP 25]

	//Inclinometer sensors (INS, group 6)
	/*In_x*/sensors[In_x]=potValue[In_x_real]; //around x axis (forward walking direction)
	/*In_y*/sensors[In_y]=potValue[In_y_real]; //around y axis (sideward walking direction)

	//Conversion to positive range [0,..,255]
	for(int i=0; i<=AMOSII_SENSOR_MAX;i++){
		if (sensors[i] < 0){
			sensors[i]+=256;
		}
	}


	bool default_preprocessing = true;
	if (default_preprocessing){
		processSensors(sensors);
	}

	//Your own,e.g.,
	bool koh_preprocessing = false;
	if (koh_preprocessing){
		processSensorsKOH(sensors);
	}


	return this->sensornumber;

}

/*Different sensors processing*/
void AmosIISerialV2::processSensors(sensor* psensors){

	//Foot sensor (FS, Group 1): Scaling to 0 (off ground),..,1 (on ground)


	//Change here later Diusable this and check the raw data to make the magic number
	psensors[R0_fs]= ((psensors[R0_fs]-15)/(90-15));  //[min = 7 (off ground), max =  207 (on ground)]
	psensors[R1_fs]= ((psensors[R1_fs]-60)/(120-60)); //[min = 15 (off ground), max = 196 (on ground)]
	psensors[R2_fs]= ((psensors[R2_fs]-30)/(110-30)); // [min = 20 (off ground), max = 200 (on ground)]
	psensors[L0_fs]= ((psensors[L0_fs]-40)/(100-40)); //[min = 28 (off ground), max = 196 (on ground)]
	psensors[L1_fs]= ((psensors[L1_fs]-30)/(130-30)); //[min = 27 (off ground), max = 195 (on ground)]
	psensors[L2_fs]= ((psensors[L2_fs]-40)/(110-40)); //[min = 20 (off ground), max = 200 (on ground)]

	//Clipping foot signals
	for(int i = R0_fs; i<= L2_fs; i++){
		if(psensors[i]>1.0)
			psensors[i] = 1.0;
		if(psensors[i]<0.0)
			psensors[i] = 0.0;
	}

	//US sensor (US, Group 2) // UNDK30U6112 range = 6 cm-40cm : Scaling to 0 (not detect),...,1 (detect obstacles)
	//eduard
	//scales the us sensor from 0 to 1. 40cm is 0 and 0cm is 1
	//thresholding the US sensors
	if((psensors[FR_us]<132)||(psensors[FL_us]<132)){
		psensors[FR_us]=((psensors[FR_us]-132)/(1-132));//[min = 1 (detect object ~ 6 cm at front), max 135 (detect object at 40 cm),max 220 (detect object at 62 cm), max = 255 (not detect very close object)]
		psensors[FL_us]=((psensors[FL_us]-132)/(1-132));//[min = 1 (detect object ~ 6 cm at front), max 135 (detect object at 40 cm),max 220 (detect object at 62 cm), max = 255 (not detect very close object)]
	}
	else{
		psensors[FR_us]=0;//[min = 1 (detect object ~ 6 cm at front), max 135 (detect object at 40 cm),max 220 (detect object at 62 cm), max = 255 (not detect very close object)]
		psensors[FL_us]=0;//[min = 1 (detect object ~ 6 cm at front), max 135 (detect object at 40 cm),max 220 (detect object at 62 cm), max = 255 (not detect very close object)]
	}
	if(psensors[FR_us]<0)psensors[FR_us]=0;
	if(psensors[FL_us]<0)psensors[FL_us]=0;


	//eduard
	//scales ir sensors from 0 ...1 . 0 is no object and 1 is very close to sensor, about 3mm
	//	psensors[R0_irs]=((psensors[R0_irs]-5)/(50-5));//[min = 2 (not detect object), max 75 (detect object ~ 3mm at front)
	//	psensors[R1_irs]=((psensors[R1_irs]-40)/(55-40));//[min = 2 (not detect object), max 75 (detect object ~ 3mm at front)
	//	psensors[R2_irs]=((psensors[R2_irs]-40)/(55-40));//[min = 2 (not detect object), max 75 (detect object ~ 3mm at front)
	//	psensors[L0_irs]=((psensors[L0_irs]-5)/(50-5));//[min = 2 (not detect object), max 75 (detect object ~ 3mm at front)
	//	psensors[L1_irs]=((psensors[L1_irs]-20)/(55-20));//[min = 2 (not detect object), max 75 (detect object ~ 3mm at front)
	//	psensors[L2_irs]=((psensors[L2_irs]-1)/(80-1));//[min = 2 (not detect object), max 75 (detect object ~ 3mm at front)

	psensors[R0_irs]=((psensors[R0_irs]-1)/(80-1));//[min = 2 (not detect object), max 75 (detect object ~ 3mm at front)
	psensors[R1_irs]=((psensors[R1_irs]-5)/(55-5));//[min = 2 (not detect object), max 75 (detect object ~ 3mm at front)
	psensors[R2_irs]=((psensors[R2_irs]-1)/(55-1));//[min = 2 (not detect object), max 75 (detect object ~ 3mm at front)
	psensors[L0_irs]=((psensors[L0_irs]-1)/(80-1));//[min = 2 (not detect object), max 75 (detect object ~ 3mm at front)
	psensors[L1_irs]=((psensors[L1_irs]-1)/(80-1));//[min = 2 (not detect object), max 75 (detect object ~ 3mm at front)
	psensors[L2_irs]=((psensors[L2_irs]-1)/(55-1));//[min = 2 (not detect object), max 75 (detect object ~ 3mm at front)

	if(psensors[R0_irs]>1)psensors[R0_irs]=1;
	if(psensors[R1_irs]>1)psensors[R1_irs]=1;
	if(psensors[R2_irs]>1)psensors[R2_irs]=1;
	if(psensors[L0_irs]>1)psensors[L0_irs]=1;
	if(psensors[L1_irs]>1)psensors[L1_irs]=1;
	if(psensors[L2_irs]>1)psensors[L2_irs]=1;

	if(psensors[R0_irs]<0)psensors[R0_irs]=0;
	if(psensors[R1_irs]<0)psensors[R1_irs]=0;
	if(psensors[R2_irs]<0)psensors[R2_irs]=0;
	if(psensors[L0_irs]<0)psensors[L0_irs]=0;
	if(psensors[L1_irs]<0)psensors[L1_irs]=0;
	if(psensors[L2_irs]<0)psensors[L2_irs]=0;
	//end eduard


	//Photo psensors (PS, group 4) : Scaling to 0 (dark),..,1 (bright)
	psensors[M_ps]=((psensors[M_ps]-1)/(250-1));//[min = 1 (very dark), max 250 (detect light (very bright))
	psensors[R_ps]=((psensors[R_ps]-1)/(250-1));//[min = 1 (very dark), max 250 (detect light (very bright))
	psensors[L_ps]=((psensors[L_ps]-1)/(250-1));//[min = 1 (very dark), max 250 (detect light (very bright))


}




/** sets actual motorcommands
  @param motors motors scaled to [-1,1]
  @param motornumber length of the motor array
 */
void AmosIISerialV2::setMotors(const motor* motors, int motornumber){

	assert(motornumber >= this->motornumber);


	end =0; // Null as Sync-byte
	comByte=1;


	// -------------------- initializing the Motor range ------------------------

//	//TR0_m
//	//Change here//Changed
servoPosMin[0] = 60; //243;
servoPosMax[0] = 160; //6;
//
//TR1_m
servoPosMin[1] = 10; //243;
servoPosMax[1] = 75; //6;
//TR2_m
servoPosMin[2] = 70; //243;
servoPosMax[2] = 165; //6;
//
//TL0_m
servoPosMin[3] = 210; //9;
servoPosMax[3] = 120; //243;
//TL1_m
servoPosMin[4] = 210; //9;
servoPosMax[4] = 145; //243;
//TL2_m
servoPosMin[5] = 180; //9;
servoPosMax[5] = 85; //243;

	double walkingheigh = 15;//50;


	//CR0_m
		servoPosMin[6] = 1; //6;
		servoPosMax[6] = 155-walkingheigh; //250-walkingheigh;
		//CR1_m
		servoPosMin[7] = 5; //250;
		servoPosMax[7] = 135-walkingheigh+10; //6+walkingheigh;
		//CR2_m
		servoPosMin[8] = 1;//250;
		servoPosMax[8] = 135-walkingheigh+10;//6+walkingheigh;

		//CL0_m
		servoPosMin[9] = 215; //250;
		servoPosMax[9] = 65+walkingheigh; //6+walkingheigh;
		//CL1_m
		servoPosMin[10] = 255; //6;
		servoPosMax[10] = 125+walkingheigh-10; //250-walkingheigh;
		//CL2_m
		servoPosMin[11] = 255; //6;
		servoPosMax[11] = 120+walkingheigh; //250-walkingheigh;

	double walkingheigh2 = 0;//10.0;//20;


	//FR0_m
	servoPosMin[12] = 40; //180;//187;
	servoPosMax[12] = 230; //10+walkingheigh2;
	//FR1_m
	servoPosMin[13] = 20; //61;
	servoPosMax[13] = 230; //241-walkingheigh2;
	//FR2_m
	servoPosMin[14] = 20; //60;
	servoPosMax[14] = 240; //243-walkingheigh2;

	//FL0_m
	servoPosMin[15] = 225; //75;//73 63;
	servoPosMax[15] = 50; //245-walkingheigh2;
	//FL1_m
	servoPosMin[16] = 245; //186;
	servoPosMax[16] = 20; //7+walkingheigh2;
	//FL2_m
	servoPosMin[17] = 245; //189;
	servoPosMax[17] = 10; //6+walkingheigh2;

	//M18 backbone joint
	servoPosMin[18] = 5;
	servoPosMax[18] = 60;

	//Tail Lower
	servoPosMin[19] = 240;
	servoPosMax[19] = 10;

	//Tail Upper
	servoPosMin[20] = 240;
	servoPosMax[20] = 10;


//--------------------------------------------------------------------------------------------


	//TR0_m
	//Change here//Changed
	servoPosMin[0] = 50; //243;
	servoPosMax[0] = 170; //6;

	//TR1_m
	servoPosMin[1] = 1; //243;
	servoPosMax[1] = 100; //6;


	//TR2_m
	servoPosMin[2] = 60;//70; //243;
	servoPosMax[2] = 175;//165; //6;

	//TL0_m
	servoPosMin[3] = 220;//210; //9;
	servoPosMax[3] = 100;//120; //243;
	//TL1_m
	servoPosMin[4] = 220;//210; //9;
	servoPosMax[4] = 125;//145; //243;
	//TL2_m
	servoPosMin[5] = 190;//180; //9;
	servoPosMax[5] = 75;//85; //243;






	// ##################### move motors ################
	for(int i=0;i<AMOSII_MOTOR_MAX;i++)
	{
		motorCom[i] = motors[i];// set LpzMotor value before processing and sending
		if (i<12 && i>5) {
			motorCom[i]-=0.1;
		}
		if (motorCom[i]>1) motorCom[i]=1;
		if (motorCom[i]<-1) motorCom[i]=-1;
	}



	//TR0
	//Change here//Changed
	serialPos[20] = /*160;//Done! */(int) (double)(((motorCom[0]+1.0)/2.0)*(servoPosMax[0]-servoPosMin[0])+servoPosMin[0]) ;//22
	serialPos[25] = /*75;//Done!*/(int) (double)(((motorCom[1]+1.0)/2.0)*(servoPosMax[1]-servoPosMin[1])+servoPosMin[1]) ;//26
	serialPos[31] = /*70;//Done!*/(int) (double)(((motorCom[2]+1.0)/2.0)*(servoPosMax[2]-servoPosMin[2])+servoPosMin[2]) ;//12
	serialPos[10] = /*120;//Done!*/(int) (double)(((motorCom[3]+1.0)/2.0)*(servoPosMax[3]-servoPosMin[3])+servoPosMin[3]) ;//21
	serialPos[14] = /*145;//Done!*/(int) (double)(((motorCom[4]+1.0)/2.0)*(servoPosMax[4]-servoPosMin[4])+servoPosMin[4]) ;//29
	serialPos[3] = /*170;//Done!*/(int) (double)(((motorCom[5]+1.0)/2.0)*(servoPosMax[5]-servoPosMin[5])+servoPosMin[5]) ;//11

	//CR0
	serialPos[17] = /*115;//*/(int) (double)(((motorCom[6]+1.0)/2.0)*(servoPosMax[6]-servoPosMin[6])+servoPosMin[6]) ;//23
	serialPos[26] = /*115;//*/(int) (double)(((motorCom[7]+1.0)/2.0)*(servoPosMax[7]-servoPosMin[7])+servoPosMin[7]) ;//30
	serialPos[32] = /*110;//*/(int) (double)(((motorCom[8]+1.0)/2.0)*(servoPosMax[8]-servoPosMin[8])+servoPosMin[8]) ;//13
	serialPos[11] = /*105;//*/(int) (double)(((motorCom[9]+1.0)/2.0)*(servoPosMax[9]-servoPosMin[9])+servoPosMin[9]) ;//19
	serialPos[13] = /*150;//*/(int) (double)(((motorCom[10]+1.0)/2.0)*(servoPosMax[10]-servoPosMin[10])+servoPosMin[10]);//28
	serialPos[4] = /*150;//*/(int) (double)(((motorCom[11]+1.0)/2.0)*(servoPosMax[11]-servoPosMin[11])+servoPosMin[11]);//9

	//FR0
	serialPos[19] = /*100;//*/(int) (double)(((motorCom[12]+1.0)/2.0)*(servoPosMax[12]-servoPosMin[12])+servoPosMin[12]);//24
	serialPos[27] = /*120;//*/(int) (double)(((motorCom[13]+1.0)/2.0)*(servoPosMax[13]-servoPosMin[13])+servoPosMin[13]);//25
	serialPos[30] = /*140;//*/(int) (double)(((motorCom[14]+1.0)/2.0)*(servoPosMax[14]-servoPosMin[14])+servoPosMin[14]);//14
	serialPos[12] = /*145;//*/(int) (double)(((motorCom[15]+1.0)/2.0)*(servoPosMax[15]-servoPosMin[15])+servoPosMin[15]);//20
	serialPos[15] = /*120;//*/(int) (double)(((motorCom[16]+1.0)/2.0)*(servoPosMax[16]-servoPosMin[16])+servoPosMin[16]);//27
	serialPos[5] = /*120;//*/(int) (double)(((motorCom[17]+1.0)/2.0)*(servoPosMax[17]-servoPosMin[17])+servoPosMin[17]);//10

	// Backbone Joint
	//serialPos[1] = 120;
	serialPos[18] = (int) (double)(((motorCom[18]+1.0)/2.0)*(servoPosMax[18]-servoPosMin[18])+servoPosMin[18]) ;

	//Tail lower
	serialPos[7] =(int) (double)(((motorCom[16]+1.0)/2.0)*(servoPosMax[19]-servoPosMin[19])+servoPosMin[19]) ;
	//Tail upper
	serialPos[8] =(int) (double)(((motorCom[17]+1.0)/2.0)*(servoPosMax[20]-servoPosMin[20])+servoPosMin[20]) ;

	//usleep(1000);
	usleep(10000);
	// do some processing for motor commands before sending AMOS sensors

	sprintf(serial_motor, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"
			,comByte,serialPos[1],serialPos[2],
			serialPos[3],serialPos[4],serialPos[5],serialPos[6],serialPos[7],serialPos[8],
			serialPos[9],serialPos[10],serialPos[11],serialPos[12],serialPos[13],serialPos[14],
			serialPos[15],serialPos[16],serialPos[17],serialPos[18],serialPos[19],serialPos[20],
			serialPos[21],serialPos[22],serialPos[23],serialPos[24],serialPos[25],serialPos[26],
			serialPos[27],serialPos[28],serialPos[29],serialPos[30],serialPos[31],serialPos[32],end);

	//Sendding command to serial port
	write(fd1, serial_motor, 34);//sizeof(serial_msg));

	// to slow down process a bit
	usleep(10000);


	// increase time counter
	t++;


}

/*Process your sensor signals here to match to your need*/
void AmosIISerialV2::processSensorsKOH(sensor* sensors){

}


}




