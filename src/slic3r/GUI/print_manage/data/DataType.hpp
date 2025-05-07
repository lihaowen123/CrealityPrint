#ifndef DM_DataType_hpp_
#define DM_DataType_hpp_
#include "nlohmann/json.hpp"
#include <string>
namespace DM{

    struct Material
    {
        int         material_id;
        std::string vendor;
        std::string type;
        std::string name;
        std::string rfid;
        std::string color;  // #RRGGBB
        double      diameter;
        int         minTemp;
        int         maxTemp;
        double      pressure;
        int         percent;
        int         state;
        int         selected;
        int         editStatus;
        std::string userMaterial;
        bool operator==(const Material& other) const;
        bool operator!=(const Material& other) const;

        static bool compareMaterials(const DM::Material& a, const DM::Material& b);
    };

    struct MaterialBox
    {
        int box_id;
        int box_state;
        int box_type;
        int temp;
        int humidity;
        std::vector<Material> materials;

        bool operator==(const MaterialBox& other) const;
        bool operator!=(const MaterialBox& other) const;
        static bool compareMaterialBoxes(const DM::MaterialBox& a, const DM::MaterialBox& b);
        static bool findAndCompareMaterialBoxes(const std::vector<DM::MaterialBox>& boxesA, const DM::MaterialBox& boxB);
    };

    struct DeviceBoxColorInfo
    {
        int boxType = 0;  // 0: normal multi-color box,  1: extra box
        std::string color;
        int boxId;
        int materialId;
        std::string filamentType;
        std::string filamentName;
        std::string cId;
    };

    struct Device
    {
        bool valid = false;
        std::string name;
        std::string address;
        std::string mac;
        std::string model;
        std::string group;
        std::string tbId;
        std::string modelName;
        bool isMultiColorDevice=false;
        bool online=false;
        bool isCurrentDevice = false;
        bool isRelateToAccount = false;
        bool webrtcSupport = false;
        int deviceState;
        int deviceType = 0;  //0==local, 1==cx cloud,        

        std::vector<DeviceBoxColorInfo> boxColorInfos;
        std::vector<MaterialBox> materialBoxes;

        static Device deserialize(nlohmann::json& device);
    };

}
#endif /* RemotePrint_DeviceDB_hpp_ */
