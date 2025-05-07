#include "CrealityrintDump.hpp"
#include "slic3r/GUI/Widgets/StaticBox.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/GUI.hpp"
#include "slic3r/GUI/BBLStatusBar.hpp"
#include "libslic3r_version.h"
#include "nlohmann/json.hpp"
#include "miniz/miniz.h"
// 自定义对话框类



ErrorReportDialog::ErrorReportDialog(wxWindow* parent, const wxString& title)
    : Slic3r::GUI::DPIDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
    SetFont(Slic3r::GUI::wxGetApp().normal_font());
    //SetBackgroundColour(wxColor("#4b4b4d"));
    bool is_dark = Slic3r::GUI::wxGetApp().dark_mode();
    wxColour color = is_dark ? wxColor("#4b4b4d") : wxColour(255, 255, 255);
    SetBackgroundColour(color);

    // 创建垂直方向的布局管理器
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // 创建水平方向的布局管理器，用于放置标题
    wxBoxSizer* titleSizer = new wxBoxSizer(wxHORIZONTAL);
    // std::filesystem::path imagePath = "resources\\images\\warning.png";
    wxIcon warningIcon = wxArtProvider::GetIcon(wxART_WARNING, wxART_MESSAGE_BOX);

    // 将图标转换为位图
    wxBitmap bitmap(warningIcon);
    // 创建 wxStaticBitmap 控件并添加到窗口
    wxStaticBitmap* staticBitmap = new wxStaticBitmap(this, wxID_ANY, bitmap, wxPoint(FromDIP(50), FromDIP(50)));
    wxBoxSizer*     titleSizer1  = new wxBoxSizer(wxVERTICAL);
    wxStaticText*   text =
        new wxStaticText(this, wxID_ANY,
                         "A serious error has occurred in Some App. Please send this error report to us to fix the problem.");
    wxStaticText* text1 =
        new wxStaticText(this, wxID_ANY, "Please click the \"Send Report\" button to automatically publish the error report to our server.");

    text->SetFont(::Label::Body_14);
    //text->SetForegroundColour(is_dark ? *wxWHITE  : * wxBLACK);
    text1->SetFont(::Label::Body_12);
    //text1->SetForegroundColour(is_dark ? *wxWHITE  : * wxBLACK);

    titleSizer->Add(staticBitmap, 0, wxALIGN_CENTER | wxALL, 10);
    titleSizer1->Add(text, 0);
    titleSizer1->Add(text1, 0);
    titleSizer->Add(titleSizer1, 0, wxALIGN_CENTER | wxALL, 10);
    sizer->Add(titleSizer, 0, wxALIGN_CENTER | wxALL, 10);

    wxBoxSizer* contentSizer = new wxBoxSizer(wxVERTICAL);
    StaticBox*  tabCtrPanel  = new StaticBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    tabCtrPanel->SetBorderWidth(1);
    tabCtrPanel->SetBorderColor(0x7A7A7F);
    tabCtrPanel->SetMinSize(wxSize(FromDIP(578), FromDIP(175)));
    tabCtrPanel->SetSizer(contentSizer);

    sizer->Add(tabCtrPanel, 0, wxALL | wxEXPAND, 5);
    GetErrorReport();
    wxString formattedString = wxString::Format(wxT("OS: %s\nGraphicsCard: %s\nOpenGLVersion: %s\nVersion: %s\nUid: %s\n"),
                                                m_info.osDescription, m_info.graphicsCardVendor, m_info.openGLVersion, m_info.build,
                                                m_info.uuid);
    // 创建水平方向的布局管理器，用于放置文本框
    wxStaticText* vtext = new wxStaticText(tabCtrPanel, wxID_ANY, formattedString, wxPoint(20, 20));
    vtext->SetFont(::Label::Body_12);
    //vtext->SetForegroundColour(is_dark ? *wxWHITE : *wxBLACK);
    contentSizer->Add(vtext, 0, wxALIGN_LEFT | wxALL, 10);

    // 创建发送按钮
    // wxButton* sendButton = new wxButton(this, wxID_OK, "SendReport");
    StateColor btn_bg_green(std::pair<wxColour, int>(wxColour(21, 191, 89), StateColor::Pressed),
                            std::pair<wxColour, int>(wxColour(21, 191, 89), StateColor::Hovered),
                            std::pair<wxColour, int>(wxColour(142, 142, 159), StateColor::Normal));

    Button* sendButton = new Button(this, _L("SendReport"));
    sendButton->SetBackgroundColor(btn_bg_green);
    // sendButton->SetBorderColor(*wxWHITE);
    sendButton->SetBorderWidth(0);
    sendButton->SetTextColor(wxColour("#FFFFFE"));
    sendButton->SetFont(Label::Body_12);
    sendButton->SetSize(wxSize(FromDIP(100), FromDIP(24)));
    sendButton->SetMinSize(wxSize(FromDIP(100), FromDIP(24)));
    sendButton->SetCornerRadius(FromDIP(12));
    sendButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) {
        sendReport();
        EndModal(wxID_OK);
    });

    // 创建取消按钮
    // wxButton* cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
    Button* cancelButton = new Button(this, _L("Cancel"));
    cancelButton->SetBackgroundColor(btn_bg_green);
    cancelButton->SetBorderWidth(0);
    cancelButton->SetTextColor(wxColour("#FFFFFE"));
    cancelButton->SetFont(Label::Body_12);
    cancelButton->SetSize(wxSize(FromDIP(100), FromDIP(24)));
    cancelButton->SetMinSize(wxSize(FromDIP(100), FromDIP(24)));
    cancelButton->SetCornerRadius(FromDIP(12));
    cancelButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) {
        EndModal(wxID_CANCEL);
    });

    // 创建水平方向的布局管理器，用于放置按钮
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->Add(sendButton, 0, wxALL, 5);
    buttonSizer->Add(cancelButton, 0, wxALL, 5);

    // 将按钮布局管理器添加到主布局管理器
    sizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, 10);

    SetMinSize(wxSize(FromDIP(600), FromDIP(350)));
    SetMaxSize(wxSize(FromDIP(600), FromDIP(350)));
    // 设置对话框的布局管理器
    SetSizer(sizer);
    // 调整对话框大小以适应内容
    sizer->Fit(this);
    Slic3r::GUI::wxGetApp().UpdateDlgDarkUI(this);
}

