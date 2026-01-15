//
// Created by jessy on 1/15/2026.
//

#ifndef IPMEDTH_NFI_RENDERLOOPPROCESS_H
#define IPMEDTH_NFI_RENDERLOOPPROCESS_H

#include "AndroidEngine.h"

#include <EngineUtils/Threading.hpp>
#include <chrono>
#include <functional>

class RenderLoopProcess : public vle::utils::Thread {
public:
    explicit RenderLoopProcess(AndroidEngine* engine);
    ~RenderLoopProcess() = default;

protected:
    void run() override;

private:
    AndroidEngine* _engine = nullptr;

    std::chrono::steady_clock::time_point _currentTime;
};


#endif //IPMEDTH_NFI_RENDERLOOPPROCESS_H
