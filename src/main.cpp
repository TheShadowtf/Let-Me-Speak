#include "utils/image.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include "stb_image.h"
#include <unordered_map>
#include <regex>
#include <filesystem>
#include "debug/GLogMacros.h"
#include <fstream>
#include <nlohmann/json.hpp> // Include the JSON library (e.g., nlohmann/json)

std::unordered_map<std::string, GLuint> emojiMap;
std::unordered_map<std::string, std::string> emojiDescriptions = {
    {"happy", ":emoji_1f600:"},  // Happy face emoji
    {"love", ":emoji_2764:"},    // Red heart emoji
    {"thumbs_up", ":emoji_1f44d:"}, // Thumbs up emoji
    // Add more descriptions here...
};

// Struct to hold emoji metadata
struct EmojiMetadata {
    std::string emoji;
    std::string hexcode;
    std::string group;
    std::string subgroups;
    std::string annotation;
    std::string tags;
};

// Map to store emojis grouped by categories
std::unordered_map<std::string, std::vector<EmojiMetadata>> emojiCategories;

// Function to load emoji metadata from openmoji.json
void LoadEmojiMetadata(const std::string& jsonFilePath)
{
    std::ifstream file(jsonFilePath);
    if (!file.is_open()) {
        GLOG_ERROR("Failed to open emoji metadata file: {}", jsonFilePath);
        return;
    }

    try {
        nlohmann::json jsonData;
        file >> jsonData;

        for (const auto& emojiEntry : jsonData) {
            if (emojiEntry.contains("emoji") && emojiEntry.contains("group")) {
                EmojiMetadata metadata;
                metadata.emoji = emojiEntry.value("emoji", "");
                metadata.hexcode = emojiEntry.value("hexcode", "");
                metadata.group = emojiEntry.value("group", "");
                metadata.subgroups = emojiEntry.value("subgroups", "");
                metadata.annotation = emojiEntry.value("annotation", "");
                metadata.tags = emojiEntry.value("tags", "");

                emojiCategories[metadata.group].push_back(metadata);
            }
        }

        GLOG_INFO("Loaded {} emoji categories from metadata.", emojiCategories.size());
    } catch (const std::exception& e) {
        GLOG_ERROR("Error parsing emoji metadata: {}", e.what());
    }
}

// Function to display emojis grouped by categories
void DisplayEmojiCategories()
{
    for (const auto& [category, emojis] : emojiCategories) {
        ImGui::Text("%s", category.c_str());
        ImGui::Separator();

        for (const auto& emoji : emojis) {
            ImGui::Text("%s (%s)", emoji.emoji.c_str(), emoji.annotation.c_str());
            ImGui::SameLine();
        }
        ImGui::NewLine();
    }
}

// Optimized function to load all emoji images from the "assets/emojis" directory
void LoadAllEmojis()
{
    std::string path = "assets/emojis";

    // Check if the path exists
    if (!std::filesystem::exists(path)) {
        GLOG_ERROR("Emoji folder path not found: {}", path);
        return;
    }

    // Iterate through all files in the emoji folder
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();

            // Filter by valid image extensions
            if (filename.find(".png") != std::string::npos || filename.find(".jpg") != std::string::npos) {
                std::string base = filename.substr(0, filename.find_last_of('.')); // Extract filename without extension
                std::transform(base.begin(), base.end(), base.begin(), ::tolower); // Ensure lowercase
                std::replace(base.begin(), base.end(), '-', '_'); // Replace hyphens with underscores
                std::string shortcode = ":emoji_" + base + ":"; // Generate shortcode

                GLuint texID = LoadTextureFromFile(entry.path().string().c_str());
                if (texID == 0) {
                    GLOG_ERROR("Failed to load texture for: {}", shortcode);
                } else {
                    emojiMap[shortcode] = texID; // Map shortcode to texture ID
                }
            }
        }
    }

    GLOG_INFO("Loaded {} emojis.", emojiMap.size());
}

// Simplified function to render an emoji by its shortcode
void RenderMessage(const std::string& msg)
{
    std::regex emojiRegex("(:emoji_[a-z0-9_]+:)"); // Regex to match emoji shortcodes
    std::sregex_iterator it(msg.begin(), msg.end(), emojiRegex);
    std::sregex_iterator end;

    size_t last = 0;
    for (; it != end; ++it)
    {
        // Render text before the emoji
        std::string before = msg.substr(last, it->position() - last);
        if (!before.empty()) {
            ImGui::TextUnformatted(before.c_str());
            ImGui::SameLine(0, 2.0f);
        }

        // Render the emoji
        std::string shortcode = it->str();
        if (emojiMap.count(shortcode)) {
            GLuint texID = emojiMap[shortcode];
            if (texID) {
                ImGui::Image((ImTextureID)(intptr_t)texID, ImVec2(20, 20)); // Render emoji at 20x20 size
                ImGui::SameLine(0, 2.0f);
            } else {
                GLOG_ERROR("Invalid texture ID for emoji: {}", shortcode);
            }
        } else {
            GLOG_ERROR("Emoji shortcode not found: {}", shortcode);
            ImGui::TextUnformatted(shortcode.c_str()); // Fallback: Render the shortcode as text
            ImGui::SameLine(0, 2.0f);
        }

        last = it->position() + it->length();
    }

    // Render remaining text after the last emoji
    std::string after = msg.substr(last);
    if (!after.empty()) {
        ImGui::TextUnformatted(after.c_str());
    }
}

