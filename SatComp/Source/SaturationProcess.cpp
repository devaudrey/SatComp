/*
  ==============================================================================

    SaturationProcess.cpp
    Created: 22 Feb 2024 3:05:46pm
    Author:  audrey dawson

  ==============================================================================
*/

#include "SaturationProcess.h"

void SaturationProcess::prepareToPlay(double sampleRate) {
    
    if (Fs != (float) sampleRate) {
        Fs = (float) sampleRate;
    }
    
}

// –––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––

float SaturationProcess::processSample(float x) {
    
    float y = x;
    float outputTrim_lin = (outputTrim_dB == 0.f) ? 0.f : pow(10.0f, outputTrim_dB * 0.05f);
    
    
    if (x != 0.f) {
       
        // apply drive amount
        y *= abs(pow(10.0f, drive_dB * 0.05f));
        
        // saturate the signal
        // (2/pi) * atan(drive * x) // << THE EQUATION

        y = _twoOverPi * atan(y);
        
        // convert to linear & apply to signal
        if (x * -1 < 0) {
            y += outputTrim_lin;
        }
        else {
            y -= outputTrim_lin;
        }
    }
    
    // return processed signal
    return y;
    
}

void SaturationProcess::process(float * buffer, int numSamples, int channel) {
    
    for (int n = 0; n < numSamples; n++) {
        
        float x = buffer[n];
        buffer[n] = processSample(x);
        
    }
    
}

// –––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––

void SaturationProcess::setDriveAmount(float drive) {
    
    drive_dB = drive;
    
}

void SaturationProcess::setOutputTrim(float outputGainReduction) {
    
    outputTrim_dB = outputGainReduction;
    
}
