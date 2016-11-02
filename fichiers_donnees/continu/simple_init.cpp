#define S_FUNCTION_NAME simple_init 
#include "ttkernel.cpp"
#include "mex.h"
#include <iostream>


// in definition    
class PortIn{
    public:
        const int sol1_5percent = 1; 
        const int sol1_1percent = 2; 
        const int sol2_5percent = 3; 
        const int sol2_1percent = 4; 
        const int stop_antibiotic = 5; 
        const int critical_glycemia = 6; 
        const int average_glycemia = 7;  
};

// out definition
class PortOut{
    public:
        const int sol1 = 2;
        const int sol2 = 1;
        const int anticoagulant = 3;
        const int antibiotic = 4;
        const int glucose_injection = 5;
};

// Data structure used for the task data 
class CtrlData { 
    public:
        bool isSol1Empty;
        bool isSol2Empty;
        int actualSolution;
        double lastAntibioticInjection;
        double lastAnticoagulantInjection;
        double antibioticPeriod;
        double exectime;
        PortOut* out;
        PortIn* in;
        // time constantes
        const double ONE_DAY_IN_SECONDS = 86400;
        const double TWO_MINUTES_IN_SECONDS = 120;
        const double HEIGHT_HOURS_IN_SECONDS = 28800;
        // Const used in simulation
        const double DEFAULT_PRIORITY = 15;
        const double DEFAULT_PERIOD = HEIGHT_HOURS_IN_SECONDS;   
        const double FIRST_INJECTION_CALL = 120; // => 2minutes
};

// handler
double sol1_5percent(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data; 
 
    switch(segment) { 
        case 1:  
            // alert
            mexPrintf("alert");
            return d->exectime; 
     default: 
         return FINISHED; 
     } 
 } 
// handler
double sol1_1percent(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data; 
 
    switch(segment) { 
        case 1:
            ttAnalogOut(d->out->sol1,0);                 
            d->isSol1Empty = true;
                
            if(!d->isSol2Empty){                
                ttAnalogOut(d->out->sol2,1);
                d->actualSolution = 2;
            }
            else{
                // PANIC
                //alarm('no more insuline');
            }
            
            return d->exectime; 
     default: 
         return FINISHED; 
     } 
 } 

// handler
double sol2_5percent(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data; 
 
    switch(segment) { 
        case 1:  
            // alert
            return d->exectime; 
     default: 
         return FINISHED; 
     } 
 } 

// handler
double sol2_1percent(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data; 
 
    switch(segment) { 
        case 1:
            ttAnalogOut(d->out->sol2,0);            
            d->isSol2Empty = true;
                    
            if (!d->isSol1Empty){
                ttAnalogOut(d->out->sol1,1);
                d->actualSolution = 1;
            }
            else{
                // PANIC => bad bad bad
                //alarm('no more insuline');
            }
            return d->exectime; 
     default: 
         return FINISHED; 
     } 
 } 

// handler
double stop_antibiotic(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data; 
 
    switch(segment) { 
        case 1:            
            ttAnalogOut(d->out->antibiotic,0);// stop antibiotic injection
            return d->exectime; 
     default: 
         return FINISHED; 
     } 
 } 

// handler
double critical_glycemia(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data; 
 
    switch(segment) { 
        case 1:  
            // stop everything and make an urgency injection
            ttAnalogOut(d->out->sol1,0);
            ttAnalogOut(d->out->sol2,0);
            ttAnalogOut(d->out->anticoagulant,0);
            ttAnalogOut(d->out->antibiotic,0);
            ttAnalogOut(d->out->glucose_injection,1);
            return d->exectime; 
     default: 
         return FINISHED; 
     } 
 } 

// handler
double average_glycemia(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data; 
 
    switch(segment) { 
        case 1:  
            // stop all injection
            ttAnalogOut(d->out->anticoagulant,0);
            ttAnalogOut(d->out->antibiotic,0);
            ttAnalogOut(d->out->glucose_injection,0);
            //start insuline
            switch(d->actualSolution){
                case 1:
                    ttAnalogOut(d->out->sol1,1);
                   break;
                case 2:
                    ttAnalogOut(d->out->sol2,1);
                    break;                               
            }
            
            return d->exectime; 
     default: 
         return FINISHED; 
     } 
 } 

