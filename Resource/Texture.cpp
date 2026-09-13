#include "Core/pch.h"
#include "Resource/Texture.h"

namespace rcore {

  ID3D11ShaderResourceView* Texture::getTextureView() const {
    if (!m_valid) {
      RCORE_LOG(WARN, "Tried accessing invalid texture");
      return nullptr;
    }

    return m_textureView.Get();
  }

  UINT Texture::getWidth() const {
    if (!m_valid) {
      RCORE_LOG(WARN, "Tried accessing invalid texture");
      return 0;
    }

    return m_size.first;
  }

  UINT Texture::getHeight() const {
    if (!m_valid) {
      RCORE_LOG(WARN, "Tried accessing invalid texture");
      return 0;
    }

    return m_size.second;
  }

  bool Texture::isValid() const {
    return m_valid;
  }

}