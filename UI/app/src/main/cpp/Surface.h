//
// Created by jessy on 1/18/2026.
//

#ifndef IPMEDTH_NFI_SURFACE_H
#define IPMEDTH_NFI_SURFACE_H

#include <cstdint>
#include <android/asset_manager.h>
#include <android/native_window.h>

/*
 * This class encompasses lifecycle hooks of the Android surface view. These hooks are
 * surfaceCreated = IAndroidSurface::IAndroidSurface(...)
 * surfaceChanged = IAndroidSurface::resize(...)
 * surfaceDestroyed = IAndroidSurface::~IAndroidSurface( )
 * surfaceRedrawNeeded = IAndroidSurface::drawFrame( )
 * */
class IAndroidSurface {
public:
    IAndroidSurface() = default;
    virtual ~IAndroidSurface() = default;

public:
    virtual void resize(std::int32_t width, std::int32_t height) = 0;
    virtual void drawFrame() = 0;
};

#endif //IPMEDTH_NFI_SURFACE_H
