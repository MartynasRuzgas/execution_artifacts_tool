/*
Copyright (c) 2019 Martynas Ruzgas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "eat.hpp"
#include "eat_output.hpp"
#include <imgui.h>
#include <imgui_internal.h>

#include <deque>
#include <fstream>
#include <Psapi.h>
#include <winuser.h>
#include <VersionHelpers.h>

#pragma comment(lib, "User32.lib")

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

    std::string futil_get_user_save_path(HWND hwnd);
    void        futil_copy_to_clipboard(HWND hwnd, std::string& data);
    void        futil_show_about(bool& show_about);
    void        futil_show_memory_stats(std::deque<float>& memory_usage_history);

    void on_frame(HWND hwnd)
    {
        ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
        if(ImGui::Begin("MainWindow",
                        nullptr,
                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar)) {
            static std::string last_save_location            = "";
            static std::string last_artifact_filename        = "";
            static std::string input_text_buffer             = "Ready.";
            static bool        show_about                    = false;
            static bool        is_min_windows_10             = IsWindows10OrGreater();
            static bool        auto_save_result_to_file      = false;
            static bool        auto_copy_result_to_clipboard = false;
            static bool        dont_display_result           = false;

            auto lmbd_futil_write_result_to_file = [&](bool working_dir = false) {
                if(!working_dir && !last_save_location.empty()) {
                    std::ofstream fout(last_save_location);
                    if(!fout.is_open())
                        MessageBoxA(hwnd, "Failed to open file.", "EAT - Error", MB_OK);
                    fout << input_text_buffer;
                }
                else {
                    std::ofstream fout(last_artifact_filename + ".txt");
                    if(!fout.is_open())
                        MessageBoxA(hwnd, "Failed to open file.", "EAT - Error", MB_OK);
                    fout << input_text_buffer;
                }
            };

            // Lambda to be called after we query an artifact.
            auto lmbd_futil_postprocess = [&](const char* filename) {
                last_artifact_filename = filename;

                if(auto_save_result_to_file)
                    lmbd_futil_write_result_to_file(true);

                if(auto_copy_result_to_clipboard)
                    futil_copy_to_clipboard(hwnd, input_text_buffer);

                if(dont_display_result) {
                    input_text_buffer._Tidy_deallocate();
                    input_text_buffer =
                        "Results were not displayed. You may re-enable them in the options tree.";
                }
            };

            // Do the menubar
            if(ImGui::BeginMenuBar()) {
                if(ImGui::BeginMenu("File")) {
                    bool empty = last_artifact_filename.empty();

                    if(ImGui::MenuItem(("Save " + last_artifact_filename).c_str()) &&
                       !empty) {
                        lmbd_futil_write_result_to_file();
                    }

                    if(empty && ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Query an artifact first.");
                    }

                    if(ImGui::MenuItem(("Save " + last_artifact_filename +
                                        (empty ? "as..." : " as..."))
                                           .c_str()) &&
                       !empty) {
                        last_save_location = futil_get_user_save_path(hwnd);
                        if(!last_save_location.empty())
                            lmbd_futil_write_result_to_file();
                    }

                    if(empty && ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Query an artifact first.");
                    }

                    if(ImGui::MenuItem("Copy All To Clipboard")) {
                        futil_copy_to_clipboard(hwnd, input_text_buffer);
                    }
                    ImGui::EndMenu();
                }
                if(ImGui::BeginMenu("Info##Menu")) {
                    if(ImGui::MenuItem("About")) {
                        show_about = true;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            // Do the about window
            if(show_about)
                futil_show_about(show_about);

            // Do the memory stats
            static std::deque<float> memory_usage_history;
            if(ImGui::TreeNode("Memory Stats")) {
                futil_show_memory_stats(memory_usage_history);
                ImGui::TreePop();
            }
            else
                // Clear the history if we didn't show it.
                memory_usage_history.clear();

            // Do the options menu
            if(ImGui::TreeNode("Options")) {
                ImGui::Checkbox("Auto save result to file", &auto_save_result_to_file);
                ImGui::Checkbox("Auto copy result to clipboard",
                                &auto_copy_result_to_clipboard);
                ImGui::Checkbox("Don't display results", &dont_display_result);
                ImGui::TreePop();
            }

            // Lambda to fit elements in the same line ONLY if they fit in the frame.
            auto lmbd_futil_wrap_sameline = [&](const char* prev_button_name) {
                ImGuiStyle& style = ImGui::GetStyle();
                if(ImGui::GetItemRectMax().x + style.ItemSpacing.x +
                       ImGui::CalcTextSize(prev_button_name).x +
                       style.FramePadding.x * 2.f <
                   ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x)
                    ImGui::SameLine();
            };

            if(ImGui::Button("< Clear Screen >")) {
                input_text_buffer._Tidy_deallocate();
                input_text_buffer = "Cleared.";
            }
            lmbd_futil_wrap_sameline("< Clear Screen >");

            { // UsnJournal
                static bool        usn_query_completed = false;
                static std::string usn_result          = "";
                if(ImGui::Button("UsnJournal")) {
                    if(MessageBoxA(
                           hwnd,
                           "This operation can take a while and use a bunch of cpu/memory resources. You can track the usage in the Memory Stats.\nContinue?",
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
                    lmbd_futil_postprocess("UsnJournal.txt");
                }
            }
            lmbd_futil_wrap_sameline("UsnJournal");

            if(ImGui::Button("UserAssist")) {
                input_text_buffer = eat::get_user_assist_info();
                lmbd_futil_postprocess("UserAssist.txt");
            }
            lmbd_futil_wrap_sameline("UserAssist");

            if(ImGui::Button("AppCompatFlags")) {
                input_text_buffer = eat::get_app_compat_flags_info();
                lmbd_futil_postprocess("AppCompatFlags.txt");
            }
            lmbd_futil_wrap_sameline("AppCompatFlags");

            if(ImGui::Button("MUI Cache")) {
                input_text_buffer = eat::get_mui_cache_info();
                lmbd_futil_postprocess("MUI Cache.txt");
            }
            lmbd_futil_wrap_sameline("MUI Cache");

            if(ImGui::Button("RecentApps (W10+)") && is_min_windows_10) {
                input_text_buffer = eat::get_recent_apps_info();
                lmbd_futil_postprocess("RecentApps.txt");
            }
            lmbd_futil_wrap_sameline("RecentApps (W10+)");

            if(ImGui::Button("Shim/AppCompat(Cache)")) {
                input_text_buffer = eat::get_shim_cache_info();
                lmbd_futil_postprocess("Shim_AppCompat(Cache).txt");
            }
            lmbd_futil_wrap_sameline("Shim/AppCompat(Cache)");

            if(ImGui::Button("RunMRU")) {
                input_text_buffer = eat::get_run_mru();
                lmbd_futil_postprocess("RunMRU.txt");
            }
            lmbd_futil_wrap_sameline("RunMRU");

            if(ImGui::Button("RecentDocsMRU")) {
                input_text_buffer = eat::get_recent_docs_mru();
                lmbd_futil_postprocess("RecentDocsMRU.txt");
            }

            ImGui::InputTextMultiline("##EatOutputTextBox",
                                      const_cast<char*>(input_text_buffer.c_str()),
                                      input_text_buffer.size(),
                                      ImVec2(-1.f, -1.f),
                                      ImGuiInputTextFlags_ReadOnly);

            ImGui::End();
        }
    }

    std::string futil_get_user_save_path(HWND hwnd)
    {
        char filename[MAX_PATH] = { 0 };

        OPENFILENAME ofn = { 0 };
        ofn.lStructSize  = sizeof(ofn);
        ofn.hwndOwner    = hwnd;
        ofn.lpstrFilter  = "Text Files\0*.txt\0Any File\0*.*\0";
        ofn.lpstrFile    = filename;
        ofn.nMaxFile     = MAX_PATH;
        ofn.lpstrTitle   = "Save File As";
        ofn.Flags        = OFN_DONTADDTORECENT;

        return GetOpenFileNameA(&ofn) ? filename : "";
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

    void futil_show_about(bool& show_about)
    {
        ImGui::SetNextWindowSize(ImVec2(580.f, 156.f), ImGuiCond_Once);
        if(ImGui::Begin("About##AboutWindow", &show_about, ImGuiWindowFlags_NoCollapse)) {
            ImGui::Text(
                "An Open-Source, purely C++ Microsoft Windows execution/interaction artifact tool.");
            ImGui::Text("");
            ImGui::Text("Dear ImGui By Omar Cornut and all dear imgui contributors,");
            ImGui::Text("Execution Artifacts By Justas Masiulis and Martynas Ruzgas.");
            ImGui::Text("");
            ImGui::Text("This software is licensed under the MIT license,");
            if(ImGui::Button("Check out the source code on github!")) {
                ShellExecuteW(
                    0,
                    0,
                    L"https://github.com/MartynasRuzgas/execution_artifacts_tool",
                    0,
                    0,
                    SW_SHOW);
            }

            ImGui::End();

            if(ImGui::IsWindowFocused())
                show_about = false;
        }
    }

    void futil_show_memory_stats(std::deque<float>& memory_usage_history)
    {
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if(GetProcessMemoryInfo(
               GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            const auto     private_usage_mb = pmc.PrivateUsage / 1024 / 1024.f;
            constexpr auto kDisplayCount    = 120;

            ImGui::Text("Process Memory: %0.2fMB", private_usage_mb);

            memory_usage_history.emplace_back(private_usage_mb);
            if(memory_usage_history.size() > kDisplayCount)
                memory_usage_history.pop_front();

            auto value_getter = [](void* pmemory_usage_history, int i) -> float {
                auto& muh = *(decltype(&memory_usage_history))pmemory_usage_history;
                if(i > muh.size() - 1)
                    return 0.f;
                return muh[i];
            };

            // Get the peak working set alligned to power of 2's.
            int64_t alligned_pws = 0x1000;
            for(; alligned_pws < pmc.PeakWorkingSetSize / 1024; alligned_pws *= 2)
                ;

            std::string overlay_text =
                "Scale: " + std::to_string(alligned_pws / 1024) + "MB";

            ImGui::PlotLines("##WorkingSetGraph",
                             value_getter,
                             &memory_usage_history,
                             kDisplayCount,
                             0,
                             overlay_text.c_str(),
                             0.f,
                             alligned_pws / 1024.f,
                             ImVec2(0.f, 70));
        }
    }

} // namespace eat