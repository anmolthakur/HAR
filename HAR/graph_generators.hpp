#ifndef graph_generators_hpp
#define graph_generators_hpp

#include <cmath>
#include <cstdlib>

struct GraphGenerator
{
    virtual unsigned int numSamples() = 0;
    virtual float minValue() = 0;
    virtual float maxValue() = 0;
    virtual float valueAt(int sample) = 0;
};


struct SinWaveGenerator : public GraphGenerator
{
    static constexpr float TwoPi = 2.0f * 3.14159265358979323846;
    
    SinWaveGenerator(int numcycles, int resolution, float minVal, float maxVal)
    : numCycles_(numcycles)
    , resolution_(resolution)
    , numSamples_((float)(numcycles * resolution))
    , minVal_(minVal)
    , maxVal_(maxVal)
    {
    }

    unsigned int numSamples() override { return numSamples_; }
    float minValue() override { return minVal_; }
    float maxValue() override { return maxVal_; }
    float valueAt(int sample) override
    {
        return minValue() + (maxValue() - minValue()) * 0.5f * (::sin(numCycles_ * TwoPi * sample / numSamples_) + 1.0f);
    }
    
private:
    int numCycles_, resolution_;
    float numSamples_;
    float minVal_, maxVal_;
};

struct TriangleWaveGenerator : public GraphGenerator
{
    TriangleWaveGenerator(int numSamples)
    : numSamples_(numSamples)
    {}
    
    unsigned int numSamples() override { return numSamples_; }
    float minValue() override { return 0; }
    float maxValue() override { return 10; }
    float valueAt(int sample) override
    {
        return ((sample % 2) == 0)? minValue() : maxValue();
    }
    
private:
    int numSamples_;

};

struct RandomWaveGenerator : public GraphGenerator
{
    RandomWaveGenerator(int numSamples)
    : numSamples_(numSamples)
    {
    }
    
    unsigned int numSamples() override { return numSamples_; }
    float minValue() override { return 0; }
    float maxValue() override { return 10; }
    float valueAt(int sample) override
    {
        return minValue() + (maxValue() - minValue()) * ((::rand() % 1000) / 1000.0f);
    }
    
private:
    int numSamples_;
};

#endif /* graph_generators_hpp */
