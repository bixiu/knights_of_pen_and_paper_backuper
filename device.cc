#include "device.hpp"
#include <tuple>
#include <iostream>
Device::~Device(){
    lockdownd_client_free(lockdown);
    idevice_free(device);
}
Device::Device(std::string udid){
    idevice_error_t idevice_err;
    lockdown = nullptr;
    device = nullptr;
    if(udid == ""){
        idevice_err = idevice_new(&device,NULL);
    }else
        idevice_err = idevice_new(&device,udid.c_str());
    if (IDEVICE_E_SUCCESS != idevice_err)
        throw imobiledevice_exception("No iPhone found, is it plugged in?, " + std::to_string(idevice_err));
    lockdownd_error_t lockdownd_err;
    lockdownd_err = lockdownd_client_new_with_handshake(device, &lockdown, "appwill");
    if (LOCKDOWN_E_PASSWORD_PROTECTED == lockdownd_err)
        throw(imobiledevice_exception(std::string("password protected, ") + std::to_string(lockdownd_err)));
    if (LOCKDOWN_E_SUCCESS != lockdownd_err)
        throw(imobiledevice_exception(std::string("handshake error, ") + std::to_string(lockdownd_err)));
}
std::string Device::get_all_apps_info(){
    plist_t client_opts = instproxy_client_options_new();
    instproxy_client_options_add(client_opts, "ApplicationType", "User", NULL);
    instproxy_error_t err;
    plist_t apps = NULL;
    instproxy_client_t ipc = NULL;
    lockdownd_service_descriptor_t service = NULL;
    lockdownd_client_t client = get_lockdown();
    lockdownd_error_t lockdownd_err;
    ASSERT(LOCKDOWN_E_SUCCESS,lockdownd_err,lockdownd_start_service(client, "com.apple.mobile.installation_proxy",
          &service),std::string("failed to start installation_proxy service, "));
    ASSERT(INSTPROXY_E_SUCCESS,err,instproxy_client_new(get_device(), service, &ipc),std::string("failed to create new instproxy client, "));
    err = instproxy_browse(ipc, client_opts, &apps);
    instproxy_client_options_free(client_opts);
    if (err != INSTPROXY_E_SUCCESS)
        throw(imobiledevice_exception(std::string("ERROR: instproxy_browse returned ") + std::to_string(err)));
    if (!apps || (plist_get_node_type(apps) != PLIST_ARRAY))
        throw(imobiledevice_exception(std::string("ERROR: instproxy_browse returnd an invalid plist!")));
    char * bin=NULL;
    uint32_t bin_length;
    plist_to_xml(apps,&bin,&bin_length);
    std::string bin_{bin,bin_length};
    free(bin);
    plist_free(apps);
    instproxy_client_free(ipc);
    return bin_;
}


App::~App(){
    afc_client_free(afc_client);
    house_arrest_client_free(house_arrest);
}

App::App(Device_ptr device, std::string bundle_id){
    this->bundle_id = bundle_id;
    lockdownd_client_t & lockdown = device->get_lockdown();
    lockdownd_error_t lockdownd_err;
    service = nullptr;
    house_arrest = nullptr;
    afc_client = nullptr;
    lockdownd_err = lockdownd_start_service(lockdown, HOUSE_ARREST_SERVICE_NAME, &service);
    if (LOCKDOWN_E_SUCCESS != lockdownd_err)
        throw(imobiledevice_exception(std::string("can't start service") + std::to_string(lockdownd_err)));
    house_arrest_error_t house_arrest_err;
    house_arrest_err = house_arrest_client_new(device->get_device(), service, &house_arrest);
    if(HOUSE_ARREST_E_SUCCESS != house_arrest_err)
        throw(imobiledevice_exception(std::string("can't create new house arrest client, ") + std::to_string(house_arrest_err)));
    house_arrest_err = house_arrest_send_command(house_arrest, "VendDocuments", bundle_id.c_str());
    plist_t dict=nullptr;
    house_arrest_err = house_arrest_get_result(house_arrest, &dict);
    if (HOUSE_ARREST_E_SUCCESS != house_arrest_err)
        throw(imobiledevice_exception(std::string("can't get house arrest result, ") + std::to_string(house_arrest_err)));
    char * str = nullptr;
    plist_t node = plist_dict_get_item(dict, "Error");
    if (node){
        plist_get_string_val(node, &str);
        if (str){
            std::string str_(str);
            free(str);
            plist_free(dict);
            dict = nullptr;
            throw(imobiledevice_exception(std::string("house arrest command error, ") + str_));
        }
    }
    if(dict)
        plist_free(dict);
    afc_error_t afc_err;
    afc_err = afc_client_new_from_house_arrest_client(house_arrest, &afc_client);
    if(AFC_E_SUCCESS != afc_err)
        throw(imobiledevice_exception(std::string("can't create new afc client from house arrest client") + std::to_string(afc_err)));

}

std::vector<std::string> App::read_dir(std::string path){
    std::vector<std::string> dir;
    afc_error_t afc_err;
    char ** list = NULL;
    afc_err = afc_read_directory(afc_client, path.c_str(), &list);
    if (AFC_E_SUCCESS != afc_err){
        throw(imobiledevice_exception(std::string("failed to read path ") + path + ", " + std::to_string(afc_err)));
    }
    for(int i=0;list[i] != NULL and std::string(list[i])!="";++i)
        dir.push_back(std::string(list[i]));
    return dir;
}

std::string App::read_file(std::string path){
    afc_error_t afc_err;
    uint64_t handle;
    std::ostringstream out;
    ASSERT(AFC_E_SUCCESS,afc_err,afc_file_open(afc_client, path.c_str(), AFC_FOPEN_RDONLY, &handle),std::string("failed to open file, "))
    char buf[81920];
    uint32_t bytes_read;
    do {
        ASSERT(AFC_E_SUCCESS, afc_err, afc_file_read(afc_client, handle, buf, sizeof(buf), &bytes_read), "failed to read file content, ");
        out << std::string(buf,bytes_read);
    }while(bytes_read > 0);
    return out.str();
}
void App::write_file(std::string path, std::string content){
    afc_error_t afc_err;
    uint64_t handle;
    ASSERT(AFC_E_SUCCESS,afc_err,afc_file_open(afc_client, path.c_str(), AFC_FOPEN_WRONLY, &handle),std::string("failed to open file, "))
    uint32_t bytes_written;
    ASSERT(AFC_E_SUCCESS, afc_err, afc_file_write(afc_client, handle, content.c_str(), content.length(), &bytes_written), "failed to read file content, ");
    if(bytes_written != content.length())
        throw imobiledevice_exception("write content to file failed");
}
