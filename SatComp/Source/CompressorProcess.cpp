/*
  ==============================================================================

    CompressorProcess.cpp
    Created: 22 Feb 2024 3:06:16pm
    Author:  audrey dawson

  ==============================================================================
*/

#include "CompressorProcess.h"

void CompressorProcess::prepareToPlay(double sampleRate) {
    
    if (Fs != sampleRate) {
        Fs = sampleRate;
    }
    
    initVariables();
}

void CompressorProcess::initVariables(){
    
    setAttack(30.f);
    setRelease(1000.f);
    setRatio(3);
    setKneeWidth(2.f);
    setThreshold(0.f);
    setMakeupGain(0.f);
}

float CompressorProcess::processSample(float x, int channel) {

    // convert the previous sample to dB
    
    outputPrevious[channel] = convert_dB(abs(outputPrevious[channel]));
    
    
    
    // static gain CPU – calculates compression amount
    
    if (outputPrevious[channel] > threshold + kneeWidth/2.f) {
        gainSC = threshold + ((outputPrevious[channel] - threshold)/(float)ratio);
    }
    
    else if ((outputPrevious[channel] >= threshold - kneeWidth/2.f)&&(outputPrevious[channel] <= threshold + kneeWidth/2.f)) {
        gainSC = outputPrevious[channel] +
            ((1.f / (float)ratio - 1.f) * pow(outputPrevious[channel] - threshold + kneeWidth/2.f,2.f))/(2.f * kneeWidth);
    }
    else {
        gainSC = outputPrevious[channel];
    }
    
    
    
    // calculates gain change for the upcoming smoothing algorithm
    
    gainChange_dB = gainSC - outputPrevious[channel];
    
    
    
    // gain smoothing algorithm
    
    if (gainChange_dB < gainSmoothPrev[channel])
        gainSmooth = ((1 - alphaA) * gainChange_dB) + (alphaA * gainSmoothPrev[channel]);
    else {
        gainSmooth = ((1 - alphaR) * gainChange_dB) + (alphaR * gainSmoothPrev[channel]);
    }
    
    // calculate linear gainSmooth and the final output signal (pre-phase check)
    
    float gainSmoothLin = convert_lin(gainSmooth);
    float outputSignal = convert_lin(convert_dB(gainSmoothLin * x) + makeupGain);
    
    // phase check to make sure output signal is the correct sign (+/-) based off of the input signal
    
    outputSignal *= (x != 0.f) ? x/abs(x) : 1.f;
    
    // assign previous values and return
    
    outputPrevious[channel] = outputSignal;
    gainSmoothPrev[channel] = gainSmooth;
    
    return outputSignal;
}

void CompressorProcess::process(float *buffer, int numSamples, int channel) {
    
    for (int n = 0; n < numSamples; n++) {
        
        float x = buffer[n];
        buffer[n] = processSample(x, channel);
    }
}

float CompressorProcess::convert_dB(float sample_lin) {
    
    // takes a linear audio signal sample and converts it to the dB scale

    float sample_dB = 20.f * log10(abs(sample_lin) / 1.f);

    // no negative infinity values
    if (sample_dB < -96.f) {
        sample_dB = -96.f;
    }
    
    return sample_dB;
}

float CompressorProcess::convert_lin(float sample_dB) {
    
    // takes a dB audio signal sample and converts it to the linear scale for output
    
    float sample_lin = pow(10.0f, sample_dB * 0.05f);

    return sample_lin;
}


void CompressorProcess::setAttack(float userAttack) {
    
    // Attack & release times are passed in milliseconds then converted to seconds.
    
    attack = userAttack;
    userAttack /= 1000.f; // convert to seconds
    alphaA = exp((-log(9.f))/(Fs * userAttack));
    
}

void CompressorProcess::setRelease(float userRelease) {
    
    // Attack & release times are passed in milliseconds then converted to seconds.
    
    release = userRelease;
    userRelease /= 1000.f; // convert to seconds
    alphaR = exp((-log(9.f))/(Fs * userRelease));
    
}

void CompressorProcess::setRatio(int userRatio) {
    
    ratio = userRatio;
}

void CompressorProcess::setKneeWidth(float userKnee) {
    
    kneeWidth = userKnee;
}

void CompressorProcess::setThreshold(float userThreshold) {
    
    threshold = userThreshold;
}

void CompressorProcess::setMakeupGain(float userMakeupGain) {
    
    makeupGain = userMakeupGain;
    
}
