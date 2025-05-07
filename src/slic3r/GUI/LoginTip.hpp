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
    // 1:�����û�Ԥ��, 0:���û�Ԥ�裬���û��˺�����, wxID_YES:����˵�¼
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
