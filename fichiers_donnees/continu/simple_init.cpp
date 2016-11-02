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
        double antibioticPeriod;
        double counter;
        double exectime;
        PortOut* out;
        PortIn* in;
        //SIMULATION CONST
        const bool AUTO_RESET = false;
        // time constantes
        const double ONE_DAY_IN_SECONDS = 86400;
        const double TWO_MINUTES_IN_SECONDS = 120;
        const double HEIGHT_HOURS_IN_SECONDS = 28800;
        // Const used in simulation
        const double CRITICAL_PRIORITY = 2;
        const double HIGH_PRIORITY = 10;
        const double DEFAULT_PRIORITY = 15;
        const double LOW_PRIORITY = 20;
        const double DEFAULT_PERIOD = HEIGHT_HOURS_IN_SECONDS;   
        const double FIRST_INJECTION_CALL = 1; 
};

// handler
double sol1_5percent(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data; 
    
    switch(segment) { 
        case 1:  
            std::cout << ttCurrentTime() << " : Alarm: sol 1 is at 5 percent" << std::endl;            
            return ttCurrentTime(); 
     default: 
         return FINISHED; 
     } 
 } 
// handler
double sol1_1percent(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data; 
 
    switch(segment) { 
        case 1:
            std::cout <<ttCurrentTime() << " : Alarm: sol 1 is at 1 percent" << std::endl;
            ttAnalogOut(d->out->sol1,0);                 
            d->isSol1Empty = true;
                
            if(!d->isSol2Empty || d->AUTO_RESET){                
                ttAnalogOut(d->out->sol2,1);
                d->actualSolution = 2;
            }
            else{
                std::cout <<ttCurrentTime() << " : PANIC: no more insuline" << std::endl;
            }
            
            return ttCurrentTime(); 
     default: 
         return FINISHED; 
     } 
 } 

// handler
double sol2_5percent(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data; 
 
    switch(segment) { 
        case 1:  
            std::cout <<ttCurrentTime() << " : Alarm: sol 2 is at 5 percent" << std::endl;
            return ttCurrentTime(); 
     default: 
         return FINISHED; 
     } 
 } 

// handler
double sol2_1percent(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data; 
 
    switch(segment) { 
        case 1:
            std::cout <<ttCurrentTime() << " : Alarm: sol 2 is at 1 percent" << std::endl;
            ttAnalogOut(d->out->sol2,0);            
            d->isSol2Empty = true;
                    
            if (!d->isSol1Empty || d->AUTO_RESET){
                ttAnalogOut(d->out->sol1,1);
                d->actualSolution = 1;
            }
            else{
                std::cout <<ttCurrentTime() << " : PANIC: no more insuline" << std::endl;
            }
            return ttCurrentTime(); 
     default: 
         return FINISHED; 
     } 
 } 

// handler
double stop_antibiotic(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data; 
    std::cout << ttCurrentTime() << " : Stop antibiotic" << std::endl;
    switch(segment) { 
        case 1:            
            ttAnalogOut(d->out->antibiotic,0);// stop antibiotic injection
            return ttCurrentTime(); 
     default: 
         return FINISHED; 
     } 
 } 

// handler
double inject_glucose(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data; 
    std::cout <<ttCurrentTime() << " : critical_glycemia" << std::endl;
    switch(segment) { 
        case 1:  
            // stop everything and make an urgency injection
            ttAnalogOut(d->out->sol1,0);
            ttAnalogOut(d->out->sol2,0);
            ttAnalogOut(d->out->anticoagulant,0);
            ttAnalogOut(d->out->antibiotic,0);
            ttAnalogOut(d->out->glucose_injection,1);
            return ttCurrentTime(); 
     default: 
         return FINISHED; 
     } 
 } 

// handler
double average_glycemia(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data; 
    std::cout << ttCurrentTime() <<" : Average glycemia" << std::endl;
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
            
            return ttCurrentTime(); 
     default: 
         return FINISHED; 
     } 
 } 

