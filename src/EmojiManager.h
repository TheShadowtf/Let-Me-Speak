#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <GLFW/glfw3.h>

struct EmojiMetadata {
    std::string emoji;
    std::string hexcode;
    std::string group;
    std::string subgroups;
    std::string annotation;
    std::string tags;
};

namespace EmojiManager {
    extern std::unordered_map<std::string, std::vector<EmojiMetadata>> emojiCategories;
    extern std::unordered_map<std::string, std::string> emojiNameMap; // Map for :name: to emoji

    void LoadEmojiMetadata(const std::string& jsonFilePath);
    void PreloadFrequentlyUsedEmojis();
    GLuint GetEmojiTexture(const std::string& shortcode);
    void ClearUnusedTextures();
    void CleanupTextures();
    std::string ReplaceEmojiNames(const std::string& text); // Replace :name: with emoji
}
