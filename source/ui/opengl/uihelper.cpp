#include "boxedwine.h"
#include "../boxedwineui.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../utils/stb_image.h"

// Simple helper function to load an image into a OpenGL texture with common settings
#include <GL/glew.h>

bool showMessageBox(bool open, const char* title, const char* msg) {    
    bool result = true;
    ImGui::PushID(msg);
    if (open) {
        ImGui::OpenPopup(title);
    }
    if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        SAFE_IMGUI_TEXT(msg);
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) { 
            ImGui::CloseCurrentPopup(); 
            result = false;
        }
        ImGui::SetItemDefaultFocus();
        ImGui::EndPopup();
    }
    ImGui::PopID();
    return result;
}

bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture=0;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

std::string getReadableSize(U64 bytes) {
    if (bytes < 4096) {
        return std::to_string(bytes)+"  B";
    }
    bytes /= 1024;
    if (bytes < 4096) {
        return std::to_string(bytes)+" KB";
    }
    bytes /= 1024;
    if (bytes < 4096) {
        return std::to_string(bytes)+" MB";
    }
    bytes /= 1024;
    return std::to_string(bytes)+" GB";
}

void alignNextTextRightInColumn(const char* text) {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(text).x  - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
}

void runCommandLineUtil(BoxedContainer* container, const std::string workingDirectory, const std::vector<std::string> args) {
    GlobalSettings::startUpArgs = StartUpArgs();
    container->launch();
    
    GlobalSettings::startUpArgs.setWorkingDir(workingDirectory);
    GlobalSettings::startUpArgs.addArgs(args);
    GlobalSettings::startUpArgs.videoEnabled = false;
    GlobalSettings::startUpArgs.soundEnabled = false;
    GlobalSettings::startUpArgs.apply();
}

std::string createIcon(BoxedContainer* container, const std::string& exeLocalPath, int preferedSize) {
    std::string exeNativePath = container->getDir()+exeLocalPath;
    Fs::localNameToRemote(exeNativePath);
    std::string exeName = Fs::getFileNameFromNativePath(exeNativePath);
    std::string localDirectoryPath = "/home/username/.boxedwineui/icons";
    Fs::localNameToRemote(localDirectoryPath);
    std::string nativeDirectoryPath = GlobalSettings::getRootFolder(container) + localDirectoryPath;
    if (!Fs::doesNativePathExist(nativeDirectoryPath)) {
        Fs::makeNativeDirs(nativeDirectoryPath);
    }
    std::vector<std::string> iconFiles = Fs::getFilesInNativeDirectoryWhereFileMatches(nativeDirectoryPath, exeName, ".ico", true);
    if (!iconFiles.size()) {        
        std::vector<std::string> args = {"/usr/bin/wrestool", "-x", "--output=.", "-t14", exeLocalPath.c_str()};
        runCommandLineUtil(container, "/home/username/.boxedwineui/icons", args);
        iconFiles = Fs::getFilesInNativeDirectoryWhereFileMatches(nativeDirectoryPath, exeName, ".ico", true);
    }        
    if (iconFiles.size()) {
        std::vector<std::string> pngFiles = Fs::getFilesInNativeDirectoryWhereFileMatches(nativeDirectoryPath, exeName, ".png", true);
        if (pngFiles.size()==0) {
            std::sort(iconFiles.begin(), iconFiles.end());
            std::string localIconFilePath =  "/home/username/.boxedwineui/icons/"+Fs::getFileNameFromNativePath(iconFiles[0]);
            std::vector<std::string> args = {"/usr/bin/icotool", "-x", localIconFilePath.c_str()};
            runCommandLineUtil(container, "/home/username/.boxedwineui/icons", args);
            pngFiles = Fs::getFilesInNativeDirectoryWhereFileMatches(nativeDirectoryPath, exeName, ".png", true);
        }
        if (pngFiles.size()) {
            std::string sizeCriteria = std::to_string(preferedSize)+"x"+std::to_string(preferedSize)+"x"+std::to_string(32);
            for (int i=0;i<(int)pngFiles.size();i++) {
                if (stringContains(pngFiles[i], sizeCriteria)) {
                    return pngFiles[i];
                }
            }
            sizeCriteria = std::to_string(preferedSize)+"x"+std::to_string(preferedSize)+"x"+std::to_string(8);
            for (int i=0;i<(int)pngFiles.size();i++) {
                if (stringContains(pngFiles[i], sizeCriteria)) {
                    return pngFiles[i];
                }
            }
            sizeCriteria = std::to_string(preferedSize)+"x"+std::to_string(preferedSize)+"x"+std::to_string(4);
            for (int i=0;i<(int)pngFiles.size();i++) {
                if (stringContains(pngFiles[i], sizeCriteria)) {
                    return pngFiles[i];
                }
            }
            return pngFiles[0];
        }
    }
    return "";
}