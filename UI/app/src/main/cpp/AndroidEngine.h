//
// Created by jessy on 1/13/2026.
//

#ifndef IPMEDTH_NFI_ANDROIDENGINE_H
#define IPMEDTH_NFI_ANDROIDENGINE_H

#include <EngineBackend/defs.hpp>
#include <EngineBackend/AndroidWindow.hpp>
#include <EngineBackend/Device.hpp>
#include <Systems/Renderer.hpp>

class AndroidEngine final {
public:
    explicit AndroidEngine(
            ANativeWindow* nativeWindow,
            std::uint32_t width,
            std::uint32_t height);
    ~AndroidEngine();

public:
    void resize(std::uint32_t width, std::uint32_t height);

private:
    vle::AndroidWindow _win;
    vle::EngineDevice _device;
    vle::sys::Renderer _renderer;
};


#endif //IPMEDTH_NFI_ANDROIDENGINE_H
