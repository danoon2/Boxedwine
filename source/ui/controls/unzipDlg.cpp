/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../io/fszip.h"
#include "../../util/threadutils.h"

UnzipDlg::UnzipDlg(Msg title, BString label, BString zipFilePath, BString destDirPath, std::function<void(bool)> onCompleted) : BaseDlg(title, 400, 170), label(label), onCompleted(onCompleted) {
    runInBackgroundThread([this, zipFilePath, destDirPath]() {
        BString result = FsZip::unzip(zipFilePath, destDirPath, [this](U32 percentDone, BString currentFile) {
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

void UnzipDlg::unzipFailed(BString errorMsg) {
    runOnMainUI([errorMsg]() {
        new OkDlg(Msg::GENERIC_DLG_ERROR_TITLE, errorMsg, nullptr, 500, 300);
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
    BString l = "Unzipping "+ this->currentFile;
    ImGui::SetCursorPosX(this->width / 2 - ImGui::CalcTextSize(l.c_str()).x / 2);
    SAFE_IMGUI_TEXT(l.c_str());
}