wxString ErrorReportDialog::getSystemInfo()
{
    nlohmann::json j;
    j["osDescription"]      = m_info.osDescription.ToStdString();
    j["graphicsCardVendor"] = m_info.graphicsCardVendor.ToStdString();
    j["openGLVersion"]      = m_info.openGLVersion.ToStdString();
    j["build"]              = m_info.build.ToStdString();
    j["uuid"]               = m_info.uuid.ToStdString();
    try {
        // 获取临时目录路径
        std::filesystem::path tempDir(wxFileName::GetTempDir().ToStdString());
        // 生成一个唯一的临时文件名
        std::filesystem::path tempFilePath = tempDir / "system_info.json";

        // 打开临时文件以写入 JSON 数据
        std::ofstream tempFile(tempFilePath);
        if (tempFile.is_open()) {
            // 将 JSON 对象写入文件，使用 dump(4) 进行格式化输出
            tempFile << j.dump(4);
            tempFile.close();
            return tempFilePath.wstring();
        } else {
        }
    } catch (const std::filesystem::filesystem_error& e) {
    } catch (const std::exception& e) {}

    return "";
}

void ErrorReportDialog::setDumpFilePath(wxString dumpFilePath) { m_dumpFilePath = dumpFilePath; }
void ErrorReportDialog::on_dpi_changed(const wxRect& suggested_rect)
{
    const int& em = em_unit();

    msw_buttons_rescale(this, em, {wxID_OK, wxID_CANCEL});

    // for (Item *item : m_items) item->update_valid_bmp();

    // const wxSize& size = wxSize(45 * em, 35 * em);
    // SetMinSize(/*size*/ wxSize(100, 50));

    //m_confirm->SetMinSize(SAVE_PRESET_DIALOG_BUTTON_SIZE);
    //m_cancel->SetMinSize(SAVE_PRESET_DIALOG_BUTTON_SIZE);

    Fit();
    Refresh();
}

