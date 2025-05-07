#pragma once

#include <wx/app.h>
#include <wx/wx.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/statbmp.h>
#include <wx/artprov.h>
#include <wx/statline.h>
#include <wx/filename.h>
#include <filesystem>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include "slic3r/GUI/GUI_Utils.hpp"
#include "miniz/miniz.h"


struct SystemInfo {
    wxString osDescription;
    wxString graphicsCardVendor;
    wxString openGLVersion;
    wxString build;
    wxString uuid;
};

class ErrorReportDialog : public Slic3r::GUI::DPIDialog
{
public:
    // 构造函数，接收父窗口指针和对话框标题
    ErrorReportDialog(wxWindow* parent, const wxString& title);
    wxString getSystemInfo();
    void sendReport();
    wxString zipFiles();
    void     setDumpFilePath(wxString dumpFilePath);
	bool addLogFiles(mz_zip_archive& archive);

protected:
    void on_dpi_changed(const wxRect& suggested_rect) override;
    void on_sys_color_changed() override {}

private:
    SystemInfo m_info;
    wxString m_dumpFilePath;
    wxString m_systemInfoFilePath;
    void sendEmail(wxString zipFilePath);
    void       GetErrorReport();

};