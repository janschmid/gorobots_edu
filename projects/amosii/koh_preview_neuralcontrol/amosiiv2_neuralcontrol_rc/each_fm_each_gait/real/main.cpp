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

#include <selforg/agent.h>
#include <selforg/abstractrobot.h>

#include <selforg/one2onewiring.h>
#include <selforg/sinecontroller.h>
//#include <ode_robots/amosiistdscalingwiring.h>

#include "amosIIcontrol.h"

#include "amosIIserialv1.h"  //serial interface to AMOSII version 1
#include "amosIIserialv2.h"  //serial interface to AMOSII version 2

#include <cmath>

#include <stdio.h> // standard input / output functions
#include <string.h> // string function definitions
#include <unistd.h> // standard function definitions
#include <fcntl.h> // File control definitions
#include <errno.h> // Error number definitions
#include <termios.h> // terminal control definitions
#include <time.h>   // time calls

//Adding keyboard function
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <curses.h>


using namespace lpzrobots;


using namespace std;

bool stop=0;
double noise=.0;
double realtimefactor=1;
bool   singleline = true; // whether to display the state in a single line

bool keyboard=false;

// Helper
int contains(char **list, int len,  const char *str){
  for(int i=0; i<len; i++){
    if(strcmp(list[i],str) == 0) return i+1;
  }
  return 0;
};

int main(int argc, char** argv){
  list<PlotOption> plotoptions;
  int port = 1;

  int index = contains(argv,argc,"-g");
  if(index >0 && argc>index) {
    plotoptions.push_back(PlotOption(GuiLogger, atoi(argv[index])));
  }
  if(contains(argv,argc,"-f")!=0) plotoptions.push_back(PlotOption(File));
  if(contains(argv,argc,"-n")!=0) plotoptions.push_back(PlotOption(MatrixViz));
  if(contains(argv,argc,"-l")!=0) singleline=false;
  index = contains(argv,argc,"-p");
  if(index >0 && argc>index)
    port=atoi(argv[index]);
  if(contains(argv,argc,"-h")!=0) {
    printf("Usage: %s [-g N] [-f] [-n] [p PORT]\n",argv[0]);
    printf("\t-g N\tstart guilogger with interval N\n\t-f\twrite logfile\n");
    printf("\t-n\tstart neuronviz\n");
    printf("\t-p PORT\t COM port number, default 1\n");
    printf("\t-h\tdisplay this help\n");
    exit(0);
  }

  GlobalData globaldata;
  AmosIISerialV2* robot;
  Agent* agent;
  initializeConsole();

  // Different controllers

  AbstractController* controller = new AmosIIControl();
  // one2onewiring gives full range of robot actuators
  AbstractWiring* wiring = new One2OneWiring(new WhiteUniformNoise(),true);



  //robot         = new AmosIISerialV2("/dev/ttyS0");     // using serial port
  robot         = new AmosIISerialV2("/dev/ttyUSB0");   // using USB-to-serial adapter



  agent         = new Agent(plotoptions);
  agent->init(controller, robot, wiring);
  // if you like, you can keep track of the robot with the following line.
  // this assumes that your robot returns its position, speed and orientation.
  // agent->setTracMkOptions(TrackRobot(true,false,false, false,"systemtest"));

  globaldata.agents.push_back(agent);
  globaldata.configs.push_back(robot);
  globaldata.configs.push_back(controller);


  showParams(globaldata.configs);
  printf("\nPress c to invoke parameter input shell\n");
  printf("The output of the program is more fun then useful ;-).\n");
  printf(" The number are the sensors and the position there value.\n");
  printf(" You probably want to use the guilogger with e.g.: -g 10\n");

  cmd_handler_init();
  long int t=0;


  if (keyboard==true)
  {
    //Adding keyboard function
    initscr();
    cbreak();
    //  //noecho();
    intrflush(stdscr,FALSE);
    keypad(stdscr,TRUE);
    nodelay(stdscr,TRUE);

    //  cout<<"Options: Press a= Obstacle Avoidance ON/OFF"<<endl;
    //  cout<<"Options: Press b= BJC ON/OFF"<<endl;
    //  cout<<"Options: Press e= Reflex ON/OFF"<<endl;
  }



  while(!stop){
    agent->step(noise,t);

    if (keyboard==true)
    {
      int key=0;
      key = wgetch (stdscr);


      //KEYBOARD BJC OPTION
      if (key==102){ //f
        std::cout << "FORWARD" << endl;
        ((AmosIIControl*) controller)->control_adaptiveclimbing.input.at(3)=-1;
        ((AmosIIControl*) controller)->control_adaptiveclimbing.input.at(4)=-1;
      }

      if (key==98){ //b
        std::cout << "BACK" << endl;
        ((AmosIIControl*) controller)->control_adaptiveclimbing.input.at(3)=-1;
        ((AmosIIControl*) controller)->control_adaptiveclimbing.input.at(4)=-1;
      }

      if (key==108){ //l
        std::cout << "TURN LEFT" << endl;
        ((AmosIIControl*) controller)->control_adaptiveclimbing.input.at(3)=1;
        ((AmosIIControl*) controller)->control_adaptiveclimbing.input.at(4)=-1;
      }

      if (key==114){ //r
        std::cout << "TURN" << endl;
        ((AmosIIControl*) controller)->control_adaptiveclimbing.input.at(3)=-1;
        ((AmosIIControl*) controller)->control_adaptiveclimbing.input.at(4)=1;
      }

      if (key==115){ //s
        std::cout << "TURN" << endl;
        ((AmosIIControl*) controller)->control_adaptiveclimbing.input.at(3)=0.0;
        ((AmosIIControl*) controller)->control_adaptiveclimbing.input.at(4)=0.0;
      }

      if (key==50){ //2
        std::cout << "WAVE" << endl;
        ((AmosIIControl*) controller)->control_adaptiveclimbing.Control_input=0.02;
      }
      if (key==53){ //5
        std::cout << "TETRAPOD" << endl;
        ((AmosIIControl*) controller)->control_adaptiveclimbing.Control_input=0.05;
      }
      if (key==54){ //6
        std::cout << "TETRAPOD" << endl;
        ((AmosIIControl*) controller)->control_adaptiveclimbing.Control_input=0.06;
      }
      if (key==55){ //7
        std::cout << "TRIPOD" << endl;
        ((AmosIIControl*) controller)->control_adaptiveclimbing.Control_input=0.16;
      }
      if (key==57){ //9
        std::cout << "TRIPOD" << endl;
        ((AmosIIControl*) controller)->control_adaptiveclimbing.Control_input=0.19;
      }
    }

    if(control_c_pressed()){

      if(!handleConsole(globaldata)){
        stop=1;
      }
      cmd_end_input();
    }

    t++;
  };
  delete robot;
  delete agent;
  closeConsole();
  fprintf(stderr,"terminating\n");
  // should clean up but what costs the world
  return 0;
}
