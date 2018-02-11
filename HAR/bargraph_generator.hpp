#ifndef bargraph_generator_hpp
#define bargraph_generator_hpp

#include <cstdlib>
#include <chrono>


struct BarGraphGenerator
{
    virtual void update() = 0;
    
    virtual unsigned int numBars() = 0;
    
    /** Return value in range 0 to 100 */
    virtual float valueAtBar(int bar) = 0;
};


struct TestBarGraphGenerator : public BarGraphGenerator
{
    TestBarGraphGenerator()
    {
        auto now = std::chrono::system_clock::now();
        for (int k = 0; k < 6; ++k)
        {
            values_[k] = genRandomValue(100.0f);
            timeToRefreshValue_[k] = now + std::chrono::milliseconds(genRandomValue<long>(1000));
        }
    }
    
    unsigned int numBars() override { return 6; }
    
    /** Return value in range 0 to 100 */
    float valueAtBar(int bar) override
    {
        return values_[bar];
    }
    
    void update() override
    {
        auto now = std::chrono::system_clock::now();
        for (int k = 0; k < 6; ++k)
        {
            if (now > timeToRefreshValue_[k])
            {
                values_[k] = genRandomValue(100.0f);
                timeToRefreshValue_[k] = now + std::chrono::milliseconds(genRandomValue<long>(1000));
            }
        }
    }
    
private:
    template <typename T>
    T genRandomValue(T range) { return (T)(range * ((rand() % 1000) / 1000.0f)); }
    
    float values_[6];
    
    using time_pt = decltype(std::chrono::system_clock::now());
    time_pt timeToRefreshValue_[6];
    
};

#endif /* bargraph_generator_hpp */
