#include <glad/glad.h>
#include "EmojiManager.h"
#include "Interface.h"
#include "Utils.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include "debug/GLog.h" // Ensure the correct header file for GLog is included
#include "debug/GLogMacros.h" // Include macros if required for GLog functionality
#include "debug/GLogMacros.h"
#include <filesystem>
#include <thread>
#include <chrono>
#include <atomic>

void glfw_error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

// Static variables to track window state
static bool isWindowMoving = false;
static bool isWindowFocused = true; // Track if the window is focused
static std::atomic<bool> isIdle(false); // Use atomic for thread-safe idle state

// Free function for window position callback
void WindowPosCallback(GLFWwindow*, int, int)
{
    isWindowMoving = true;
    GLOG_DEBUG("Window is moving.");
}

// Free function for window focus callback
void WindowFocusCallback(GLFWwindow*, int focused)
{
    isWindowFocused = (focused == GLFW_TRUE);
    if (isWindowFocused) {
        isWindowMoving = false; // Reset moving state when the window regains focus
        GLOG_DEBUG("Window regained focus. Resetting isWindowMoving.");
    }
}

int main()
{
    GLog::init("logs.txt");
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        GLOG_ERROR("Failed to initialize GLFW.");
        return -1;
    }

    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "LMS - Let Me Speak", nullptr, nullptr);
    if (!window) {
        GLOG_ERROR("Failed to create GLFW window.");
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        GLOG_ERROR("Failed to initialize OpenGL loader.");
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load emoji metadata
    EmojiManager::LoadEmojiMetadata("assets/emojis/openmoji.json");

    const double idleThreshold = 1.0; // 1 second of inactivity to consider idle
    double lastInteractionTime = glfwGetTime();

    // Set GLFW callbacks
    glfwSetWindowPosCallback(window, WindowPosCallback);
    glfwSetWindowFocusCallback(window, WindowFocusCallback);

    GLOG_INFO("Starting main loop.");
    while (!glfwWindowShouldClose(window))
    {
        // Use glfwWaitEventsTimeout to reduce CPU usage during idle
        glfwWaitEventsTimeout(0.01);

        // Detect idle state
        double currentTime = glfwGetTime();
        if (currentTime - lastInteractionTime > idleThreshold) {
            isIdle = true;
        }

        // Reset idle state on interaction
        if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
            isIdle = false;
            isWindowMoving = false; // Reset moving state on interaction
            lastInteractionTime = currentTime;
            GLOG_DEBUG("User interaction detected. Resetting idle and moving states.");
        }

        // Debug log for state tracking
        GLOG_DEBUG("isIdle: {}, isWindowMoving: {}, isWindowFocused: {}", isIdle.load(), isWindowMoving, isWindowFocused);

        // Skip rendering if the window is not focused
        if (!isWindowFocused) {
            GLOG_DEBUG("Window not focused. Skipping rendering.");
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Example interaction: Reset idle state when a button is pressed
        if (ImGui::Button("Reset Idle State")) {
            isIdle = false;
            isWindowMoving = false; // Reset moving state
            lastInteractionTime = currentTime;
            GLOG_DEBUG("Reset Idle State button pressed.");
        }

        Interface::RenderMainWindow();
        Interface::RenderEmojiBrowser();

        // Clear unused textures periodically to free memory
        static int frameCount = 0;
        if (++frameCount % 300 == 0) {
            GLOG_DEBUG("Clearing unused textures.");
            EmojiManager::ClearUnusedTextures();
        }

        // Render
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        // Limit frame rate
        Utils::LimitFrameRate(60);
    }

    GLOG_INFO("Exiting main loop. Cleaning up resources.");
    // Cleanup
    EmojiManager::CleanupTextures();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    GLog::close();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}