// handler
double launchInjection(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data; 
    std::cout <<ttCurrentTime() << " : First injection !" << std::endl;
    switch(segment) { 
        case 1:  
            ttAnalogOut(d->out->sol1,1);            
            return ttCurrentTime(); 
     default: 
         return FINISHED; 
     } 
 } 

// periodic method
double injectAnticoagulant(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data;
        
    switch(segment) { 
        case 1:
            d->counter += d->antibioticPeriod;
            if(d->counter >= d->ONE_DAY_IN_SECONDS){
                std::cout << ttCurrentTime() <<" : inject Anticoagulant !" << std::endl;
                d->counter = 0;
                ttAnalogOut(d->out->anticoagulant,1);
            }
            ttCreateTimer("stop_anticoagulant_timer",  ttCurrentTime() + 120, "stop_anticoagulant");
            
            return ttCurrentTime(); 
     default: 
         return FINISHED; 
     } 
 }

// periodic method
double stopAnticoagulant(int segment, void *data) { 
    CtrlData *d = (CtrlData *)data;
    
    switch(segment) { 
        case 1:  
            ttAnalogOut(d->out->anticoagulant,0);                        
            std::cout <<ttCurrentTime() << " : inject antibiotic" << std::endl;
            ttAnalogOut(d->out->antibiotic,1);
            
            return ttCurrentTime(); 
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
    data->counter = data->ONE_DAY_IN_SECONDS;
    data->antibioticPeriod = data->DEFAULT_PERIOD; // default period and then ask user
    
    // first time
    data->exectime = 0,1;
    
    ttInitKernel(prioFP); 
    
    // =========================== Tasks
    
    // SOLUTION 1, 5 PERCENTAGE
    ttCreateHandler("sol1_5percent", data->LOW_PRIORITY, sol1_5percent, data); 
    ttAttachTriggerHandler(data->in->sol1_5percent, "sol1_5percent"); 
    
    // SOLUTION 1, 1 PERCENTAGE
    ttCreateHandler("sol1_1percent", data->HIGH_PRIORITY, sol1_1percent, data); 
    ttAttachTriggerHandler(data->in->sol1_1percent, "sol1_1percent");
    
    // SOLUTION 2, 5 PERCENTAGE
    ttCreateHandler("sol2_5percent", data->LOW_PRIORITY, sol2_5percent, data); 
    ttAttachTriggerHandler(data->in->sol2_5percent, "sol2_5percent");
    
    // SOLUTION 2, 1 PERCENTAGE
    ttCreateHandler("sol2_1percent", data->HIGH_PRIORITY, sol2_1percent, data); 
    ttAttachTriggerHandler(data->in->sol2_1percent, "sol2_1percent");
        
    // STOP ANTIBITOTIC
    ttCreateHandler("stop_antibiotic", data->DEFAULT_PRIORITY, stop_antibiotic, data); 
    ttAttachTriggerHandler(data->in->stop_antibiotic, "stop_antibiotic");
           
    // CRITICAL GLYCEMIA
    ttCreateHandler("critical_glycemia", data->CRITICAL_PRIORITY, inject_glucose, data); 
    ttAttachTriggerHandler(data->in->critical_glycemia, "critical_glycemia");
        
    // AVERAGE GLYCEMIA
    ttCreateHandler("average_glycemia", data->HIGH_PRIORITY, average_glycemia, data); 
    ttAttachTriggerHandler(data->in->average_glycemia, "average_glycemia");    
    
    // ANTIBIOTIC INJECTION
    ttCreatePeriodicTask("antibiotic_injection", data->FIRST_INJECTION_CALL, data->antibioticPeriod, injectAnticoagulant, data);
    ttSetPriority(data->DEFAULT_PRIORITY, "antibiotic_injection");
    
    //STOP ANTICOAGULANT
    ttCreateHandler("stop_anticoagulant", data->DEFAULT_PRIORITY, stopAnticoagulant, data);
   
    // init first injection
    ttCreateTask("first_injection", data->DEFAULT_PRIORITY, launchInjection, data); 
    ttCreateJob("first_injection");
}


void cleanup() {
   CtrlData* data = (CtrlData*) ttGetUserData();
   // delete in, out and data
   //delete data->in;
   //delete data->out;
   delete data;
}
