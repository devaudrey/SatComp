/*
  ==============================================================================

    CompressorProcess.h
    Created: 22 Feb 2024 3:06:16pm
    Author:  audrey dawson

  ==============================================================================
*/

#pragma once
#include <math.h>



class CompressorProcess {
    
public:
    
    void prepareToPlay(double sampleRate);
    void initVariables();
    
    float processSample(float x, int channel);
    void process(float * buffer, int numSamples, int channel);
      
//    void prepareForNextLoop(float prevOutputSample_dB, float prevGainSmooth);
    
    float convert_dB(float sample_lin);
    float convert_lin (float sample_dB);
    
    void setAttack(float userAttack);
    void setRelease(float userRelease);
    void setRatio(int userRatio);
    void setKneeWidth(float userKnee);
    void setThreshold(float userThreshold);
    void setMakeupGain(float userMakeupGain);
    
    
    
private:
    
    double Fs;
    
    float attack;
    float release;
    float alphaA;
    float alphaR;
    
    float threshold;
    int ratio;
    float kneeWidth;
    float makeupGain;
    
    float gainSC;
    float gainChange_dB;
    float gainSmooth = 0.f;
    
    float gainSmoothPrev[2] = {0.f};
    float outputPrevious[2] = {0.f};
    
};

