#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../io/fszip.h"
#include "../../util/threadutils.h"

UnzipDlg::UnzipDlg(int title, const std::string& label, const std::string& zipFilePath, const std::string& destDirPath, std::function<void(bool)> onCompleted) : BaseDlg(title, 400, 170), label(label), onCompleted(onCompleted) {
    runInBackgroundThread([this, zipFilePath, destDirPath]() {
        std::string result = FsZip::unzip(zipFilePath, destDirPath, [this](U32 percentDone, std::string currentFile) {
            this->percentDone = percentDone;
            this->currentFile = currentFile;
            });
        if (result.length()==0) {
            this->unzipCompleted();
        } else {
            this->unzipFailed(result);
        }
        });
    GlobalSettings::useFastFrameRate(true);
}

UnzipDlg::~UnzipDlg() {
    GlobalSettings::useFastFrameRate(false);
}

void UnzipDlg::unzipCompleted() {
    this->onCompleted(true);
    this->done();
}

void UnzipDlg::unzipFailed(const std::string& errorMsg) {
    runOnMainUI([errorMsg]() {
        new OkDlg(GENERIC_DLG_ERROR_TITLE, errorMsg, nullptr, 500, 300);
        return false;
        });
    this->onCompleted(false);
    this->done();
}

void UnzipDlg::run() {
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    ImGui::SetCursorPosX(this->width / 2 - ImGui::GetSpinnerWidth() / 2);
    ImGui::Spinner("1");
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    ImGui::SetCursorPosX(this->width / 2 - ImGui::CalcTextSize(this->label.c_str()).x / 2);
    SAFE_IMGUI_TEXT(this->label.c_str());
    
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    ImGui::Dummy(ImVec2(0.0f, 0.0f));
    ImGui::SameLine(this->extraVerticalSpacing * 2);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImGui::GetColorU32(ImGuiCol_ButtonActive));
    ImGui::ProgressBar(this->percentDone / 100.0f, ImVec2(this->width - this->extraVerticalSpacing * 4, 0));
    ImGui::PopStyleColor();

    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    std::string l = "Unzipping "+ this->currentFile;
    ImGui::SetCursorPosX(this->width / 2 - ImGui::CalcTextSize(l.c_str()).x / 2);
    SAFE_IMGUI_TEXT(l.c_str());
}