void ErrorReportDialog::sendEmail(wxString zipFilePath)
{
        // 发送邮件
        CURL *curl;
        CURLcode res = CURLE_OK;
        struct curl_slist *recipients = NULL;
        struct curl_slist*   headerlist = NULL;
        //struct upload_status upload_ctx = { 0 };
        /* Time */
        time_t     rawtime;
        struct tm* timeinfo;
        char       time_buffer[128];
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(time_buffer, 128, "%a, %d %b %Y %H:%M:%S %z", timeinfo);
        const char payload_template[] = "Date: %s\r\n"
                                        "To: %s\r\n"
                                        "From: %s\r\n"
                                        "Message-ID: <%s>\r\n"
                                        "Subject: %s\r\n"
                                        "\r\n";
        char*      from               = DUMPTOOL_USER;
        char*      to                 = DUMPTOOL_TO;
        char* message_id = from;
        char*  subject          = "Error Report"; 
        size_t payload_text_len = strlen(payload_template) + strlen(time_buffer) + strlen(to) + strlen(from) + strlen(subject) +
                                  strlen(message_id) + 1;

        char* payload_text = (char*) malloc(payload_text_len);
        memset(payload_text, 0, payload_text_len);
        sprintf(payload_text, payload_template, time_buffer, to, from, message_id, subject);
        curl = curl_easy_init();
        curl_version_info_data* ver = curl_version_info(CURLVERSION_NOW);

         if(curl) {
             curl_mime* mime = curl_mime_init(curl);

             // 设置正文部分
             curl_mimepart* part = curl_mime_addpart(mime);
             curl_mime_data(part, "This is an error report.", CURL_ZERO_TERMINATED);
             curl_mime_type(part, "text/plain");


             // 设置附件部分
             part = curl_mime_addpart(mime);
             curl_mime_filedata(part, zipFilePath.ToStdString().c_str()); // 替换为你的文件路径
             curl_mime_name(part, "dumpinfo.zip");               // 设置附件名称
             curl_mime_filename(part, "dumpinfo.zip");           // 设置附件在邮件中的显示名称
             
             headerlist = curl_slist_append(headerlist, payload_text);
             curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from);
             recipients = curl_slist_append(recipients, to);
             curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
             curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
             curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
             curl_easy_setopt(curl, CURLOPT_URL, DUMPTOOL_HOST);
             curl_easy_setopt(curl, CURLOPT_USERNAME, DUMPTOOL_USER);
            curl_easy_setopt(curl, CURLOPT_PASSWORD, DUMPTOOL_PASS);
            
            
            
            
         
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
            //curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        
            
            
            res = curl_easy_perform(curl);

            if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s", curl_easy_strerror(res));

            curl_slist_free_all(recipients);
            curl_mime_free(mime);
            curl_easy_cleanup(curl);
            
        }

        return ;
    }
wxString ErrorReportDialog::zipFiles() {
        // 创建一个zip文件
        wxString format1 = "%Y%m%d%H%M%S";
        wxString zipFilePath = wxString::Format("%s/CrealityPrint_%s_%s.zip",wxFileName::GetTempDir(),CREALITYPRINT_VERSION,wxDateTime::Now().Format(format1));
        mz_zip_archive archive;
        mz_zip_zero_struct(&archive);
        mz_bool status = mz_zip_writer_init_file(&archive, zipFilePath.mb_str(), 0);
        if (!status) {
            std::cerr << "Failed to create zip file!" << std::endl;
            return "";
        }
        // 添加文件到zip文件
        wxFileName fileName(m_dumpFilePath);
        wxString nameWithExt = fileName.GetFullName();
        status = mz_zip_writer_add_file(&archive, nameWithExt.mb_str(), m_dumpFilePath.mb_str(), "", 0, MZ_BEST_COMPRESSION);
        if (!status) {
            std::cerr << "Failed to add file to zip!" << std::endl;
            mz_zip_writer_end(&archive);
            return "";
        }
         status = mz_zip_writer_add_file(&archive, "system_info.json", m_systemInfoFilePath.mb_str(), "", 0, MZ_BEST_COMPRESSION);
        if (!status) {
            std::cerr << "Failed to add file to zip!" << std::endl;
            mz_zip_writer_end(&archive);
            return "";
        }
		 //添加日志文件
        if (!addLogFiles(archive)) {
            std::cerr << "Failed to add log files to zip!" << std::endl;
            mz_zip_writer_end(&archive);
            return "";
        }

        status = mz_zip_writer_finalize_archive(&archive);
        if (MZ_FALSE == status) {
            mz_zip_writer_end(&archive);
            return "";
        }
        // 关闭zip文件
        mz_zip_writer_end(&archive);
        return zipFilePath;
    }
