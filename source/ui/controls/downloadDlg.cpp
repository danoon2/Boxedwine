#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../util/networkutils.h"
#include "../../util/threadutils.h"
#include "../../io/fs.h"

DownloadDlg::DownloadDlg(int title, const std::string& label, const std::string& url, const std::string& filePath, std::function<void(bool)> onCompleted, U64 expectedFileSize) : BaseDlg(title, 400, 150), label(label), url(url), filePath(filePath), percentDone(0), expectedFileSize(expectedFileSize), onCompleted(onCompleted) {
    std::string tmpFilePath = filePath + ".part";
    if (Fs::doesNativePathExist(tmpFilePath)) {
        if (Fs::deleteNativeFile(tmpFilePath) != 0) {
            std::string errorMsg = "Failed to delete tmp file: " + tmpFilePath + " (";
            errorMsg += strerror(errno);
            errorMsg += ")";
            downloadFailed(errorMsg);
            return;
        }
    }
    runInBackgroundThread([this, url, tmpFilePath]() {
        std::string errorMsg;        
        bool result = downloadFile(url, tmpFilePath, [this](U64 bytesCompleted) {
            if (this->expectedFileSize) {
                this->percentDone = (U32)(100 * bytesCompleted / this->expectedFileSize);
            }
            
            }, NULL, errorMsg);
        if (result) {
            this->downloadCompleted();
        } else {
            this->downloadFailed(errorMsg);
        }
        });    
}

void DownloadDlg::downloadCompleted() {
    if (Fs::doesNativePathExist(filePath)) {
        if (Fs::deleteNativeFile(filePath) != 0) {
            std::string errorMsg = "Failed to delete old file: " + filePath + " (";
            errorMsg += strerror(errno);
            errorMsg += ")";
        }
    }
    std::string tmpFilePath = filePath + ".part";
    if (rename(tmpFilePath.c_str(), filePath.c_str()) != 0) {
        std::string errorMsg = "Failed to rename tmp file: " + tmpFilePath + " (";
        errorMsg += strerror(errno);
        errorMsg += ")";
        downloadFailed(errorMsg);
    } else {
        this->onCompleted(true);
        this->done();
    }
}

void DownloadDlg::downloadFailed(const std::string& errorMsg) {
    this->errorMsg = errorMsg;
    showErrorMsg(true);
}

void DownloadDlg::showErrorMsg(bool open) {
    if (!showMessageBox("DownloadErrorMsg", true, getTranslation(GENERIC_DLG_ERROR_TITLE), this->errorMsg.c_str())) {
        this->onCompleted(false);
        this->done();
    }
}

void DownloadDlg::run() {
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    ImGui::SetCursorPosX(this->width / 2 - ImGui::GetSpinnerWidth() / 2);
    ImGui::Spinner("1");
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    ImGui::SetCursorPosX(this->width / 2 - ImGui::CalcTextSize(this->label.c_str()).x / 2);
    SAFE_IMGUI_TEXT(this->label.c_str());
    if (this->expectedFileSize) {
        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
        ImGui::Dummy(ImVec2(0.0f, 0.0f));
        ImGui::SameLine(this->extraVerticalSpacing*2);
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImGui::GetColorU32(ImGuiCol_ButtonActive));
        ImGui::ProgressBar(this->percentDone / 100.0f, ImVec2(this->width - this->extraVerticalSpacing*4, 0));
        ImGui::PopStyleColor();
    }
    if (this->errorMsg.length()) {
        showErrorMsg(false);
    }
}
