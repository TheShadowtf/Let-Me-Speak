#include "Utils.h"
#include <GLFW/glfw3.h>
#include <thread>
#include <chrono>

namespace Utils {
    void LimitFrameRate(int targetFPS)
    {
        static auto lastTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto frameTime = std::chrono::duration<double>(1.0 / targetFPS);

        auto elapsed = currentTime - lastTime;
        if (elapsed < frameTime) {
            std::this_thread::sleep_for(frameTime - elapsed);
        }
        lastTime = std::chrono::high_resolution_clock::now();
    }
}
