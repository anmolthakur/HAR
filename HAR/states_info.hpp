#ifndef StatesInfo_hpp
#define StatesInfo_hpp

#include <cstdlib>
#include <chrono>

struct StatesInfo
{
    virtual unsigned int numStates() = 0;
    virtual bool isStateActive(int state) = 0;
};


struct TestStatesInfo : public StatesInfo
{
    TestStatesInfo()
    {
        updateStates();
    }
    
    unsigned int numStates() override { return 19; }
    bool isStateActive(int state) override { return states_[state]; }
    
    void update()
    {
        auto now = std::chrono::system_clock::now();
        if (now > nextUpdateTime)
        {
            updateStates();
        }
    }
    
private:
    void updateStates()
    {
        for (int k = 0; k < 19; ++k)
        {
            states_[k] = getRandomState();
        }
        nextUpdateTime = std::chrono::system_clock::now() + std::chrono::milliseconds((long)(100 + rand() % 2000));
    }
    
    bool getRandomState() { return rand() % 2? true : false; }
    bool states_[19];
    
    using time_pt = std::chrono::system_clock::time_point;
    time_pt nextUpdateTime;
};

#endif /* StatesInfo_hpp */
