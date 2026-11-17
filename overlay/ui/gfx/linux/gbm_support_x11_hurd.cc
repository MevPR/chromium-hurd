// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// GBMSupportX11 for GNU/Hurd. There are no DRM render nodes (and no GBM) on
// the Hurd, so this behaves exactly like the Linux implementation does when
// no GBM device can be opened: native pixmaps are unsupported.

#include "ui/gfx/linux/gbm_support_x11.h"

#include "base/no_destructor.h"
#include "build/build_config.h"
#include "ui/gfx/linux/gbm_buffer.h"
#include "ui/gfx/linux/gbm_device.h"

static_assert(BUILDFLAG(IS_HURD), "Only for GNU/Hurd");

namespace ui {

// static
GBMSupportX11* GBMSupportX11::GetInstance() {
  static base::NoDestructor<GBMSupportX11> instance;
  return instance.get();
}

GBMSupportX11::GBMSupportX11() = default;

GBMSupportX11::~GBMSupportX11() = default;

std::unique_ptr<GbmBuffer> GBMSupportX11::CreateBuffer(
    viz::SharedImageFormat format,
    const gfx::Size& size,
    gfx::BufferUsage usage) {
  return nullptr;
}

bool GBMSupportX11::CanCreateBufferForFormat(viz::SharedImageFormat format) {
  return false;
}

std::unique_ptr<GbmBuffer> GBMSupportX11::CreateBufferFromHandle(
    const gfx::Size& size,
    viz::SharedImageFormat format,
    gfx::NativePixmapHandle handle) {
  return nullptr;
}

}  // namespace ui
