#pragma once
#include <string> // Include the string header

namespace Interface {
    void RenderMainWindow();
    void RenderEmojiBrowser();
    void RenderMessage(const std::string& message); // Correct declaration
}