void glfw_error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

int main()
{
    GLog::init("logs.txt");
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return -1;

    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "LMS - Let Me Speak", nullptr, nullptr);
    if (!window)
        return -1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize OpenGL loader!" << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    LoadAllEmojis();
    LoadEmojiMetadata("assets/emojis/openmoji.json");

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 🔄 State Management
        enum class PanelMode
        {
            ChannelView,
            FriendsView
        };
        static PanelMode panelMode = PanelMode::ChannelView;
        static std::string selectedFriend = "";

        // === Main Window Setup ===
        ImGui::Begin("MainWindow", nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove);
        ImGui::SetWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::SetWindowPos(ImVec2(0, 0));

        float winW = ImGui::GetContentRegionAvail().x;
        float winH = ImGui::GetContentRegionAvail().y;
        float col1W = 250.0f;
        float col2W = 250.0f;
        float col3W = winW - col1W - col2W;

        // === LEFT PANEL: Friends Toggle + Channels ===
        ImGui::BeginChild("LeftPanel", ImVec2(col1W, winH), true);
        if (ImGui::Button("💬 Friends", ImVec2(-1, 0)))
        {
            panelMode = PanelMode::FriendsView;
        }
        ImGui::Separator();
        ImGui::Text("Channels");
        if (ImGui::Button("# gaming", ImVec2(-1, 0)))
        {
            panelMode = PanelMode::ChannelView;
        }
        if (ImGui::Button("# music", ImVec2(-1, 0)))
        {
            panelMode = PanelMode::ChannelView;
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // === MIDDLE PANEL: Friend List or Voice List ===
        ImGui::BeginChild("MiddlePanel", ImVec2(col2W, winH), true);
        if (panelMode == PanelMode::ChannelView)
        {
            ImGui::Text("On Voice:");
            ImGui::Separator();
            ImGui::Text("User1 🎙️");
            ImGui::Text("User2 🔇");
        }
        else if (panelMode == PanelMode::FriendsView)
        {
            ImGui::Text("Your Friends:");
            ImGui::Separator();
            if (ImGui::Button("Friend A", ImVec2(-1, 0)))
            {
                selectedFriend = "Friend A";
            }
            if (ImGui::Button("Friend B", ImVec2(-1, 0)))
            {
                selectedFriend = "Friend B";
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // === RIGHT PANEL: Chat with selected friend (if any) ===
        ImGui::BeginChild("RightPanel", ImVec2(col3W, winH), true);

        if (panelMode == PanelMode::FriendsView && !selectedFriend.empty())
        {
            ImGui::Text("%s", selectedFriend.c_str());
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 80);
            if (ImGui::Button("📞 Call"))
            {
                // Call logic here
            }
            ImGui::Separator();
        }

        ImGui::BeginChild("ChatLog", ImVec2(0, winH - 60), true);

        if (panelMode == PanelMode::FriendsView && !selectedFriend.empty())
        {
            ImGui::TextWrapped("%s: Hello!", selectedFriend.c_str());
            RenderMessage("Hello there! :emoji_1f600: How are you?"); // Example with text and emoji
            RenderMessage("I love this! :emoji_2764:"); // Example with another emoji
            RenderMessage("Flags are cool! :emoji_1F0CF:"); // Example with a flag emoji
        }
        else
        {
            ImGui::TextDisabled("Select a friend to view the conversation.");
        }

        ImGui::EndChild(); // ChatLog

        // === Chat Input ===
        static char inputBuf[256] = "";
        ImGui::SetCursorPosY(winH - 25);
        ImGui::PushItemWidth(-1);
        if (ImGui::InputText("##ChatInput", inputBuf, IM_ARRAYSIZE(inputBuf),
                             ImGuiInputTextFlags_EnterReturnsTrue))
        {
            // TODO: send message
            inputBuf[0] = '\0';
        }
        ImGui::PopItemWidth();

        ImGui::EndChild(); // RightPanel

        ImGui::End(); // MainWindow

        ImGui::Begin("Emoji Browser");
        DisplayEmojiCategories(); // Display emojis grouped by categories
        ImGui::End();

        // === Render ===
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    GLog::close();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}