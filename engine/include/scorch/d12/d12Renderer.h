#pragma once

#ifdef RENDERER_D12

#include "render/renderer.h"
#include "core/utils.h"

#include <dxgi1_4.h>
#include "directx/d3d12.h"
#include "directx/d3dx12.h"
#include "dxguids/dxguids.h"
#include "directx/d3dx12_barriers.h"

using Microsoft::WRL::ComPtr;

namespace SC
{
	class D12Renderer : public Renderer
	{
	public:
		D12Renderer();
		~D12Renderer();

		void Init() override;
		void Cleanup() override;
		void CreateSwapchain() override;

		void BeginFrame() override;
		void EndFrame() override;


		void SubmitCommandBuffer(const CommandBuffer& commandBuffer) override;
		CommandBuffer& GetFrameCommandBuffer() const override;
		Renderpass* DefaultRenderPass() const override;
		RenderTarget* DefaultRenderTarget() const override;

	private:
		void CreateFactory();
		void CreateAdapter();
		void CreateDevice();
	private:
		ComPtr<IDXGIFactory4> m_factory{ nullptr };
		ComPtr<ID3D12Debug1> m_debugController{ nullptr };

		ComPtr<IDXGIAdapter1> m_adapter{ nullptr };
		ComPtr<ID3D12Device> m_device{ nullptr };
		ComPtr<ID3D12DebugDevice> m_debugDevice{ nullptr };
	};
}

#endif // RENDERER_D12
