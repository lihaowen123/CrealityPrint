#ifndef slic3r_PrinterMgr_hpp_
#define slic3r_PrinterMgr_hpp_
#include <string>

#include "nlohmann/json.hpp"

namespace DM {
    class DeviceMgr {
    public:
        struct Data
        {
            int connectType;
            std::string model;
            std::string mac;
            std::string address;
            std::string name;
        };
    public:
        DeviceMgr();
        ~DeviceMgr();
        void Load();
        void Save();
        void AddDevice(std::string group, Data& data);
        void RemoveDevice(std::string name);
        void EditDeiveName(std::string name, std::string nameNew);
        void UpdateDevice(std::string address, Data& data);
        void AddGroup(std::string name, bool is_save=true);
        void RemoveGroup(std::string name);
        void EditGroupName(std::string name, std::string nameNew);
        void remove2FirstGroup(std::string group);
        void move2Group(std::string originGroup, std::string targetGroup, std::string address);
        void sortGroup(std::vector<std::string> order = {"new group2", "22", "new group"});
        void SetMergeState(bool state);
        bool IsMergeState();
        nlohmann::json GetData();
        void Get(std::map<std::string, std::vector<DeviceMgr::Data>>& store, std::vector<std::string>& order);
        bool IsGroupExist(std::string name);
        bool IsPrinterExist(std::string mac);
        std::vector<std::string> GetSamePrinter(std::string mac);
        void SetCurrentDevice(std::string mac);
        std::string GetCurrentDevice();
    private:
        struct priv;
        std::unique_ptr<priv> p;
    public:
        static DeviceMgr& Ins();
    };

}

#endif /* slic3r_Tab_hpp_ */
