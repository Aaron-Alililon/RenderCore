#include "Core/pch.h"
#include "Resource/RenderTarget.h"

namespace rcore {

  RenderTarget::RenderTarget(D3D11_TEXTURE2D_DESC const& textureDescriptor, D3D11_SHADER_RESOURCE_VIEW_DESC const& srvDescriptor) : m_faces{ textureDescriptor.ArraySize }, m_mipLevels{ textureDescriptor.MipLevels } {
    if (
      validateDescriptors(textureDescriptor, srvDescriptor) &&
      createTexture(textureDescriptor, srvDescriptor)
    ) {
      m_valid = true;
    }
  }

  void RenderTarget::clearRTV(float r, float g, float b, float a) const {
    if (!m_valid) {
      RCORE_LOG(WARN, "Tried accessing invalid render target");
      return;
    }

    float clearCol[] = { r, g, b, a };
    for (size_t i = 0; i < m_rtvs.size(); i++) D3D11Device::get().rawContext()->ClearRenderTargetView(m_rtvs.at(i).Get(), clearCol);
  }

  ID3D11Texture2D* RenderTarget::getTexture() const {
    if (!m_valid) {
      RCORE_LOG(WARN, "Tried accessing texture of invalid render target");
      return nullptr;
    }

    return m_texture.Get();
  }

  ID3D11ShaderResourceView* RenderTarget::getSRV() const {
    if (!m_valid) {
      RCORE_LOG(WARN, "Tried accessing SRV of invalid render target");
      return nullptr;
    }

    return m_srv.Get();
  }

  ID3D11RenderTargetView* RenderTarget::getRTV(int face, int mip) const {
    if (!m_valid) {
      RCORE_LOG(WARN, "Tried accessing RTV of invalid render target");
      return nullptr;
    }

    int index = face + mip * m_faces;
    return m_rtvs.at(index).Get();
  }

  std::vector<ID3D11RenderTargetView*> RenderTarget::getRTVs() const {
    if (!m_valid) {
      RCORE_LOG(WARN, "Tried accessing RTVs of invalid render target");
      return { };
    }

    std::vector<ID3D11RenderTargetView*> rawVec{ };
    for (size_t i = 0; i < m_rtvs.size(); i++) rawVec.push_back(m_rtvs.at(i).Get());

    return rawVec;
  }

  bool RenderTarget::isValid() const {
    return m_valid;
  }

  bool RenderTarget::validateDescriptors(D3D11_TEXTURE2D_DESC const& textureDescriptor, D3D11_SHADER_RESOURCE_VIEW_DESC const& srvDescriptor) const {
    if (textureDescriptor.ArraySize <= 0) {
      RCORE_LOG(WARN, "Tried creating RenderTarget with 0 faces");
      return false;
    }

    if (textureDescriptor.MipLevels <= 0) {
      RCORE_LOG(WARN, "Tried creating RenderTarget with 0 mip levels");
      return false;
    }

    if (textureDescriptor.Format != srvDescriptor.Format) {
      RCORE_LOG(WARN, "Tried creating RenderTarget with texture/SRV format mismatch");
      return false;
    }

    return true;
  }

  bool RenderTarget::createTexture(D3D11_TEXTURE2D_DESC const& textureDescriptor, D3D11_SHADER_RESOURCE_VIEW_DESC const& srvDescriptor) {
    HRESULT result;
    
    // Texture
    result = D3D11Device::get().raw()->CreateTexture2D(&textureDescriptor, nullptr, &m_texture);
    if (FAILED(result)) {
      RCORE_LOG(ERR, "Failed to create render target texture");
      return false;
    }

    // SRV
    result = D3D11Device::get().raw()->CreateShaderResourceView(m_texture.Get(), &srvDescriptor, &m_srv);
    if (FAILED(result)) {
      RCORE_LOG(ERR, "Failed to create render target SRV");
      return false;
    }

    // RTV
    m_rtvs.resize(m_faces * m_mipLevels);

    for (UINT mip = 0; mip < m_mipLevels; mip++) {
      for (UINT face = 0; face < m_faces; face++) {
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
        rtvDesc.Format = textureDescriptor.Format;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        rtvDesc.Texture2DArray.MipSlice = mip;
        rtvDesc.Texture2DArray.FirstArraySlice = face;
        rtvDesc.Texture2DArray.ArraySize = 1;

        UINT index = face + mip * m_faces;

        result = D3D11Device::get().raw()->CreateRenderTargetView(m_texture.Get(), &rtvDesc, &m_rtvs.at(index));
        if (FAILED(result)) {
          RCORE_LOG(ERR, "Failed to create render target RTV");
          return false;
        }
      }
    }

    return true;
  }

}