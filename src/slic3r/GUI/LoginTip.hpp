#ifndef slic3r_LoginTip_hpp_
#define slic3r_LoginTip_hpp_
#include <vector>
#include <string>
#include <map>
#include <json_diff.hpp>

namespace Slic3r {
namespace GUI {

class LoginTip
{
public:
    static LoginTip& getInstance();
    // 1:不是用户预设, 0:是用户预设，且用户账号正常, wxID_YES:点击了登录
    int isFilamentUserMaterialValid(const std::string& userMaterial);
    int showTokenInvalidTipDlg(const std::string& fromPage);
    void resetHasSkipToLogin();

private:
    int syncShowTokenInvalidTipDlg(const std::string& fromPage);
    int showAutoMappingNoLoginTipDlg(const std::string& fromPage);
    int showAutoMappingDiffAccountTipDlg(const std::string& fromPage);

private:
    LoginTip();

private:
    bool m_bShowTipDlg = true;
    bool m_bHasSkipToLogin = false;
};

}
}

#endif
