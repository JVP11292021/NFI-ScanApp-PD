//
// Created by jessy on 1/13/2026.
//

#ifndef IPMEDTH_NFI_ANDROIDENGINE_H
#define IPMEDTH_NFI_ANDROIDENGINE_H

#include "Semantics.h"

#include <EngineBackend/defs.hpp>
#include <EngineBackend/AndroidWindow.hpp>
#include <EngineBackend/Device.hpp>
#include <EngineBackend/Descriptors.hpp>
#include <EngineBackend/Model.hpp>
#include <EngineBackend/Object.hpp>
#include <Systems/Renderer.hpp>

using UboBuffer = vle::Buffer;

class AndroidEngine final {
public:
    explicit AndroidEngine(
            ANativeWindow* nativeWindow,
            std::uint32_t width,
            std::uint32_t height);
    ~AndroidEngine();

    NON_COPYABLE(AndroidEngine)
public:
    // TODO needs to be implemented
    void resize(std::uint32_t width, std::uint32_t height);
    void mapUniformBufferObjects();
    void makeDescriptorSets();

private:
    vle::AndroidWindow _win;
    vle::EngineDevice _device;
    vle::sys::Renderer _renderer;

public:
    std::unique_ptr<vle::DescriptorPool> globalPool{};
    vle::ObjectMap objects;
    vle::ObjectMap points;

};


#endif //IPMEDTH_NFI_ANDROIDENGINE_H
