#ifndef _WPMANAGER_H_
#define _WPMANAGER_H_

#include <string>
#include <vector>

namespace WaterPrinter
{
    
    class WPManager
    {
    public:
        WPManager(const char* confile);
        ~WPManager();

        bool loadConf();
        bool loadUSBConf();
        //void Run();
    
    public:
        std::string file_path_;
        bool wait_usb_;
    
        std::vector<std::string> fnames_;
        
        int line_wait_time_;
        int map_wait_time_;
        
        bool echo_;
        std::string conf_name_;
        std::string usb_path_;
    };
}


#endif /* _WPMANAGER_H_ */
