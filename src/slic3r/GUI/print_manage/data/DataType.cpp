#include "DataType.hpp"
namespace DM{
 
    bool DM::Material::operator==(const DM::Material& other) const
    {
        return  type == other.type && color == other.color;
    }

    // only used for compare in the param filament panel update
    bool DM::Material::operator!=(const DM::Material& other) const
    {
        return !(*this == other);
    }

    // only used for compare in the param filament panel update
    bool DM::MaterialBox::operator==(const DM::MaterialBox& other) const
    {
        return box_id == other.box_id && materials == other.materials;
    }

    // only used for compare in the param filament panel update
    bool DM::MaterialBox::operator!=(const DM::MaterialBox& other) const
    {
        return !(*this == other);
    }


    DM::Device Device::deserialize(nlohmann::json& device)
    {
        DM::Device data;
        try{
            if (!device.empty())
            {
                data.valid = true;

                data.mac = device.contains("mac") ? device["mac"].get<std::string>() : "";
                data.address = device.contains("address") ? device["address"].get<std::string>() : "";
                data.model = device.contains("model") ? device["model"].get<std::string>() : "";
                data.online = device.contains("online") ? device["online"].get<bool>() : false;
                data.deviceState = device.contains("deviceState") ? device["deviceState"].get<int>() : 0;
                data.name = device.contains("name") ? device["name"].get<std::string>() : "";

                data.deviceType = device.contains("deviceType") ? device["deviceType"].get<int>() : 0;
                data.isCurrentDevice = device.contains("isCurrentDevice") ? device["isCurrentDevice"].get<bool>() : false;
                data.webrtcSupport = device.contains("webrtcSupport") ? device["webrtcSupport"].get<int>() == 1 : false;
                data.tbId = (device.contains("tbId") && !device["tbId"].is_null()) ? device["tbId"].get<std::string>() : "";
                data.modelName = device.contains("modelName") ? device["modelName"].get<std::string>() : "";
                data.isMultiColorDevice = device.contains("IsMultiColorDevice") ? device["IsMultiColorDevice"].get<bool>() : false;

                if (device.contains("boxsInfo") && device["boxsInfo"].contains("boxColorInfo")) {
                    for (const auto& box_info : device["boxsInfo"]["boxColorInfo"]) {
                        DM::DeviceBoxColorInfo box_color_info;
                        box_color_info.boxType = box_info["boxType"].get<int>();
                        box_color_info.color = box_info["color"].get<std::string>();
                        box_color_info.boxId = box_info["boxId"].get<int>();
                        box_color_info.materialId = box_info["materialId"].get<int>();
                        box_color_info.filamentType = box_info["filamentType"].get<std::string>();
                        box_color_info.filamentName = box_info["filamentName"].get<std::string>();
                        if (box_info.contains("cId")) {
                            box_color_info.cId = box_info["cId"].get<std::string>();
                        }
                        data.boxColorInfos.push_back(box_color_info);
                    }
                }

                if (device.contains("boxsInfo") && device["boxsInfo"].contains("materialBoxs")) {
                    auto& materialBoxs = device["boxsInfo"]["materialBoxs"];

                    for (const auto& box : materialBoxs) {
                        DM::MaterialBox materialBox;
                        materialBox.box_id = box.contains("id") ? box["id"].get<int>() : 0;
                        materialBox.box_state = box.contains("state") ? box["state"].get<int>() : 0;
                        materialBox.box_type = box.contains("type") ? box["type"].get<int>() : 0;
                        if (box.contains("temp")) {
                            materialBox.temp = box["temp"];
                        }
                        if (box.contains("humidity")) {
                            materialBox.humidity = box["humidity"];
                        }

                        if (box.contains("materials")) {
                            for (const auto& material : box["materials"]) {
                                DM::Material mat;
                                mat.material_id = material.contains("id") ? material["id"].get<int>() : 0;
                                mat.vendor = material.contains("vendor") ? material["vendor"].get<std::string>() : "";
                                mat.type = material.contains("type") ? material["type"].get<std::string>() : "";
                                mat.name = material.contains("name") ? material["name"].get<std::string>() : "";
                                mat.rfid = material.contains("rfid") ? material["rfid"].get<std::string>() : "";
                                mat.color = material.contains("color") ? material["color"].get<std::string>() : "";
                                mat.diameter = material.contains("diameter") ? material["diameter"].get<double>() : 0.0;
                                mat.minTemp = material.contains("minTemp") ? material["minTemp"].get<int>() : 0;
                                mat.maxTemp = material.contains("maxTemp") ? material["maxTemp"].get<int>() : 0;
                                mat.pressure = material.contains("pressure") ? material["pressure"].get<double>() : 0.0;
                                mat.percent = material.contains("percent") ? material["percent"].get<int>() : 0;
                                mat.state = material.contains("state") ? material["state"].get<int>() : 0;
                                mat.selected = material.contains("selected") ? material["selected"].get<int>() : 0;
                                mat.editStatus = material.contains("editStatus") ? material["editStatus"].get<int>() : 0;
                                mat.userMaterial = material.contains("userMaterial") ? material["userMaterial"].get<std::string>() : "";
                                materialBox.materials.push_back(mat);
                            }
                        }

                        data.materialBoxes.push_back(materialBox);
                    }
                }

            }
        }
        catch (std::exception &e) {
            BOOST_LOG_TRIVIAL(error) << "DM::Device Device::deserialize;Error:" << e.what();
        }
        
        return data;
    }

    bool Material::compareMaterials(const DM::Material& a, const DM::Material& b)
    {
        return a.type == b.type && a.color == b.color && a.state == b.state && a.editStatus == b.editStatus;
    }

    bool MaterialBox::compareMaterialBoxes(const DM::MaterialBox& a, const DM::MaterialBox& b)
    {
        if (a.box_id != b.box_id || a.box_type != b.box_type)
        {
            return false;
        }

        if (a.materials.size() != b.materials.size()) {
            return false;
        }

        for (const auto& materialA : a.materials) {
            auto it = std::find_if(b.materials.begin(), b.materials.end(),
                [&materialA](const DM::Material& materialB) {
                    return materialA.material_id == materialB.material_id;
                });

            if (it == b.materials.end() || !Material::compareMaterials(materialA, *it)) {
                return false;
            }
        }

        return true;
    }

    bool MaterialBox::findAndCompareMaterialBoxes(const std::vector<DM::MaterialBox>& boxesA, const DM::MaterialBox& boxB)
    {
        auto it = std::find_if(boxesA.begin(), boxesA.end(),
            [&boxB](const DM::MaterialBox& boxA) {
                return boxA.box_id == boxB.box_id && boxA.box_type == boxB.box_type;
            });

        if (it == boxesA.end()) {
            return false;
        }

        return compareMaterialBoxes(*it, boxB);
    }

}