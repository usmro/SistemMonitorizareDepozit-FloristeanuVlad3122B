#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>

#include "data/Database.h"
#include "core/Depozit.h"
#include "ui/LoginUI.h"
#include "ui/MainUI.h"

int main() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1400, 800,
        "Sistem Monitorizare Depozit", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.FontGlobalScale = 1.5f;

    // ===== TEMA DARK BLUE =====
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_WindowBg]             = ImVec4(0.06f, 0.08f, 0.14f, 1.00f);
    colors[ImGuiCol_ChildBg]              = ImVec4(0.08f, 0.10f, 0.18f, 1.00f);
    colors[ImGuiCol_PopupBg]              = ImVec4(0.08f, 0.10f, 0.18f, 1.00f);
    colors[ImGuiCol_Header]               = ImVec4(0.15f, 0.30f, 0.55f, 1.00f);
    colors[ImGuiCol_HeaderHovered]        = ImVec4(0.20f, 0.40f, 0.70f, 1.00f);
    colors[ImGuiCol_HeaderActive]         = ImVec4(0.25f, 0.45f, 0.75f, 1.00f);
    colors[ImGuiCol_Button]               = ImVec4(0.15f, 0.30f, 0.55f, 1.00f);
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.20f, 0.40f, 0.70f, 1.00f);
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.25f, 0.50f, 0.80f, 1.00f);
    colors[ImGuiCol_FrameBg]              = ImVec4(0.10f, 0.15f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.15f, 0.22f, 0.35f, 1.00f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.18f, 0.27f, 0.42f, 1.00f);
    colors[ImGuiCol_TitleBg]              = ImVec4(0.05f, 0.08f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.08f, 0.14f, 0.28f, 1.00f);
    colors[ImGuiCol_Separator]            = ImVec4(0.20f, 0.40f, 0.70f, 0.80f);
    colors[ImGuiCol_Border]               = ImVec4(0.15f, 0.30f, 0.55f, 0.60f);
    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.05f, 0.08f, 0.15f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.15f, 0.30f, 0.55f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.20f, 0.40f, 0.70f, 1.00f);
    colors[ImGuiCol_Tab]                  = ImVec4(0.10f, 0.18f, 0.35f, 1.00f);
    colors[ImGuiCol_TabHovered]           = ImVec4(0.20f, 0.40f, 0.70f, 1.00f);
    colors[ImGuiCol_TabActive]            = ImVec4(0.15f, 0.30f, 0.60f, 1.00f);
    colors[ImGuiCol_Text]                 = ImVec4(0.85f, 0.90f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]         = ImVec4(0.40f, 0.50f, 0.65f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]        = ImVec4(0.10f, 0.18f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight]     = ImVec4(0.15f, 0.25f, 0.45f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt]        = ImVec4(0.08f, 0.12f, 0.22f, 1.00f);
    colors[ImGuiCol_NavHighlight]         = ImVec4(0.25f, 0.50f, 0.90f, 1.00f);

    style.WindowRounding    = 6.0f;
    style.FrameRounding     = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding      = 4.0f;
    style.TabRounding       = 4.0f;
    style.FramePadding      = ImVec2(8, 4);
    style.ItemSpacing       = ImVec2(8, 6);
    style.WindowPadding     = ImVec2(12, 12);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    Database db("depozit.db");
    if (!db.conecteaza()) {
        std::cerr << "Eroare la conectarea bazei de date!" << std::endl;
        return -1;
    }

    LoginUI loginUI(db);
    MainUI* mainUI = nullptr;
    Depozit* depozit = nullptr;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (!loginUI.eAutentificat()) {
            loginUI.render();
        } else {
            if (!mainUI) {
                depozit = new Depozit(db);
                mainUI = new MainUI(db, *depozit, loginUI.getUserCurent());
            }
            mainUI->render();

            if (mainUI->trebuieLogout()) {
                delete mainUI;
                delete depozit;
                mainUI = nullptr;
                depozit = nullptr;
                loginUI.reset();
            }
        }

        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.06f, 0.08f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    delete mainUI;
    delete depozit;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}