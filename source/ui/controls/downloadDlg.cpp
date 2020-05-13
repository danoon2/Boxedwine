#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../util/networkutils.h"
#include "../../util/threadutils.h"
#include "../../io/fs.h"
#include <SDL.h>

#ifdef WIN32
#undef BOOL
#include <winsock.h>
#else
#include <unistd.h>
static void closesocket(int socket) { close(socket); }
#endif

DownloadDlg::DownloadDlg(int title, const std::string& label, const std::string& url, const std::string& filePath, std::function<void(bool)> onCompleted, U64 expectedFileSize) : BaseDlg(title, 400, 175), label(label), url(url), filePath(filePath), percentDone(0), expectedFileSize(expectedFileSize), onCompleted(onCompleted), cancelled(false), socketfd(0), downloadDone(false) {
    this->tmpFilePath = filePath + ".part";
    if (Fs::doesNativePathExist(this->tmpFilePath)) {
        if (Fs::deleteNativeFile(this->tmpFilePath) != 0) {
            std::string errorMsg = "Failed to delete tmp file: " + this->tmpFilePath + " (";
            errorMsg += strerror(errno);
            errorMsg += ")";
            downloadFailed(errorMsg);
            return;
        }
    }
    runInBackgroundThread([this, url]() {
        std::string errorMsg;        
        bool result = downloadFile(url, this->tmpFilePath, [this](U64 bytesCompleted) {
            if (this->expectedFileSize) {
                this->percentDone = (U32)(100 * bytesCompleted / this->expectedFileSize);
            }
            
            }, NULL, errorMsg, &this->cancelled, &this->socketfd);
        if (result) {
            this->downloadCompleted();
        } else {
            this->downloadFailed(errorMsg);
        }
        this->socketfd = 0;
        this->downloadDone = true;
        });    
    GlobalSettings::useFastFrameRate(true);
}

DownloadDlg::~DownloadDlg() {
    this->cancelled = true;
    if (this->socketfd) {
        closesocket(this->socketfd);
    }
    while (!downloadDone) {
        SDL_Delay(1);
    }
    GlobalSettings::useFastFrameRate(false);
}

void DownloadDlg::downloadCompleted() {
    if (Fs::doesNativePathExist(filePath)) {
        if (Fs::deleteNativeFile(filePath) != 0) {
            std::string errorMsg = "Failed to delete old file: " + filePath + " (";
            errorMsg += strerror(errno);
            errorMsg += ")";
        }
    }
    if (rename(this->tmpFilePath.c_str(), filePath.c_str()) != 0) {
        std::string errorMsg = "Failed to rename tmp file: " + this->tmpFilePath + " (";
        errorMsg += strerror(errno);
        errorMsg += ")";
        downloadFailed(errorMsg);
    } else {
        this->onCompleted(true);
        this->done();
    }
}

void DownloadDlg::downloadFailed(const std::string& errorMsg) {
    if (this->cancelled) {
        Fs::deleteNativeFile(this->tmpFilePath);
        this->done();
    } else {
        this->errorMsg = errorMsg;
        showErrorMsg(true);
    }
}

void DownloadDlg::showErrorMsg(bool open) {
    std::string error = this->errorMsg;

    runOnMainUI([error]() {
        new OkDlg(GENERIC_DLG_ERROR_TITLE, error, nullptr);
        return false;
        });
    this->onCompleted(false);
    this->done();
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
    float buttonWidth = ImGui::CalcTextSize(getTranslation(GENERIC_DLG_CANCEL)).x + ImGui::GetStyle().FramePadding.x * 2;
    ImGui::SetCursorPosX(this->width/2-buttonWidth/2);
    ImGui::SetCursorPosY(this->height - ImGui::GetFrameHeightWithSpacing() - ImGui::GetStyle().ItemSpacing.y);
    if (ImGui::Button(getTranslation(GENERIC_DLG_CANCEL))) {
        // :TODO: what if the socket is stuck, this will only work if the socket is just too slow
        this->cancelled = true;
        if (this->socketfd) {
            closesocket(this->socketfd);
        }
        this->label = getTranslation(DOWNLOADDLG_CANCELLING_LABEL);
    }
    if (this->errorMsg.length()) {
        showErrorMsg(false);
    }
}
