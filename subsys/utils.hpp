#ifndef utils_hpp
#define utils_hpp

#include <fstream>
#include <time.h>
#include <string>
#include <sstream>

// Functions
//
unsigned int getClosestPowerOfTwo(unsigned int n);


// Classes
//
class OutputData
{
public:
    static void Init()
    {
        time_t t = time(0);   // get time now
        struct tm * now = localtime( & t );
        
        std::stringstream ss;
        ss << (now->tm_year + 1900) << '-'
        << (now->tm_mon + 1) << '-'
        <<  now->tm_mday << "_"
        << now->tm_hour << '-'
        << now->tm_min << '-'
        << now->tm_sec;
        PostFix = ss.str();
        
        CsvExtension = ".csv";
    }
    
    static std::string CreateCSVFilename(const std::string &basename)
    {
        return std::string("") + basename + "_" + PostFix + CsvExtension;
    }
    
    
    class ScopedFileStreamForAppend
    {
    public:
        ScopedFileStreamForAppend(const std::string &basename)
        {
            x_file.open(OutputData::CreateCSVFilename(basename).c_str(), std::ios::app);
        }
        ~ScopedFileStreamForAppend()
        {
            x_file.close();
        }
        
        std::ofstream &GetStream() { return x_file; }
        
    private:
        std::ofstream x_file;
    };
    
    
private:
    static std::string PostFix;
    static std::string CsvExtension;
};

#endif /* utils_hpp */
