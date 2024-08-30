#ifndef _PPM_H
#define _PPM_H

#include "Arduino.h"
#include "driver/timer.h"
#include "driver/gpio.h"

/* HW configuration struct */
typedef struct
{
    uint8_t         ppm_p;         //step signal gpio
    uint8_t         channel;          //dir signal gpio
    timer_group_t   timer_group;    //timer group, useful if we are controlling more than 2 steppers
    timer_idx_t     timer_idx;      //timer index, useful if we are controlling 2steppers
} PPM_config_t;


class PPMencoder
{
    public:
    

    private:
};



#endif

