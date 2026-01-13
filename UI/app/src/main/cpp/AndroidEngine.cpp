//
// Created by jessy on 1/13/2026.
//

#include "AndroidEngine.h"

#include <cstdint>

static constexpr std::int32_t WIDTH = 800;
static constexpr std::int32_t HEIGHT = 600;

AndroidEngine::AndroidEngine(ANativeWindow* nativeWindow)
    : _win(nativeWindow, WIDTH, HEIGHT, "NFI Scan App"),
      _device(_win),
      _renderer(_win, _device)
{

}

AndroidEngine::~AndroidEngine() {}
