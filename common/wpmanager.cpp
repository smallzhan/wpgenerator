#include "wpmanager.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

using namespace std;
using namespace WaterPrinter;

WPManager::WPManager(const char *conf_name)
    :conf_name_(conf_name)
{
    fnames_.clear();
}

WPManager::~WPManager()
{}

bool WPManager::loadConf()
{
    ifstream conf(conf_name_.c_str());
    if (!conf.is_open()) 
    {
        cout << "no conf file " << conf_name_ << "found, exit!" << endl;
        return false;
    }
    
    string line;
    while (getline(conf, line))
    {
        size_t first = string::npos;
        if (line.size() > 3)
        {
            first = line.find_first_not_of(" \t\n");
            
            if (first == string::npos || line[first] == '#') 
            {
                continue;
            }
            else
            {
                size_t last = line.find_first_of("#");
                if (last == string::npos) 
                {
                    line = line.substr(first);
                }
                else
                {
                    line = line.substr(first, last - first);
                }
                
                size_t equal = line.find("=");
                if (equal != string::npos)
                {
                    string key = line.substr(0, equal);
                    string value = line.substr(equal + 1);
                    
                    first = value.find_first_not_of(" \t\n\r");
                    last = value.find_last_not_of(" \t\n\r");
                    value = value.substr(first, last - first + 1);
                    
 //                   printf("key = [%s], value = [%s]\n", key.c_str(), value.c_str());
                    
                    if (strncmp(key.c_str(), "wait_usb", 8) == 0)
                    {
                        if (value == "true" || value == "1" || value == "on")
                        {
                            wait_usb_ = true;
                        }
                        else if (value == "false" || value == "0" || value == "off")
                        {
                            wait_usb_ = false;
                        }
                    }
                    else if (strncmp(key.c_str(), "file_path", 9) == 0)
                    {
                        file_path_ = value;
                    }
	            else if (strncmp(key.c_str(), "usb_path", 8) == 0)
                    {
                        usb_path_  = value;
                    }
                    else if(strncmp(key.c_str(), "line_wait_time", 9) ==0)
                    {
                        line_wait_time_ = atoi(value.c_str());
                    }
                    else if (strncmp(key.c_str(), "map_wait_time", 9) == 0)
                    {
                        map_wait_time_ = atoi(value.c_str());
                    }
                    else if (strncmp(key.c_str(), "echo", 4) == 0) 
                    {
                        if (value == "1" || value == "on" || value == "true")
                        {
                            echo_ = true;
                        }
                        else if (value == "0" || value == "off" || value == "false")
                        {
                            echo_ = false;
                        }
                    }
                    else if(strncmp(key.c_str(), "files", 5) == 0)
                    {
                        value = value.substr(1, value.size() - 2);
                        size_t comma = value.find_first_of(",");
                        while (comma != string::npos)
                        {
                            string file = value.substr(0, comma);
                            value = value.substr(comma + 1);
                            first = value.find_first_not_of(" \t\n\r");
                            value = value.substr(first);
                            fnames_.push_back(file);
                            comma = value.find_first_of(",");
                        }
                        fnames_.push_back(value);
                    }
                }
            }
        }
        else
        {
            continue;
        }
    }
    return true;
}

bool WPManager::loadUSBConf()
{
    string usb_conf = usb_path_ + "files/wp.conf";
    ifstream conf(usb_conf.c_str());
    if (!conf.is_open()) 
    {
        cout << "no usb conf file found!" << endl;
        return false;
    }
    
    string line;
    while (getline(conf, line))
    {
        size_t first = string::npos;
        if (line.size() > 3)
        {
            first = line.find_first_not_of(" \t\n");
            
            if (first == string::npos || line[first] == '#') 
            {
                continue;
            }
            else
            {
                size_t last = line.find_first_of("#");
                if (last == string::npos) 
                {
                    line = line.substr(first);
                }
                else
                {
                    line = line.substr(first, last - first);
                }
                
                size_t equal = line.find("=");
                if (equal != string::npos)
                {
                    string key = line.substr(0, equal);
                    string value = line.substr(equal + 1);
                    
                    first = value.find_first_not_of(" \t\n\r");
                    last = value.find_last_not_of(" \t\n\r");
                    value = value.substr(first, last - first + 1);
                    
                    if(strncmp(key.c_str(), "files", 5) == 0)
                    {
                        value = value.substr(1, value.size() - 2);
                        size_t comma = value.find_first_of(",");
                        while (comma != string::npos)
                        {
                            string file = value.substr(0, comma);
                            value = value.substr(comma + 1);
                            first = value.find_first_not_of(" \t\n\r");
                            value = value.substr(first);
                            fnames_.push_back(file);
                            comma = value.find_first_of(",");
                        }
                        fnames_.push_back(value);
                    }
                }
            }
        }
        else
        {
            continue;
        }
    }
    return true;
}
