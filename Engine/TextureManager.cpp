#include "TextureManager.h"

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
#include <windows.h>
#include <Logger.h>
#include <DirectXCommon.h>
#include <Console.h>

//-----------------------------------------------------------------------------------------
// using
//-----------------------------------------------------------------------------------------
using namespace DxObject;

////////////////////////////////////////////////////////////////////////////////////////////
// Texture class
////////////////////////////////////////////////////////////////////////////////////////////

void Texture::Load(const std::string& filePath, DirectXCommon* dxCommon) {

	// dxCommonの保存
	dxCommon_ = dxCommon;

	// デバイス, CommandListの取り出し
	ID3D12Device* device = dxCommon_->GetDeviceObj()->GetDevice();
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	DirectX::ScratchImage mipImage = TextureMethod::LoadTexture(filePath);
	const DirectX::TexMetadata metadata = mipImage.GetMetadata();

	resource_ = TextureMethod::CreateTextureResource(device, metadata);
	intermediateResouce_ = TextureMethod::UploadTextureData(resource_.Get(), mipImage, device, commandList);

	// SRV - shaderResourceViewの生成
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.Format                  = metadata.format;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels     = UINT(metadata.mipLevels);

		// SRVを生成するDescriptorHeapの場所を決める
		descriptorSRV_ = dxCommon_->GetDescriptorsObj()->GetCurrentDescriptor(DxObject::DescriptorType::SRV);

		// SRVの生成
		device->CreateShaderResourceView(
			resource_.Get(),
			&desc,
			descriptorSRV_.handleCPU
		);
	}
}

void Texture::Unload() {
	dxCommon_->GetDescriptorsObj()->Erase(descriptorSRV_);
}

////////////////////////////////////////////////////////////////////////////////////////////
// RenderTexture class methods
////////////////////////////////////////////////////////////////////////////////////////////

void RenderTexture::Create(int32_t width, int32_t height, DirectXCommon* dxCommon) {

	// dxCommonの保存
	dxCommon_ = dxCommon;

	// deviceの取り出し
	auto device = dxCommon_->GetDeviceObj()->GetDevice();

	// resourceの生成
	{
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width            = width;
		desc.Height           = height;
		desc.MipLevels        = 1;
		desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.Flags            = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		desc.SampleDesc.Count = 1;
		desc.DepthOrArraySize = 1;

		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_DEFAULT;

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format   = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		clearValue.Color[0] = 0.1f;
		clearValue.Color[1] = 0.25f;
		clearValue.Color[2] = 0.5f;
		clearValue.Color[3] = 1.0f;

		device->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&desc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&clearValue,
			IID_PPV_ARGS(&resource_)
		);
	}

	// SRV - shaderResourceViewの生成
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels     = 1;

		// SRVを生成するDescriptorHeapの場所を決める
		descriptorSRV_ = dxCommon_->GetDescriptorsObj()->GetCurrentDescriptor(DxObject::DescriptorType::SRV);

		// SRVの生成
		device->CreateShaderResourceView(
			resource_.Get(),
			&desc,
			descriptorSRV_.handleCPU
		);
	}

	// RTV - RenderTargetViewの生成
	{
		descriptorRTV_ = dxCommon_->GetDescriptorsObj()->GetCurrentDescriptor(DescriptorType::RTV);

		device->CreateRenderTargetView(
			resource_.Get(),
			nullptr,
			descriptorRTV_.handleCPU
		);
	}

}

void RenderTexture::Unload() {
	dxCommon_->GetDescriptorsObj()->Erase(descriptorRTV_);
}

////////////////////////////////////////////////////////////////////////////////////////////
// TextureManager class methods
////////////////////////////////////////////////////////////////////////////////////////////

void TextureManager::Init(DirectXCommon* dxCommon) {

	// dxCommonの保存
	dxCommon_ = dxCommon;

	LoadTexture("resources/uvChecker.png");
	LoadTexture("resources/tile_black.png");
	LoadTexture("resources/monsterBall.png");

}

void TextureManager::Term() {
	textures_.clear();
	dxCommon_ = nullptr;
}

void TextureManager::EnableTexture() {
	for (auto& textureData : textures_) {
		if (textureData.second.isTextureEnabled) {
			continue;
		}

		textureData.second.isTextureEnabled = true;
		Console::GetInstance()->SetLog(
			"[Send Texture] filePath: " + textureData.first
		);
	}
}

void TextureManager::LoadTexture(const std::string& filePath) {
	auto it = textures_.find(filePath);
	if (it != textures_.end()) { //!< 同一keyが見つかった場合
		// 参照数にインクリメント
		textures_[filePath].referenceCount++;
		return;
	}

	// textureの登録
	textures_[filePath].texture          = std::make_unique<Texture>();
	textures_[filePath].texture->Load(filePath, dxCommon_);
	textures_[filePath].referenceCount   = 1;
	textures_[filePath].isTextureEnabled = false; // commandListに積んだだけなのでまだ使えない

	Console::GetInstance()->SetLog(
		"[Load Texture] filePath: " + filePath
	);
}