void ErrorReportDialog::sendReport()
    {
         m_systemInfoFilePath = getSystemInfo();
        if(!m_dumpFilePath.IsEmpty() && !m_systemInfoFilePath.IsEmpty()) {
            wxString dumpfile = zipFiles();
            if(dumpfile.IsEmpty()) {
                    return ;
            }
            sendEmail(dumpfile);
        }
    }
void ErrorReportDialog::GetErrorReport()
    {
        // 获取错误报告
        wxString osDescription = wxGetOsDescription();
        m_info.osDescription   = osDescription;
        m_info.build           = wxString(CREALITYPRINT_VERSION, wxConvUTF8);
        m_info.uuid            = wxDateTime::Now().Format("%Y%m%d%H%M%S");
        // 获取显卡信息
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW!" << std::endl;
            return;
        }

        // 创建隐藏（不可见）窗口
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        GLFWwindow* window = glfwCreateWindow(1, 1, "", nullptr, nullptr);

        if (!window) {
            std::cerr << "Failed to create hidden GLFW window!" << std::endl;
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window);

        // 初始化GLEW
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW!" << std::endl;
            return;
        }

        // 获取显卡信息
        const GLubyte* renderer = glGetString(GL_RENDERER);
        const GLubyte* version  = glGetString(GL_VERSION);

        std::cout << "Renderer: " << renderer << std::endl;
        std::cout << "OpenGL version: " << version << std::endl;
        m_info.graphicsCardVendor = wxString(reinterpret_cast<char*>(const_cast<GLubyte*>(renderer)), wxConvUTF8);
        m_info.openGLVersion      = wxString(reinterpret_cast<char*>(const_cast<GLubyte*>(version)), wxConvUTF8);
        // 销毁窗口和清理
        glfwDestroyWindow(window);
        glfwTerminate();

        return;
    }
    // 自定义应用程序类
bool ErrorReportDialog::addLogFiles(mz_zip_archive& archive) {
    try {
        // 获取日志目录路径
        std::string log_dir = Slic3r::data_dir() + "\\log";
        std::filesystem::path log_path(log_dir);
        
        // 检查目录是否存在
        if (!std::filesystem::exists(log_path)) {
            std::cerr << "Log directory does not exist: " << log_dir << std::endl;
            return false;
        }

        // 收集所有.log.0文件
        std::vector<std::filesystem::path> log_files;
        for (const auto& entry : std::filesystem::directory_iterator(log_path)) {
            if (entry.path().extension() == ".0" && 
                entry.path().stem().extension() == ".log") {
                log_files.push_back(entry.path());
            }
        }

        // 按最后修改时间排序
        std::sort(log_files.begin(), log_files.end(),
            [](const std::filesystem::path& a, const std::filesystem::path& b) {
                return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
            });

        // 只取最近的5个文件
        size_t count = std::min(size_t(5), log_files.size());
        for (size_t i = 0; i < count; ++i) {
            const auto& log_file = log_files[i];
            std::string filename = log_file.filename().string();
            
            // 添加到zip文件
            mz_bool status = mz_zip_writer_add_file(&archive, 
                filename.c_str(),
                log_file.string().c_str(),
                "", 0, MZ_BEST_COMPRESSION);
                
            if (!status) {
                std::cerr << "Failed to add log file to zip: " << filename << std::endl;
                return false;
            }
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception while adding log files: " << e.what() << std::endl;
        return false;
    }
}