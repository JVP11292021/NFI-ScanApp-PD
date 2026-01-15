//
// Created by jessy on 1/15/2026.
//

#include "RenderLoopProcess.h"

#include <EngineBackend/FrameInfo.hpp>
#include <stdexcept>

RenderLoopProcess::RenderLoopProcess(AndroidEngine *engine)
    : _engine(engine)
{
    if (!this->_engine)
        throw std::runtime_error("Could not instantiate Render loop process, because AndroidEngine is nullptr");

    this->_currentTime = std::chrono::high_resolution_clock::now();
}

void RenderLoopProcess::run() {
    while(!this->_engine->killLoop()) {
        // Need to poll events from window

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTimeElapsed =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - this->_currentTime).count();
        this->_currentTime = newTime;

        constexpr auto MAX_FRAME_TIME_ELAPSED = 10000.f;
        frameTimeElapsed = glm::min(frameTimeElapsed, MAX_FRAME_TIME_ELAPSED);

        auto aspect = this->_engine->getAspectRatio();

        this->_engine->renderFrame(frameTimeElapsed);
        this->_engine->waitForDevice();
    }
}