void TextureManager::CreateRenderTexture(int32_t width, int32_t height, const std::string& key) {
	if (FindKey(key)) {
		Log("TextureManager::CreateRenderTexture \n if (it != textures_.end()) { \n return; \n} \n");
		Console::GetInstance()->SetLog("warning: texture already made. [key]: " + key, Console::warningColor);
		return;
	}

	auto renderTexture = std::make_unique<RenderTexture>();
	renderTexture->Create(width, height, dxCommon_);

	textures_[key].texture = std::move(renderTexture);
	textures_[key].referenceCount = 1;
	textures_[key].isTextureEnabled = true;

	Console::GetInstance()->SetLog(
		"[Create RenderTexture] key: " + key
	);
}

void TextureManager::UnloadTexture(const std::string& key) {
	assert(FindKey(key)); //!< 同一keyが見つからなかった
	//assert(textures_[key].isTextureEnabled); //!< SRVが作られてないのに削除使用としてる

	// 参照先が消える
	textures_[key].referenceCount--;

	if (textures_[key].referenceCount == 0) { //!< 参照先がない場合
		textures_[key].texture.reset();
		textures_.erase(key);
	}
}

void TextureManager::DeleteTexture(const std::string& key) {
	assert(FindKey(key)); //!< 同一keyが見つからなかった
	assert(textures_[key].isTextureEnabled); //!< SRVが作られてないのに削除使用として

	textures_[key].texture.reset();
	textures_.erase(key);

	Console::GetInstance()->SetLog(
		"[Delete Texture] key: " + key
	);
}

TextureManager* TextureManager::GetInstance() {
	static TextureManager instance;
	return &instance;
}

//=========================================================================================
// private methods
//=========================================================================================

std::unordered_map<std::string, TextureManager::TextureData>::const_iterator TextureManager::GetKey(const std::string& key) const {
	auto it = textures_.find(key);
	if (it == textures_.end()) {
		assert(false);
	}

	return it;
}

bool TextureManager::FindKey(const std::string& key) const {
	auto it = textures_.find(key);
	if (it == textures_.end()) {
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
// TextureMethod namespace methods
////////////////////////////////////////////////////////////////////////////////////////////

DirectX::ScratchImage TextureMethod::LoadTexture(const std::string& filePath) {
	DirectX::ScratchImage image = {};
	std::wstring filePathW = ToWstring(filePath); //!< wstringに変換

	auto hr = DirectX::LoadFromWICFile(
		filePathW.c_str(),
		DirectX::WIC_FLAGS_FORCE_SRGB,
		nullptr,
		image
	);

	if (FAILED(hr)) {
		std::string errorLog;
		errorLog
			= "[Texture Not Found] \n filePath: " + filePath;

		ExternalLogger::Write(errorLog);
		Assert(false, errorLog, "Error: LoadTexture");
	}

	// MipMapsの生成
	DirectX::ScratchImage mipImage = {};

	hr = DirectX::GenerateMipMaps(
		image.GetImages(),
		image.GetImageCount(),
		image.GetMetadata(),
		DirectX::TEX_FILTER_SRGB,
		0,
		mipImage
	);

	assert(SUCCEEDED(hr));

	return mipImage;
}

ComPtr<ID3D12Resource> TextureMethod::CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata) {
	// デスクの設定
	D3D12_RESOURCE_DESC desc = {};
	desc.Width            = UINT(metadata.width);
	desc.Height           = UINT(metadata.height);
	desc.MipLevels        = UINT16(metadata.mipLevels);
	desc.DepthOrArraySize = UINT16(metadata.arraySize);
	desc.Format           = metadata.format;
	desc.SampleDesc.Count = 1;
	desc.Dimension        = D3D12_RESOURCE_DIMENSION(metadata.dimension);

	// ヒーププロパティの設定
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_DEFAULT;

	// Resourceの生成
	ComPtr<ID3D12Resource> result;

	auto hr = device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&result)
	);

	assert(SUCCEEDED(hr));

	return result;
}

[[nodiscard]]
ComPtr<ID3D12Resource> TextureMethod::UploadTextureData(
	ID3D12Resource* texture, const DirectX::ScratchImage& mipImages,
	ID3D12Device* device, ID3D12GraphicsCommandList* commandList) {

	std::vector<D3D12_SUBRESOURCE_DATA> subresource;
	DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresource);

	uint64_t intermediateSize = GetRequiredIntermediateSize(texture, 0, UINT(subresource.size()));
	ComPtr<ID3D12Resource> intermediateResource = DxObjectMethod::CreateBufferResource(device, intermediateSize);
	UpdateSubresources(commandList, texture, intermediateResource.Get(), 0, 0, UINT(subresource.size()), subresource.data());

	// 転送後は利用できるように D3D12_RESOUCE_STATE_COPY_DESC -> D3D12_RESOUCE_STATE_GENETIC_READ へ変更
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource   = texture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_GENERIC_READ;

	commandList->ResourceBarrier(1, &barrier);

	return intermediateResource;
}

