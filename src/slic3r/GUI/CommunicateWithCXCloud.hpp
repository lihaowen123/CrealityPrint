#ifndef slic3r_CommunicateWithCXCloud_hpp_
#define slic3r_CommunicateWithCXCloud_hpp_

#include <string>
#include <vector>
#include <map>
#include <set>

#include "libslic3r/PresetBundle.hpp"

namespace Slic3r {
namespace GUI {

    struct UserProfileListItem
    {
        std::string id = "";
        long long                lastModifyTime = 0;
        std::vector<std::string> nozzleDiameter;
        std::string printerIntName = "";
        std::string type           = "";
        std::string version        = "";
        std::string zipUrl         = "";
    };

    struct PreUpdateProfileRetInfo
    {
        std::string name = "";
        std::string settingId = "";
        std::string updatedInfo = "";
        long long   updateTime = 0;
    };

    struct UploadFileInfo
    {
        std::string file = "";
        std::string name = "";
        std::string type     = "";
        std::string settingId = "";
    };

    struct CXCloudLoginInfo
    {
        std::string token = "";
        std::string userId = "";
        bool tokenValid = false;
    };

    struct LocalUserPreset
    {
        std::string file     = "";
        std::string infoFile = "";
        std::string name     = "";
        std::string type     = "";
        bool        isJson   = false;
        bool        needDel  = true;
    };
    struct SyncToLocalRetInfo
    {
        std::vector<LocalUserPreset> vtLocalUserPreset;
        bool                         bPrinterAllOk  = true;
        bool                         bFilamentAllOk = true;
        bool                         bProcessAllOk  = true;
    };


    enum class ENDownloadConfigState {
        ENDCS_NOT_DOWNLOAD,         //  未开始下载
        ENDCS_DOWNLOAD_SUCCESS,     //  下载成功
        ENDCS_DOWNLOAD_FAIL,        //  下载失败
        ENDCS_CXCLOUD_NO_CONFIG     //  创想云没有配置文件
    };

    class CXCloudDataCenter
    {
    public:
        static CXCloudDataCenter& getInstance();

        std::map<std::string, std::map<std::string, std::string>> getUserCloudPresets();
        void setUserCloudPresets(const std::string& presetName, const std::string& settingID, const std::map<std::string, std::string>& mapPresetValue);
        void cleanUserCloudPresets();
        void updateUserCloudPresets(const std::string& presetName, const std::string& settingID, const std::map<std::string, std::string>& mapPresetValue);
        int  deleteUserPresetBySettingID(const std::string& settingID);

        void setDownloadConfigToLocalState(ENDownloadConfigState state);
        // -1:未开始下载 0:下载成功， 1：下载失败, 2: 创想云没有配置文件
        ENDownloadConfigState getDownloadConfigToLocalState();
        // 是否到了上传配置文件到创想云的时间
        bool isUpdateConfigFileTimeout();
        void resetUpdateConfigFileTime();
        void setConfigFileRetInfo(const PreUpdateProfileRetInfo& retInfo);
        const PreUpdateProfileRetInfo& getConfigFileRetInfo();
        void                           setSyncData(const json& j) { m_jsonCXCloudSyncData = j; }
        const json&                    getSyncData() { return m_jsonCXCloudSyncData; };
        void                           updateCXCloutLoginInfo(const std::string& userId, const std::string& token);
        void                           setTokenInvalid();
        bool                           isTokenValid();

    private:
        CXCloudDataCenter() = default;
        ~CXCloudDataCenter() = default;

    private:
        std::map<std::string, std::map<std::string, std::string>> m_mapUserCloudPresets;
        std::map<std::string, std::string>                        m_mapSettingID2PresetName;
        std::mutex                                                 m_mutexUserCloudPresets;
        ENDownloadConfigState                                     m_enDownloadConfigToLocalState = ENDownloadConfigState::ENDCS_NOT_DOWNLOAD;
        long long m_llUpdateConfigFileTimestamp = 0;
        PreUpdateProfileRetInfo                                   m_configFileRetInfo;
        json                                                      m_jsonCXCloudSyncData = json();
        CXCloudLoginInfo                                          m_cxCloudLoginInfo;
        std::mutex                                                m_cxCloudLoginInfoMutex;
    };

    class CLocalDataCenter
    {
    public:
        static CLocalDataCenter& getInstance();

        bool isPrinterPresetExisted(const std::string& presetName);
        bool isFilamentPresetExisted(const std::string& presetName);
        bool isProcessPresetExisted(const std::string& presetName);

    private:
        CLocalDataCenter()   = default;
        ~CLocalDataCenter() = default;
    };

    struct UserInfo;
    class CommunicateWithCXCloud
    {
    public:
        struct LastError
        {
            std::string code      = "";
            std::string message   = "";
            std::string requestId = "";
        };
        CommunicateWithCXCloud();
        ~CommunicateWithCXCloud();
        //  用户参数包列表
        int getUserProfileList(std::vector<UserProfileListItem>& vtUserProfileListItem);
        //  下载用户预设
        int downloadUserPreset(const UserProfileListItem& userProfileListItem, std::string& saveJsonFile);
        //  请求上传用户参数包
        int preUpdateProfile_create(const UploadFileInfo& fileInfo, PreUpdateProfileRetInfo& retInfo);
        int preUpdateProfile_update(const UploadFileInfo& fileInfo, PreUpdateProfileRetInfo& retInfo);
        //  删除用户预设
        int deleteProfile(const std::string& ssDeleteSettingId);
        const LastError& getLastError() { return m_lastError; }

    private:
        void setLastError(const std::string& code, const std::string& msg);
        int  saveSyncDataToLocal(const UserInfo&            userInfo,
                                 const UserProfileListItem& userProfileListItem,
                                 const std::string&         body,
                                 std::string&               saveJsonFile);
        int  saveLocalDeviceToLocal(const UserInfo&            userInfo,
                                    const UserProfileListItem& userProfileListItem,
                                    const std::string&         body,
                                    std::string&               saveJsonFile);

    private:
        LastError m_lastError;
    };


    class CommunicateWithFrontPage
    {
    public:
        CommunicateWithFrontPage();
        ~CommunicateWithFrontPage();

        int getUserPresetParams(const std::map<std::string, std::map<std::string, std::string>>& mapUserCloudPresets, std::string& ssJsonData);

    private:
        int getPrinterPresetParams(const std::map<std::string, std::string>& mapPrinterPreset, std::string& ssJsonData);
        int getFilamentPresetParams(const std::map<std::string, std::string>& mapFilamentPreset, std::string& ssJsonData);
        int getProcessPresetParams(const std::map<std::string, std::string>& mapProcessPreset, std::string& ssJsonData);
        std::string getMapValue(const std::map<std::string, std::string>& mapObj, const std::string& key);

    private:
        std::set<std::string> m_setIgnoreField;
        size_t                                m_printerId = 0;
        size_t                                m_filamentId = 0;
        size_t                                m_processId = 0;
        std::unordered_map<std::string, json> m_mapPrinterData;
    };
}
}

#endif
