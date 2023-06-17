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

		void BeginRenderPass(const Renderpass* renderPass, const RenderTarget* renderTarget, float clearR = 0, float clearG = 0, float clearB = 0, float clearDepth = 1.0f) override;
		void EndRenderPass() override;

		void SetViewport(const Viewport& viewport) override;
		void SetScissor(const Scissor& viewport) override;

		void BindPipeline(const Pipeline* pipeline) override;
		void BindVertexBuffer(const Buffer* buffer) override;
		void BindIndexBuffer(const Buffer* buffer) override;
		void BindDescriptorSet(const PipelineLayout* pipelineLayout, const DescriptorSet* descriptorSet, int set = 0) override;
		
		void PushConstants(const PipelineLayout* pipelineLayout, uint32_t rangeIndex, uint32_t offset, uint32_t size, void* data) override;

		void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;

		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) override;
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
