#include "utils/image.h"
#include "EmojiManager.h"
#include "debug/GLogMacros.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <regex>
#include <filesystem>

namespace EmojiManager {
    const size_t MAX_TEXTURE_CACHE_SIZE = 100; // Limit the number of cached textures
    std::list<std::string> textureUsageOrder; // Track usage order for LRU
    std::unordered_map<std::string, std::vector<EmojiMetadata>> emojiCategories;
    std::unordered_map<std::string, GLuint> emojiTextureCache;
    std::unordered_map<std::string, std::string> emojiNameMap; // Map for :name: to hexcode

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
                if (emojiEntry.contains("hexcode") && emojiEntry.contains("annotation")) {
                    EmojiMetadata metadata;
                    metadata.emoji = emojiEntry.value("emoji", "");
                    metadata.hexcode = emojiEntry.value("hexcode", "");
                    metadata.group = emojiEntry.value("group", "");
                    metadata.subgroups = emojiEntry.value("subgroups", "");
                    metadata.annotation = emojiEntry.value("annotation", "");
                    metadata.tags = emojiEntry.value("tags", "");

                    // Generate shortcut name by replacing spaces with underscores in annotation
                    std::string shortcutName = ":" + metadata.annotation;
                    std::replace(shortcutName.begin(), shortcutName.end(), ' ', '_');
                    shortcutName += ":";

                    // Add to emoji categories
                    emojiCategories[metadata.group].push_back(metadata);

                    // Map shortcut name to hexcode for image loading
                    emojiNameMap[shortcutName] = metadata.hexcode;

                    // Debug log for each emoji added
                    GLOG_DEBUG("Added emoji: {} -> {}", shortcutName, metadata.hexcode);
                }
            }

            GLOG_INFO("Loaded {} emoji categories and {} emoji names.", emojiCategories.size(), emojiNameMap.size());
        } catch (const std::exception& e) {
            GLOG_ERROR("Error parsing emoji metadata: {}", e.what());
        }
    }

    GLuint GetEmojiTexture(const std::string& shortcode)
    {
        if (emojiTextureCache.count(shortcode)) {
            // Move the accessed texture to the front of the usage order
            textureUsageOrder.remove(shortcode);
            textureUsageOrder.push_front(shortcode);
            return emojiTextureCache[shortcode];
        }

        // Retrieve the hexcode from the emojiNameMap
        if (!emojiNameMap.count(shortcode)) {
            GLOG_ERROR("Emoji shortcode not found: {}", shortcode);
            return 0;
        }

        std::string hexcode = emojiNameMap[shortcode];
        std::transform(hexcode.begin(), hexcode.end(), hexcode.begin(), ::tolower); // Ensure lowercase
        std::string filePath = "assets/emojis/" + hexcode + ".png";

        if (!std::filesystem::exists(filePath)) {
            GLOG_ERROR("Emoji texture file not found: {}", filePath);
            return 0;
        }

        GLuint texID = LoadTextureFromFile(filePath.c_str());
        if (texID == 0) {
            GLOG_ERROR("Failed to load texture for: {}", shortcode);
            return 0;
        }

        // Add the texture to the cache
        emojiTextureCache[shortcode] = texID;
        textureUsageOrder.push_front(shortcode);

        // If the cache exceeds the limit, remove the least recently used texture
        if (emojiTextureCache.size() > MAX_TEXTURE_CACHE_SIZE) {
            std::string lru = textureUsageOrder.back();
            glDeleteTextures(1, &emojiTextureCache[lru]);
            emojiTextureCache.erase(lru);
            textureUsageOrder.pop_back();
        }

        return texID;
    }

    void ClearUnusedTextures()
    {
        // Clear all textures from the cache
        for (const auto& [shortcode, texID] : emojiTextureCache) {
            glDeleteTextures(1, &texID);
        }
        emojiTextureCache.clear();
        textureUsageOrder.clear();
    }

    void CleanupTextures()
    {
        ClearUnusedTextures();
    }

    std::string ReplaceEmojiNames(const std::string& text)
    {
        std::string result;
        size_t start = 0, pos;

        while ((pos = text.find(':', start)) != std::string::npos) {
            result += text.substr(start, pos - start); // Add text before emoji
            size_t endPos = text.find(':', pos + 1);
            if (endPos == std::string::npos) break;

            std::string emojiName = text.substr(pos, endPos - pos + 1);
            if (emojiNameMap.count(emojiName)) {
                result += emojiNameMap[emojiName]; // Replace with emoji
            } else {
                result += emojiName; // Keep the original text if not found
            }
            start = endPos + 1;
        }

        result += text.substr(start); // Add remaining text
        return result;
    }
}
