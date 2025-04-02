#include "Interface.h"
#include "EmojiManager.h"
#include "imgui.h"
#include <string>
#include "debug/GLogMacros.h"

namespace Interface {
    void RenderMainWindow()
    {
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
            RenderMessage("Hello there! :grinning_face: How are you?"); // Example with text and emoji
            RenderMessage("I love this! :grinning_face_with_big_eyes:"); // Example with another emoji
            // RenderMessage("Flags are cool! :happy:"); // Example with a flag emoji
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
    }

    void RenderEmojiBrowser()
    {
        ImGui::Begin("Emoji Browser");
        for (const auto& [category, emojis] : EmojiManager::emojiCategories) {
            ImGui::Text("%s", category.c_str());
            ImGui::Separator();

            for (const auto& emoji : emojis) {
                ImGui::Text("%s (%s)", emoji.emoji.c_str(), emoji.annotation.c_str());
                ImGui::SameLine();
            }
            ImGui::NewLine();
        }
        ImGui::End();
    }

    void RenderMessage(const std::string& message)
    {
        std::string processedMessage = message;
        size_t pos = 0;

        ImGui::BeginGroup(); // Group text and images together

        while ((pos = processedMessage.find(':')) != std::string::npos) {
            size_t endPos = processedMessage.find(':', pos + 1);
            if (endPos == std::string::npos) break;

            std::string emojiName = processedMessage.substr(pos, endPos - pos + 1);
            std::string textBeforeEmoji = processedMessage.substr(0, pos);

            // Render text before the emoji
            if (!textBeforeEmoji.empty()) {
                ImGui::TextUnformatted(textBeforeEmoji.c_str());
                ImGui::SameLine(0, 0); // Avoid spacing between text and emoji
            }

            // Render the emoji as an image if it exists
            GLuint emojiTexture = EmojiManager::GetEmojiTexture(emojiName);
            if (emojiTexture != 0) {
                ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<void*>(reinterpret_cast<intptr_t*>(emojiTexture))), ImVec2(20, 20)); // Correctly cast GLuint to ImTextureID
                ImGui::SameLine(0, 0); // Avoid spacing between emoji and next text
            } else {
                // If emoji not found, render the placeholder text
                ImGui::TextUnformatted(emojiName.c_str());
                ImGui::SameLine(0, 0);
            }

            // Update the remaining message
            processedMessage = processedMessage.substr(endPos + 1);
        }

        // Render any remaining text
        if (!processedMessage.empty()) {
            ImGui::TextUnformatted(processedMessage.c_str());
        }

        ImGui::EndGroup();
    }
}
