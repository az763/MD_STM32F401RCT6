#ifndef __FIR_HIGHPASS_BESSEL_H
#define __FIR_HIGHPASS_BESSEL_H

/**
  * FIR 3 Taps High Pass Filter Bessel by alek76
  * file fir_highpass_bessel.h
  * date 18.08.2024
*/

#ifdef __cplusplus
extern "C" {
#endif
//start code
#include "main.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

// Filter 4 Hz
/**************************************************************
WinFilter version 0.8
http://www.winfilter.20m.com
akundert@hotmail.com

Filter type: High Pass
Filter model: Bessel
Filter order: 2
Sampling Frequency: 100 Hz
Cut Frequency: 4.000000 Hz
Coefficents Quantization: float

Z domain Zeros
z = 1.000000 + j 0.000000
z = 1.000000 + j 0.000000

Z domain Poles
z = 0.796944 + j -0.102310
z = 0.796944 + j 0.102310
***************************************************************/
#define Ntap 3

float fir_highpass_bessel_4hz(float NewSample) {
    float FIRCoef[Ntap] = { 
        -0.07939861231338674100,
        0.84120277537322652000,
        -0.07939861231338674100
    };

    static float x[Ntap]; //input samples
    float y=0;            //output sample
    int n;

    //shift the old samples
    for(n=Ntap-1; n>0; n--)
       x[n] = x[n-1];

    //Calculate the new output
    x[0] = NewSample;
    for(n=0; n<Ntap; n++)
        y += FIRCoef[n] * x[n];
    
    return y;
}


// Filter 8 Hz
/**************************************************************
WinFilter version 0.8
http://www.winfilter.20m.com
akundert@hotmail.com

Filter type: High Pass
Filter model: Bessel
Filter order: 2
Sampling Frequency: 100 Hz
Cut Frequency: 8.000000 Hz
Coefficents Quantization: float

Z domain Zeros
z = 1.000000 + j 0.000000
z = 1.000000 + j 0.000000

Z domain Poles
z = 0.618332 + j -0.169965
z = 0.618332 + j 0.169965
***************************************************************/
//#define Ntap 3

float fir_highpass_bessel_8hz(float NewSample) {
    float FIRCoef[Ntap] = { 
        -0.13186623557371327000,
        0.73626752885257341000,
        -0.13186623557371327000
    };

    static float x[Ntap]; //input samples
    float y=0;            //output sample
    int n;

    //shift the old samples
    for(n=Ntap-1; n>0; n--)
       x[n] = x[n-1];

    //Calculate the new output
    x[0] = NewSample;
    for(n=0; n<Ntap; n++)
        y += FIRCoef[n] * x[n];
    
    return y;
}


// Filter 12 Hz
/**************************************************************
WinFilter version 0.8
http://www.winfilter.20m.com
akundert@hotmail.com

Filter type: High Pass
Filter model: Bessel
Filter order: 2
Sampling Frequency: 100 Hz
Cut Frequency: 12.000000 Hz
Coefficents Quantization: float

Z domain Zeros
z = 1.000000 + j 0.000000
z = 1.000000 + j 0.000000

Z domain Poles
z = 0.457655 + j -0.214883
z = 0.457655 + j 0.214883
***************************************************************/
//#define Ntap 3

float fir_highpass_bessel_12hz(float NewSample) {
    float FIRCoef[Ntap] = { 
        -0.16951583564071887000,
        0.66096832871856237000,
        -0.16951583564071887000
    };

    static float x[Ntap]; //input samples
    float y=0;            //output sample
    int n;

    //shift the old samples
    for(n=Ntap-1; n>0; n--)
       x[n] = x[n-1];

    //Calculate the new output
    x[0] = NewSample;
    for(n=0; n<Ntap; n++)
        y += FIRCoef[n] * x[n];
    
    return y;
}


// end code  
#ifdef __cplusplus
}
#endif

#endif /* __FIR_HIGHPASS_BESSEL_H */
