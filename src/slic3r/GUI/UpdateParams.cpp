#include "UpdateParams.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "GUI.hpp"
#include "GUI_App.hpp"
#include "Plater.hpp"
#include "NotificationManager.hpp"
#include "MsgDialog.hpp"

namespace Slic3r {
namespace GUI {

int nsUpdateParams::CXCloud::getNeedUpdatePrintes(std::vector<std::string>& vtNeedUpdatePrinter)
{
    vtNeedUpdatePrinter.clear();

    if (m_mapLocalPrinterVersion.empty()) {
        loadLocalPrinter();
    }
    if (m_bHasRequestedCXCloud) {
        for (auto& item : m_mapRemotePrinterVersion) {
            auto iter = m_mapLocalPrinterVersion.find(item.first);
            if (iter != m_mapLocalPrinterVersion.end() && item.second > iter->second) {
                vtNeedUpdatePrinter.push_back(item.first);
            }
        }
        return 0;
    }

    m_mapRemotePrinterVersion.clear();
    int         nRet                  = -1;
    std::string version               = "3.0.0"; // preset_bundle->get_vendor_profile_version(PresetBundle::BBL_BUNDLE).to_string();
    std::string version_type          = get_vertion_type();
    std::string base_url              = get_cloud_api_url();
    auto        preupload_profile_url = "/api/cxy/v2/slice/profile/official/printerList";
    Http::set_extra_headers(wxGetApp().get_extra_header());
    Http http = Http::post(base_url + preupload_profile_url);

    BOOST_LOG_TRIVIAL(warning) << "CXCloud getNeedUpdatePrintes [" << base_url << preupload_profile_url << "]";

    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    json               j;
    j["version"] = version;

    http.header("Content-Type", "application/json")
        .header("__CXY_REQUESTID_", to_string(uuid))
        .set_post_body(j.dump())
        .on_complete([&](std::string body, unsigned status) {
            json j = json::parse(body);
            if (j["code"] != 0) {
                nRet = 1; // ÇëÇóÊ§°Ü

                BOOST_LOG_TRIVIAL(error) << "CXCloud getNeedUpdatePrintes fail.code=" << j["code"];
                return;
            }
            if (j.contains("result") && j["result"].contains("printerList") && j["result"]["printerList"].is_array()) {
                for (auto& printer : j["result"]["printerList"]) {
                    std::string printerName = "";
                    if (printer.contains("nozzleDiameter") && printer["nozzleDiameter"].is_array()) {
                        for (auto& nozzle : printer["nozzleDiameter"]) {
                            printerName = "Creality " + printer["name"].get<std::string>() + " " + nozzle.get<std::string>() + " nozzle";
                            std::string version = printer["showVersion"].get<std::string>();
                            m_mapRemotePrinterVersion[printerName] = version;
                            auto iter = m_mapLocalPrinterVersion.find(printerName);
                            if (iter != m_mapLocalPrinterVersion.end()) {
                                if (version > iter->second) {
                                    vtNeedUpdatePrinter.push_back(printerName);
                                }
                            }

                        }
                    }
                }
            }
            m_bHasRequestedCXCloud = true;
            nRet = 0;
        })
        .on_error([&](std::string body, std::string error, unsigned status) {
            BOOST_LOG_TRIVIAL(error) << "CXCloud getNeedUpdatePrintes fail.err=" << error << ",status=" << status;
        })
        .on_progress([&](Http::Progress progress, bool& cancel) {

        })
        .perform_sync();

    return nRet;
}

void nsUpdateParams::CXCloud::clearLocalPrinter() {
    m_mapLocalPrinterVersion.clear();
}

int nsUpdateParams::CXCloud::loadLocalPrinter() {

    m_mapLocalPrinterVersion.clear();

    //auto cache_file = fs::path(data_dir()).append("profile_version.json");
    //if (!fs::exists(cache_file)) {
    //    auto src = fs::path(resources_dir()).append("profiles").append("Creality").append("profile_version.json");
    //    fs::copy_file(src, cache_file);
    //}
    auto cache_file = fs::path(data_dir()).append("system").append("Creality").append("profile_version.json");
    json cache_json;
    if (fs::exists(cache_file)) {
        boost::nowide::ifstream ifs(cache_file.string());
        ifs >> cache_json;
    }
    if (cache_json.contains("Creality") && cache_json["Creality"].is_array()) {
        for (auto& item : cache_json["Creality"]) {
            m_mapLocalPrinterVersion[item["name"].get<std::string>() + " " + item["nozzleDiameter"][0].get<std::string>() + " nozzle"] 
                = item["showVersion"].get<std::string>();
        }
    }

    return 0;
}

UpdateParams::UpdateParams() {}

UpdateParams& UpdateParams::getInstance()
{
    static UpdateParams instance;
    return instance;
}

void UpdateParams::checkParamsNeedUpdate()
{
    if (m_bHasUpdateParams) {
        m_CXCloud.clearLocalPrinter();
        m_vtNeedUpdatePrinter.clear();
        m_bHasUpdateParams = false;
    }

    if (m_vtNeedUpdatePrinter.empty()) {
        m_CXCloud.getNeedUpdatePrintes(m_vtNeedUpdatePrinter);
    }

    std::vector<std::string> vtCurPrinter;
    getCurrentPrinter(vtCurPrinter);

    bool hasPrinterNeedUpdate = false;
    for (auto item : vtCurPrinter) {
        if (std::find(m_vtNeedUpdatePrinter.begin(), m_vtNeedUpdatePrinter.end(), item) != m_vtNeedUpdatePrinter.end()) {
            hasPrinterNeedUpdate = true;
            break;
        }
    }
    if (hasPrinterNeedUpdate) {
        std::string ssTip = _u8L("Detected a new parameter package to update, would you like to update it?");

        wxGetApp().plater()->get_notification_manager()->push_update_params_tip(ssTip);
    } else {
        closeParamsUpdateTip();
    }
}

void UpdateParams::closeParamsUpdateTip() {
    std::string ssTip = _u8L("Detected a new parameter package to update, would you like to update it?");

    wxGetApp().plater()->get_notification_manager()->close_update_params_tip(ssTip);
}

void UpdateParams::hasUpdateParams() {
    m_bHasUpdateParams = true;
}

void UpdateParams::getCurrentPrinter(std::vector<std::string>& vtCurPrinter) 
{
    vtCurPrinter.clear();

    const std::deque<Preset>& printer_presets = GUI::wxGetApp().preset_bundle->printers.get_presets();
    for (const Preset& printer_preset : printer_presets) {
        std::string preset_name = printer_preset.name;
        if (!printer_preset.is_visible || printer_preset.is_default || printer_preset.is_project_embedded)
            continue;

        if (printer_preset.is_user()) {
            preset_name = printer_preset.inherits();
        }
        vtCurPrinter.push_back(preset_name);
    }
}

}
}
