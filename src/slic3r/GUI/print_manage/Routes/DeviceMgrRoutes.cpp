#include "DeviceMgrRoutes.hpp"
#include "nlohmann/json.hpp"
#include "../AppUtils.hpp"
#include "../data/DataCenter.hpp"
#include "../../GUI.hpp"
#include "../PrinterMgr.hpp"
#include "Http.hpp"
#include "cereal/external/base64.hpp"
#include "slic3r/GUI/Notebook.hpp"
#include "slic3r/GUI/MainFrame.hpp"
#include "../AccountDeviceMgr.hpp"
#include "slic3r/GUI/print_manage/RemotePrinterManager.hpp"
#include "slic3r/GUI/print_manage/Utils.hpp"
namespace DM {
    DeviceMgrRoutes::DeviceMgrRoutes()
    {
        this->Handler({ "init_device" }, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {
            nlohmann::json commandJson;
            commandJson["command"] = "init_device";
            commandJson["data"] = DM::DeviceMgr::Ins().GetData();
            std::string commandStr = commandJson.dump(-1,' ',true);
            wxString strJS = wxString::Format("window.handleStudioCmd('%s');", RemotePrint::Utils::url_encode(commandStr));
            AppUtils::PostMsg(browse, strJS.ToStdString());
            return true;
            });

        this->Handler({ "request_all_device", "get_devices" }, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {
            nlohmann::json commandJson;
            commandJson["command"] = cmd;
            commandJson["data"] = DataCenter::Ins().GetData();
            std::string commandStr = commandJson.dump(-1,' ',true);
            wxString strJS = wxString::Format("window.handleStudioCmd('%s');", RemotePrint::Utils::url_encode(commandStr));
            AppUtils::PostMsg(browse, strJS.ToStdString());
            return true;
            });

        // form device, real time update of the data
        this->Handler({ "update_devices" }, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {
            DM::DataCenter::Ins().update_data(json_data);
            if (DM::DataCenter::Ins().is_current_device_changed()) {
                wxPostEvent(wxGetApp().plater(), wxCommandEvent(EVT_CURRENT_DEVICE_CHANGED));
            }

            return true;
            });

        //for device module 
        this->Handler({ "get_device_merge_state" }, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {
            nlohmann::json commandJson;
            commandJson["command"] = "get_device_merge_state";
            commandJson["data"] = DM::DeviceMgr::Ins().IsMergeState();
            AppUtils::PostMsg(browse, commandJson);

            return true;
            });

        //for device module 
        this->Handler({ "set_device_merge_state" }, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {
            if (json_data.contains("state"))
            {
                DM::DeviceMgr::Ins().SetMergeState(json_data["state"].get<bool>());
            }

            return true;
            });

        // web rtc local get 
        this->Handler({ "get_webrtc_local_param" }, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {

            std::string url = json_data["url"].get<std::string>();
            std::string  sdp = json_data["sdp"].get<std::string>();
            Slic3r::Http http = Slic3r::Http::post(url);

            std::string localip = "";
            try {
                // 提取域名部分
                std::string domain = DM::AppUtils::extractDomain(url);
                // 创建一个 Boost.Asio 的 io_context 对象
                boost::asio::io_context io_context;
                // 创建一个 UDP 套接字
                boost::asio::ip::udp::socket socket(io_context);
                // 连接到一个公共的 UDP 地址和端口（Google 的公共 DNS 服务器）
                socket.connect(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(domain), 80));
                // 获取本地端点信息
                boost::asio::ip::udp::endpoint local_endpoint = socket.local_endpoint();
                // 关闭套接字
                socket.close();
                // 返回本地 IP 地址的字符串表示
                localip = local_endpoint.address().to_string();
            }
            catch (const std::exception& e) {
                // 若出现异常，输出错误信息并返回空字符串
                std::cerr << "Error: " << e.what() << std::endl;
            }
            if (!localip.empty())
            {
                std::string mdns_addr = "";
                std::vector<std::string> tokens;
                boost::split(tokens, sdp, boost::is_any_of("\n"));
                for (const auto& token : tokens) {
                    if (token.find("a=candidate") != std::string::npos) {
                        std::vector<std::string> sub_tokens;
                        boost::split(sub_tokens, token, boost::is_any_of(" "));
                        mdns_addr = sub_tokens[4];
                        break;
                        //sdp = sdp.replace("a=candidate", "a=candidate" + " " + "raddr=" + localip);
                    }
                }
                if (!mdns_addr.empty())
                {
                    boost::algorithm::replace_first(sdp, mdns_addr, localip);
                }

                //sdp = sdp.replace("
            }


            nlohmann::json j;
            j["type"] = "offer";
            j["sdp"] = sdp;

            std::string d = j.dump();
            std::string e = cereal::base64::encode((unsigned char const*)d.c_str(), d.length());

            http.header("Content-Type", "application/json")
                .set_post_body(e)
                .on_complete([&](std::string body, unsigned status) {
                if (status != 200) {
                    return;
                }

                nlohmann::json data;
                data["body"] = body;
                data["ret"] = true;

                nlohmann::json commandJson;
                commandJson["command"] = "get_webrtc_local_param";
                commandJson["data"] = data;

                AppUtils::PostMsg(browse, wxString::Format("window.handleStudioCmd('%s');", commandJson.dump(-1, ' ', true)).ToStdString());

                    })
                .on_error([&](std::string body, std::string error, unsigned status) {
                        nlohmann::json data;
                        data["body"] = body;
                        data["ret"] = false;

                        nlohmann::json commandJson;
                        commandJson["command"] = "get_webrtc_local_param";
                        commandJson["data"] = data;
                        AppUtils::PostMsg(browse,
                            wxString::Format("window.handleStudioCmd('%s');", commandJson.dump(-1, ' ', true)).ToStdString());
                    })
                        .perform_sync();

                    return true;
            });

