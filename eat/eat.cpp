#include "eat.hpp"
#include "eat_output.hpp"
#include <imgui.h>
#include <imgui_internal.h>

#include <winuser.h>
#pragma comment(lib, "User32.lib")

struct {
    bool show_about = false;
} g_uistate;

namespace eat {

    void on_initialize()
    {
        ImGuiStyle& style  = ImGui::GetStyle();
        ImVec4*     colors = style.Colors;

        colors[ImGuiCol_Text]                 = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled]         = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg]             = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
        colors[ImGuiCol_ChildBg]              = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg]              = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border]               = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]              = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
        colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        colors[ImGuiCol_FrameBgActive]        = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_TitleBg]              = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        colors[ImGuiCol_TitleBgActive]        = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg]            = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors[ImGuiCol_CheckMark]            = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_SliderGrab]           = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Button]               = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        colors[ImGuiCol_ButtonHovered]        = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_ButtonActive]         = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
        colors[ImGuiCol_Header]               = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
        colors[ImGuiCol_HeaderHovered]        = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        colors[ImGuiCol_HeaderActive]         = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Separator]            = colors[ImGuiCol_Border];
        colors[ImGuiCol_SeparatorHovered]     = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        colors[ImGuiCol_SeparatorActive]      = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        colors[ImGuiCol_ResizeGrip]           = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
        colors[ImGuiCol_ResizeGripHovered]    = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive]     = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_Tab] =
            ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
        colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
        colors[ImGuiCol_TabActive] =
            ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
        colors[ImGuiCol_TabUnfocused] =
            ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
        colors[ImGuiCol_TabUnfocusedActive] =
            ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
        colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        colors[ImGuiCol_DragDropTarget]        = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

        style.WindowRounding = 0.f;
    }

	void futil_copy_to_clipboard(HWND hwnd, std::string& data);
    void futil_show_about();

    void on_frame(HWND hwnd)
    {
        ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
        if(ImGui::Begin("MainWindow",
                        nullptr,
                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar)) {
            // Do the menubar
            if(ImGui::BeginMenuBar()) {
                if(ImGui::BeginMenu("Help")) {
                    if(ImGui::MenuItem("About")) {
                        g_uistate.show_about = true;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            if(g_uistate.show_about)
                futil_show_about();

            static std::string input_text_buffer = "Ready.";

            { // UsnJournal
                static bool        usn_query_completed = false;
                static std::string usn_result          = "";
                if(ImGui::Button("UsnJournal")) {
                    if(MessageBoxA(
                           hwnd,
                           "This operation can take a while and will use a bunch of memory.\nContinue?",
                           "Warning",
                           MB_ICONWARNING | MB_YESNO) == IDYES) {
                        eat::get_usn_journal_info_deferred(usn_result,
                                                           usn_query_completed);
                        input_text_buffer = "Querying UsnJournal...";
                    }
                }

                if(usn_query_completed) {
                    usn_query_completed = false;
                    input_text_buffer   = std::move(usn_result);
                }
            }

            ImGui::InputTextMultiline("##EatOutputTextBox",
                                      const_cast<char*>(input_text_buffer.c_str()),
                                      input_text_buffer.size(),
                                      ImVec2(-1.f, 500.f), ImGuiInputTextFlags_ReadOnly);

            ImGui::End();
        }
    }

    void futil_copy_to_clipboard(HWND hwnd, std::string& data)
    {
        if(OpenClipboard(hwnd) && EmptyClipboard()) {
            if(HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, data.size())) {
                memcpy(GlobalLock(hmem), data.data(), data.size());
                SetClipboardData(CF_TEXT, hmem);
                GlobalFree(hmem);
            }
            CloseClipboard();
        }
    }

    void futil_show_about()
    {
        if(ImGui::Begin(
               "About##AboutWindow", &g_uistate.show_about, ImGuiWindowFlags_NoCollapse)) {
            ImGui::Text("Hello");
            ImGui::End();

            if(ImGui::IsWindowFocused())
                g_uistate.show_about = false;
        }
    }

} // namespace eat