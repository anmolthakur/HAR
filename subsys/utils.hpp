#ifndef utils_hpp
#define utils_hpp

#include <fstream>
#include <time.h>
#include <string>
#include <sstream>
#include <boost/filesystem.hpp>

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
        
        OutputDir = "./output/";
        OutputDir += PostFix;
        
        boost::filesystem::create_directory(OutputDir);
        OutputDir += "/";
    }
    
    static std::string CreateCSVFilename(const std::string &basename)
    {
        return OutputDir + basename + CsvExtension;
    }
    
    
    class ScopedFileStreamForAppend
    {
    public:
        ScopedFileStreamForAppend(const std::string &basename, const std::string &header = "")
        {
            auto filename = OutputData::CreateCSVFilename(basename);

            bool bFirst = false;
            if (!boost::filesystem::exists(filename))
            {
                bFirst = true;
            }
            
            x_file.open(filename.c_str(), std::ios::app);
            
            if (bFirst && !header.empty())
            {
                x_file << header;
            }
        }
        ~ScopedFileStreamForAppend()
        {
            x_file.close();
        }
        
        std::ofstream &GetStream() { return x_file; }
        
    private:
        std::ofstream x_file;
    };
    
    
    static const std::string &GetOutputDir() { return OutputDir; }
    
private:
    static std::string PostFix;
    static std::string CsvExtension;
    
    static std::string OutputDir;
};

#endif /* utils_hpp */
