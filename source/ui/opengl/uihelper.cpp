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
        ImGui::Text(msg);
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