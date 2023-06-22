/*
  Delay Command
  Created By: Trisna Julian
  
  >Version: 0.9
  Tries to prove concept
  
  >Version: 1.0
  Date: 23-11-2017
  Implementation
  +adding some comment
  >Version: 1.1
  Date: 24-11-2017
  Improvement
  +adding delay command ms
  +default delay is in second unit
  
  >Version: 1.2
  Date: 21-12-2017
  Improvement
  +adding array command so delayCommand will check and do up to 6 command with its priority_quee
  +adding priority into function with 
 
 TODO:
 Better make this to be a class so any improvement doesn't need to adding more function. 
 Instead, just the adding constructor for improvement.
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 
 */

//time target in second

#ifndef DELAYCMD_h
#define DELAYCMD_h

#include "wiring_private.h"
const int DELAY_COMMAND_NUMBER=5;

volatile long timeReference[DELAY_COMMAND_NUMBER];
volatile long timeTarget[DELAY_COMMAND_NUMBER];
volatile bool delayEnable[DELAY_COMMAND_NUMBER];



static void nothingDo(void) {
}

static volatile voidFuncPtr doCommand[DELAY_COMMAND_NUMBER] = {
    nothingDo,
    nothingDo,
    nothingDo,
    nothingDo,
    nothingDo
};

//timeTarget on millisecond (ms) unit
void attachDelayCommand_ms(void (*_doCommand)(),long _timeTarget, uint8_t _priority){
  timeReference[_priority]=millis();
  timeTarget[_priority]=_timeTarget;
  doCommand[_priority]=_doCommand;
  delayEnable[_priority]=true;
}

//timeTarget on second (s) unit
void attachDelayCommand(void (*_doCommand)(),long _timeTarget,uint8_t _priority){
  timeReference[_priority]=millis();
  timeTarget[_priority]=_timeTarget*1000;
  doCommand[_priority]=_doCommand;
  delayEnable[_priority]=true;
}

void detachDelayCommand(uint8_t _priority){
  delayEnable[_priority]=false;
}

//cek time
//do command if time pass reach time target
void delayEvent(){
  long timePass[5]={0,0,0,0,0};
  long timeNow=millis();
  for(int i=0;i<5;i++){
  timePass[i]=timeNow-timeReference[i];
  }
  for(int i=0;i<5;i++){
  if(timePass[i]>=timeTarget[i]&&delayEnable[i]){
    timeReference[i]=millis();
    doCommand[i]();
  }
  }
}

#endif