// handler
double launchInjection(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data; 
 
    switch(segment) { 
        case 1:  
            // launch first injection injection
            ttAnalogOut(d->out->sol1,1);            
            return d->exectime; 
     default: 
         return FINISHED; 
     } 
 } 

// periodic method
double injectAnticoagulant(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data; 
 
    switch(segment) { 
        case 1:  
            if(d->exectime - d->lastAntibioticInjection > d->ONE_DAY_IN_SECONDS){
                ttAnalogOut(d->out->anticoagulant,1);
                ttSleepUntil(d->TWO_MINUTES_IN_SECONDS);//wait 2 min
                ttAnalogOut(d->out->anticoagulant,0);
                d->lastAntibioticInjection = d->exectime;
            }
            
            ttAnalogOut(d->out->antibiotic,1);
            d->lastAnticoagulantInjection = d->exectime;
            return d->exectime; 
     default: 
         return FINISHED; 
     } 
 }



// ====================================== Kernel function 

void init() {
        
    // init data, in and out
    CtrlData* data = new CtrlData;
    data->in = new PortIn;
    data->out = new PortOut;
    // at begining sol1 and sol2 are full 
    data->isSol1Empty = false;
    data->isSol2Empty = false;
    // first solution is the 1
    data->actualSolution = 1;
    // add value to force first injection
    data->lastAntibioticInjection = -1 * data->ONE_DAY_IN_SECONDS;
    data->lastAnticoagulantInjection = -1 * data->ONE_DAY_IN_SECONDS;
    data->antibioticPeriod = data->DEFAULT_PERIOD; // default period and then ask user
    //TODO: ask the user
    
    // first time
    data->exectime = 0,1;
    
    ttInitKernel(prioFP); 
    //mexPrintf("init the kernel");
    
    // =========================== Tasks
    
    // SOLUTION 1, 5 PERCENTAGE
    ttCreateHandler("sol1_5percent", data->DEFAULT_PRIORITY, sol1_5percent, data); 
    ttAttachTriggerHandler(data->in->sol1_5percent, "sol1_5percent"); 
    
    // SOLUTION 1, 1 PERCENTAGE
    ttCreateHandler("sol1_1percent", data->DEFAULT_PRIORITY, sol1_1percent, data); 
    ttAttachTriggerHandler(data->in->sol1_1percent, "sol1_1percent");
    
    // SOLUTION 2, 5 PERCENTAGE
    ttCreateHandler("sol2_5percent", data->DEFAULT_PRIORITY, sol2_5percent, data); 
    ttAttachTriggerHandler(data->in->sol2_5percent, "sol2_5percent");
    
    // SOLUTION 2, 1 PERCENTAGE
    ttCreateHandler("sol2_1percent", data->DEFAULT_PRIORITY, sol2_1percent, data); 
    ttAttachTriggerHandler(data->in->sol2_1percent, "sol2_1percent");
        
    // STOP ANTIBITOTIC
    ttCreateHandler("stop_antibiotic", data->DEFAULT_PRIORITY, stop_antibiotic, data); 
    ttAttachTriggerHandler(data->in->stop_antibiotic, "stop_antibiotic");
           
    // CRITICAL GLYCEMIA
    ttCreateHandler("critical_glycemia", data->DEFAULT_PRIORITY, critical_glycemia, data); 
    ttAttachTriggerHandler(data->in->critical_glycemia, "critical_glycemia");
        
    // AVERAGE GLYCEMIA
    ttCreateHandler("average_glycemia", data->DEFAULT_PRIORITY, average_glycemia, data); 
    ttAttachTriggerHandler(data->in->average_glycemia, "average_glycemia");
    
   // TODO: handle user input
   // =>
    
   // TODO: handle alert
   // =>
    
    // ANTIBIOTIC INJECTION
    //ttCreatePeriodicTask("antibiotic_injection", data->FIRST_INJECTION_CALL, data->antibioticPeriod, injectAnticoagulant, data);
    
   // init first injection
    ttCreateTask("first_injection", data->DEFAULT_PRIORITY, launchInjection, data); 
    ttCreateJob("first_injection");
}


void cleanup() {
   //CtrlData* data = (CtrlData*) ttGetUserData();
   // delete in, out and data
   //delete data->in;
   //delete data->out;
   //delete data;
}
