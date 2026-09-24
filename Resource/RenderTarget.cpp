#include "Core/pch.h"
#include "Resource/RenderTarget.h"

namespace rcore {

  RenderTarget::RenderTarget(D3D11_TEXTURE2D_DESC const& textureDescriptor, D3D11_SHADER_RESOURCE_VIEW_DESC const& srvDescriptor) : m_faces{ textureDescriptor.ArraySize }, m_mipLevels{ textureDescriptor.MipLevels } {
    if (
      validateDescriptors(textureDescriptor, srvDescriptor) &&
      createTexture(textureDescriptor, srvDescriptor) &&
      createRTVs()
    ) {
      m_valid = true;
    }
  }

  RenderTarget::RenderTarget(std::filesystem::path const& path) {
    if (
      loadFromFile(path) &&
      createRTVs()
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

  bool RenderTarget::storeAsDDS(std::filesystem::path const& path) const {
    HRESULT result;
    if (!m_valid) {
      RCORE_LOG(WARN, "Tried storing invalid render target");
      return false;
    }

    auto* device = D3D11Device::get().raw();
    auto* ctx = D3D11Device::get().rawContext();

    DirectX::ScratchImage image;
    result = DirectX::CaptureTexture(device, ctx, m_texture.Get(), image);
    if (FAILED(result)) {
      RCORE_LOG(ERR, "An error occured while trying to capture a render target texture");
      return false;
    }

    result = DirectX::SaveToDDSFile(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::DDS_FLAGS_NONE, path.c_str());
    if (FAILED(result)) {
      RCORE_LOG(ERR, "An error occured while trying to save a scratch image to DDS");
      return false;
    }

    return true;
  }

  bool RenderTarget::isValid() const {
    return m_valid;
  }

  bool RenderTarget::validateDescriptors(D3D11_TEXTURE2D_DESC const& textureDescriptor, D3D11_SHADER_RESOURCE_VIEW_DESC const& srvDescriptor) const {
    if (textureDescriptor.ArraySize <= 0) {
      RCORE_LOG(WARN, "Tried creating RenderTarget with 0 faces");
      return false;
    }

    bool autoMips = (textureDescriptor.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS) != 0;
    if (textureDescriptor.MipLevels == 0 && !autoMips) {
      RCORE_LOG(WARN, "Tried creating RenderTarget with 0 mip levels and no auto mip generation");
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
    
    result = D3D11Device::get().raw()->CreateTexture2D(&textureDescriptor, nullptr, &m_texture);
    if (FAILED(result)) {
      RCORE_LOG(ERR, "Failed to create render target texture");
      return false;
    }

    result = D3D11Device::get().raw()->CreateShaderResourceView(m_texture.Get(), &srvDescriptor, &m_srv);
    if (FAILED(result)) {
      RCORE_LOG(ERR, "Failed to create render target SRV");
      return false;
    }

    return true;
  }

  bool RenderTarget::loadFromFile(std::filesystem::path const& path) {
    HRESULT result;
    DirectX::TexMetadata meta{ };
    DirectX::ScratchImage image;

    result = DirectX::LoadFromDDSFile(path.c_str(), DirectX::DDS_FLAGS_NONE, &meta, image);
    if (FAILED(result)) {
      RCORE_LOG(WARN, "Failed to load render target from DDS");
      return false;
    }

    if (meta.dimension != DirectX::TEX_DIMENSION_TEXTURE2D) {
      RCORE_LOG(WARN, "Render target file is not a 2D texture");
      return false;
    }

    m_faces = static_cast<UINT>(meta.arraySize);
    m_mipLevels = static_cast<UINT>(meta.mipLevels);

    Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    result = DirectX::CreateTextureEx(
      D3D11Device::get().raw(), image.GetImages(), image.GetImageCount(), meta,
      D3D11_USAGE_DEFAULT,
      D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
      0, 0, DirectX::CREATETEX_DEFAULT, &resource);

    if (FAILED(result) || FAILED(resource.As(&m_texture))) {
      RCORE_LOG(ERR, "Failed to create render target texture from file");
      return false;
    }

    result = DirectX::CreateShaderResourceView(D3D11Device::get().raw(), image.GetImages(), image.GetImageCount(), meta, &m_srv);
    if (FAILED(result)) {
      RCORE_LOG(ERR, "Failed to create render target SRV from file");
      return false;
    }

    return true;
  }

  bool RenderTarget::createRTVs() {
    D3D11_TEXTURE2D_DESC textureDesc;
    m_texture->GetDesc(&textureDesc);

    UINT guardedMipLevels = (m_mipLevels == 0) ? 1 : m_mipLevels;

    m_rtvs.clear();
    m_rtvs.resize(m_faces * guardedMipLevels);

    for (UINT mip = 0; mip < guardedMipLevels; mip++) {
      for (UINT face = 0; face < m_faces; face++) {
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
        rtvDesc.Format = textureDesc.Format;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        rtvDesc.Texture2DArray.MipSlice = mip;
        rtvDesc.Texture2DArray.FirstArraySlice = face;
        rtvDesc.Texture2DArray.ArraySize = 1;

        UINT index = face + mip * m_faces;

        HRESULT result = D3D11Device::get().raw()->CreateRenderTargetView(m_texture.Get(), &rtvDesc, &m_rtvs.at(index));
        if (FAILED(result)) {
          RCORE_LOG(ERR, "Failed to create render target RTV");
          return false;
        }
      }
    }

    return true;
  }

}