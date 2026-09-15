#include "Core/pch.h"
#include "Lib/stb_image.h"
#include "Resource/HDRLoader.h"

namespace rcore {

  IHDRLoader::PixelComponent* HDRLoader::readTexture(std::string const& path, std::pair<UINT, UINT>* outImageSize) {
    int width, height, channels;
    float* data = stbi_loadf(path.c_str(), &width, &height, &channels, 4);

    if (!data) {
      RCORE_LOG(ERR, "Failed to load texture: " + path);
      return nullptr;
    }

    if (outImageSize) *outImageSize = std::make_pair(static_cast<UINT>(width), static_cast<UINT>(height));

    return data;
  }

}