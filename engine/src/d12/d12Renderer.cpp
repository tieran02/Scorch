#ifdef RENDERER_D12

#include "pch.h"

#include "d12/d12Renderer.h"
#include "core/log.h"

namespace
{
	// Helper class for COM exceptions
	class com_exception : public std::exception
	{
	public:
		com_exception(HRESULT hr) : result(hr) {}

		const char* what() const noexcept override
		{
			static char s_str[64] = {};
			sprintf_s(s_str, "Failure with HRESULT of %08X",
				static_cast<unsigned int>(result));
			return s_str;
		}

	private:
		HRESULT result;
	};

	// Helper utility converts D3D API failures into exceptions.
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			auto excep = com_exception(hr);
			SC::Log::PrintCore(excep.what(), SC::LogSeverity::LogError);
			throw excep;
		}
	}
}

using namespace SC;

D12Renderer::D12Renderer() : Renderer(GraphicsAPI::D12)
{

}

D12Renderer::~D12Renderer()
{
	Cleanup();
}

void D12Renderer::Init()
{
	Log::PrintCore("Creating D3D12 Renderer");
	CreateFactory();
	CreateAdapter();
	CreateDevice();
}

void D12Renderer::Cleanup()
{
	Log::PrintCore("Cleaning up D3D12 Renderer");
}

void D12Renderer::CreateSwapchain()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void D12Renderer::BeginFrame()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void D12Renderer::EndFrame()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void D12Renderer::CreateFactory()
{
	// Create Factory
	UINT dxgiFactoryFlags = 0;

#if defined(SCORCH_DEBUG)
	// Create a Debug Controller to track errors
	ComPtr<ID3D12Debug> dc;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&dc)));
	ThrowIfFailed(dc->QueryInterface(IID_PPV_ARGS(&m_debugController)));
	m_debugController->EnableDebugLayer();
	m_debugController->SetEnableGPUBasedValidation(true);

	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	HRESULT result = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory));
	ThrowIfFailed(result);
}

void D12Renderer::CreateAdapter()
{
	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != m_factory->EnumAdapters1(adapterIndex, &m_adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		m_adapter->GetDesc1(&desc);


		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}


		if (SUCCEEDED(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0,
			_uuidof(ID3D12Device), nullptr)))
		{
			break;
		}

		m_adapter.Reset();
	}
}

void D12Renderer::CreateDevice()
{
	//Create the physical device interface for our selected GPU
	ThrowIfFailed(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)));

#if defined(SCORCH_DEBUG)
	ThrowIfFailed(m_device->QueryInterface(IID_PPV_ARGS(&m_debugDevice)));
#endif
}

void D12Renderer::SubmitCommandBuffer(const CommandBuffer& commandBuffer)
{
	throw std::logic_error("The method or operation is not implemented.");
}

SC::CommandBuffer& D12Renderer::GetFrameCommandBuffer() const
{
	throw std::logic_error("The method or operation is not implemented.");
}

SC::Renderpass* D12Renderer::DefaultRenderPass() const
{
	throw std::logic_error("The method or operation is not implemented.");
}

SC::RenderTarget* D12Renderer::DefaultRenderTarget() const
{
	throw std::logic_error("The method or operation is not implemented.");
}

#endif // RENDERER_D12
