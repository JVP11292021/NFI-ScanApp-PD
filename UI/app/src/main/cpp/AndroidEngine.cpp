//
// Created by jessy on 1/13/2026.
//

#include "AndroidEngine.h"

#include <cstdint>

AndroidEngine::AndroidEngine(
        ANativeWindow* nativeWindow,
        std::uint32_t width,
        std::uint32_t height
)
    : _win(nativeWindow, width, height, "NFI Scan App"),
      _device(_win),
      _renderer(_win, _device)
{

}

AndroidEngine::~AndroidEngine() {}

void AndroidEngine::resize(std::uint32_t width, std::uint32_t height) {

}