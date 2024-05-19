#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../util/networkutils.h"
#include "../../util/threadutils.h"
#include "../../io/fs.h"
#include "knativethread.h"

#ifdef WIN32
#undef BOOL
#include <winsock.h>
#else
#include <unistd.h>
static void closesocket(int socket) { close(socket); }
#endif

DownloadDlg::DownloadDlg(Msg title, const std::vector<DownloadItem>& items, std::function<void(bool)> onCompleted) : BaseDlg(title, 400, 175), items(items), onCompleted(onCompleted) {    
    runInBackgroundThread([this]() {
        BString errorMsg; 
        U64 totalSize = 0;
        U64 completedSize = 0;
        for (auto& item : this->items) {
            totalSize += item.size;
        }
        if (totalSize) {
            this->hasSize = true;
        }
        bool result = false;
        for (auto& item : this->items) {
            BString tmpFilePath = item.filePath + ".part";
            this->currentLabel = item.label;

            if (Fs::doesNativePathExist(tmpFilePath)) {
                if (Fs::deleteNativeFile(tmpFilePath) != 0) {
                    errorMsg = "Failed to delete tmp file: " + tmpFilePath + " (";
                    errorMsg += strerror(errno);
                    errorMsg += ")";
                    result = false;
                    break;
                }
            }

            result = downloadFile(item.url, tmpFilePath, [this, totalSize, completedSize](U64 bytesCompleted) {
                if (totalSize) {
                    this->percentDone = (U32)(100 * (completedSize + bytesCompleted) / totalSize);
                }
                }, nullptr, errorMsg, &this->cancelled);
            if (!result && item.urlBackup.length() && !this->cancelled) {
                errorMsg = "";
                result = downloadFile(item.urlBackup, tmpFilePath, [this, totalSize, completedSize](U64 bytesCompleted) {
                    if (totalSize) {
                        this->percentDone = (U32)(100 * (completedSize + bytesCompleted) / totalSize);
                    }
                    }, nullptr, errorMsg, &this->cancelled);
            }
            if (!result) {
                break;
            }
            completedSize += item.size;

            if (Fs::doesNativePathExist(item.filePath)) {
                if (Fs::deleteNativeFile(item.filePath) != 0) {
                    errorMsg = "Failed to delete old file: " + item.filePath + " (";
                    errorMsg += strerror(errno);
                    errorMsg += ")";
                    result = false;
                    break;
                }
            }
            if (rename(tmpFilePath.c_str(), item.filePath.c_str()) != 0) {
                errorMsg = "Failed to rename tmp file: " + tmpFilePath + " (";
                errorMsg += strerror(errno);
                errorMsg += ")";
                result = false;
                break;
            }
        }
        if (result) {
            this->downloadCompleted();
        } else {
            this->downloadFailed(errorMsg);
        }
        this->downloadDone = true;
        });    
    GlobalSettings::useFastFrameRate(true);
}

DownloadDlg::~DownloadDlg() {
    this->cancelled = true;
    while (!downloadDone) {
        KNativeThread::sleep(1);
    }
    GlobalSettings::useFastFrameRate(false);
}

void DownloadDlg::downloadCompleted() {
    this->onCompleted(true);
    this->done();
}

void DownloadDlg::downloadFailed(BString errorMsg) {
    if (this->cancelled) {
        this->done();
    } else {
        this->errorMsg = errorMsg;
        showErrorMsg(true);
    }
}

void DownloadDlg::showErrorMsg(bool open) {
    BString error = this->errorMsg;

    runOnMainUI([error]() {
        new OkDlg(Msg::GENERIC_DLG_ERROR_TITLE, error, nullptr);
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
    ImGui::SetCursorPosX(this->width / 2 - ImGui::CalcTextSize(this->currentLabel.c_str()).x / 2);
    SAFE_IMGUI_TEXT(this->currentLabel.c_str());
    if (this->hasSize) {
        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
        ImGui::Dummy(ImVec2(0.0f, 0.0f));
        ImGui::SameLine(this->extraVerticalSpacing*2);
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImGui::GetColorU32(ImGuiCol_ButtonActive));
        ImGui::ProgressBar(this->percentDone / 100.0f, ImVec2(this->width - this->extraVerticalSpacing*4, 0));
        ImGui::PopStyleColor();
    }
    float buttonWidth = ImGui::CalcTextSize(c_getTranslation(Msg::GENERIC_DLG_CANCEL)).x + ImGui::GetStyle().FramePadding.x * 2;
    ImGui::SetCursorPosX(this->width/2-buttonWidth/2);
    ImGui::SetCursorPosY(this->height - ImGui::GetFrameHeightWithSpacing() - ImGui::GetStyle().ItemSpacing.y);
    if (ImGui::Button(c_getTranslation(Msg::GENERIC_DLG_CANCEL))) {
        // :TODO: what if the socket is stuck, this will only work if the socket is just too slow
        this->cancelled = true;
        this->currentLabel = getTranslation(Msg::DOWNLOADDLG_CANCELLING_LABEL);
    }
    if (!this->errorMsg.isEmpty()) {
        showErrorMsg(false);
    }
}
