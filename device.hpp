/*
 * used to access app's content
 */

#ifndef __DEVICE_HPP__
#define __DEVICE_HPP__
#include "common.hpp"

#ifndef APP_VERSION
#define APP_VERSION "0.0.1"
#endif
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/afc.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/house_arrest.h>
#include <libimobiledevice/installation_proxy.h>
#include "plist_af.hpp"

#define HOUSE_ARREST_SERVICE_NAME "com.apple.mobile.house_arrest"
class Device {
public:
    Device(std::string="");
    Device(Device &)=delete;
    Device& operator=(const Device&)=delete;

    lockdownd_client_t & get_lockdown(){
        return lockdown;
    }
    idevice_t & get_device(){
        return device;
    }
    void read_dir(std::string);
    std::string read_file(std::string);
    void connect_to_app(std::string bundle_id);
    std::string get_all_apps_info();
    ~Device();
private:
    idevice_t device;
    lockdownd_client_t lockdown;
};
struct imobiledevice_exception : public std::exception{
    std::string content;
    imobiledevice_exception(std::string content_):content(content_){}
#ifdef __clang__
    virtual const char* what() const noexcept{return content.c_str();}
#else
    virtual const char* what() const {return content.c_str();}
#endif
};
struct AppInfo{
   std::string CFBundleIdentifier{};
   std::string CFBundleExecutable{};
   std::string CFBundleDisplayName{};
   std::string en_CFBundleDisplayName{};
   std::string cn_CFBundleDisplayName{};
   std::string CFBundleVersion{};
   std::vector<std::string> schemes{};
};
typedef std::shared_ptr<Device> Device_ptr;
class App{
    public:
        App(Device_ptr,std::string);
        App(App &)=delete;
        App& operator=(const App&)=delete;
        ~App();
        std::vector<std::string> read_dir(std::string);
        std::string read_file(std::string);
        void write_file(std::string, std::string);
        AppInfo get_AppInfo();
    private:
        std::string bundle_id;
        house_arrest_client_t house_arrest;
        afc_client_t afc_client;
        Device_ptr device_ptr;
        lockdownd_service_descriptor_t service;
};
#define ASSERT(success,err,stat,str) { \
    err = stat; \
    if(success != err)\
        throw(imobiledevice_exception(str + std::to_string(err)));\
}
#endif

