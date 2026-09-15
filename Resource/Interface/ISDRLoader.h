#ifndef I_SDR_LOADER_H
#define I_SDR_LOADER_H

#include "Core/pch.h"
#include "Resource/Interface/ITextureLoader.h"

namespace rcore {

  class ISDRLoader : public ITextureLoader {
  public:
    using PixelComponent = unsigned char;
    virtual PixelComponent* readTexture(std::string const& path, std::pair<UINT, UINT>* outImageSize) = 0;
  };

}

#endif