        // set active device
        this->Handler({ "set_current_device" }, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {
            std::string mac = json_data.contains("device_id") ? json_data["device_id"] : "";
            DM::DeviceMgr::Ins().SetCurrentDevice(mac);
            return true;
            });

        this->Handler({ "get_current_device" }, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {

            DM::Device device= DM::DataCenter::Ins().get_current_device_data();
            if (!device.valid) {
                return true;
            }

            // Create top-level JSON object
            nlohmann::json top_level_json = {
                {"mac", device.mac}
            };

            // Create command JSON object
            nlohmann::json commandJson = {
                {"command", "get_current_device"},
                {"data", top_level_json}
            };

            AppUtils::PostMsg(browse, commandJson);

            return true;
            });

        this->Handler({ "edit_device_group_name" }, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {
            std::string oldName = json_data["oldName"];
            std::string newName = json_data["newName"];
            DM::DeviceMgr::Ins().EditGroupName(oldName, newName);
            return true;
            });

        this->Handler({ "remove_group" }, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {
            std::string name = json_data["name"];
            DM::DeviceMgr::Ins().RemoveGroup(name);
            AccountDeviceMgr::getInstance().unbind_device_by_group(name);
            return true;
            });

        this->Handler({ "edit_device_name" }, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {
            std::string address = json_data["address"];
            std::string name = json_data["name"];
            DM::DeviceMgr::Ins().EditDeiveName(address, name);
            return true;
            });

        this->Handler({ "remove_device" }, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {
            std::string address = json_data["address"];
            DM::DeviceMgr::Ins().RemoveDevice(address);
            AccountDeviceMgr::getInstance().unbind_device_by_address(address);
            return true;
            });
        this->Handler({ "update_device" }, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {
            DM::DeviceMgr::Data device;
            device.address = json_data["address"];
            device.mac = json_data["mac"];
            device.model = json_data["model"];
            device.connectType = json_data["type"];
            DM::DeviceMgr::Ins().UpdateDevice(device.address, device);
            return true;
            });
        this->Handler({ "add_device" }, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {
            DM::DeviceMgr::Data device;
            device.address = json_data["address"];
            device.mac = json_data["mac"];
            device.model = json_data["model"];
            device.connectType = json_data["type"];

            std::string group = json_data["group"];
            DM::DeviceMgr::Ins().AddDevice(group, device);
            return true;
            });

        this->Handler({"remove_to_first"}, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {
            DM::DeviceMgr::Data device;
            std::string         name = json_data["name"];
            DM::DeviceMgr::Ins().remove2FirstGroup(name);
            return true;
        });

        this->Handler({"move_to_group"}, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {
            DM::DeviceMgr::Data device;
            std::string         originGroup = json_data["originGroup"];
            std::string         targetGroup = json_data["targetGroup"];
            std::string         address     = json_data["address"];
            DM::DeviceMgr::Ins().move2Group(originGroup, targetGroup, address);
            return true;
        });

        this->Handler({"sort_group"}, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {
            DM::DeviceMgr::Data device;
            std::string              jsonString = json_data["groupMap"];
            json                     groupMap   = json::parse(jsonString);
            std::vector<std::string> groupVector;
            for (const auto& elem : groupMap) {
                groupVector.push_back(elem.get<std::string>());
            }
            DM::DeviceMgr::Ins().sortGroup(groupVector);
            return true;
        });

        this->Handler({ "add_group" }, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {
            std::string group = json_data["group"];
            DM::DeviceMgr::Ins().AddGroup(group);
            return true;
            });

        this->Handler({ "forward_device_detail" }, [](wxWebView* browse, const std::string& data, nlohmann::json& json_data, const std::string cmd) {
            wxCommandEvent e = wxCommandEvent(wxCUSTOMEVT_NOTEBOOK_SEL_CHANGED);
            e.SetId(MainFrame::TabPosition::tpDeviceMgr); // printer details page
            wxPostEvent(wxGetApp().mainframe->topbar(), e);

            return true;
            });
    }
}