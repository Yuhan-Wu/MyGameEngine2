// Includes
//=========

#include "../sContext.h"

#include "Includes.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Logging/Logging.h>
#include <Engine/ScopeGuard/cScopeGuard.h>

// Helper Declarations
//====================

namespace
{
	// In Direct3D "views" are objects that allow a texture to be used a particular way:
	// A render target view allows a texture to have color rendered to it
	ID3D11RenderTargetView* s_renderTargetView = nullptr;
	// A depth/stencil view allows a texture to have depth rendered to it
	ID3D11DepthStencilView* s_depthStencilView = nullptr;

	eae6320::cResult CreateDevice( const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight );
	eae6320::cResult InitializeViews(const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight);
}

// Interface
//==========

// Initialize / Clean Up
//----------------------

eae6320::cResult eae6320::Graphics::sContext::Initialize( const sInitializationParameters& i_initializationParameters )
{
	auto result = Results::Success;

	windowBeingRenderedTo = i_initializationParameters.mainWindow;

	// Create an interface to a Direct3D device
	if ( !( result = CreateDevice( i_initializationParameters.resolutionWidth, i_initializationParameters.resolutionHeight ) ) )
	{
		EAE6320_ASSERTF( false, "Can't initialize context without device" );
		return result;
	}

	if (!(result = InitializeViews(i_initializationParameters.resolutionWidth, i_initializationParameters.resolutionHeight)))
	{
		EAE6320_ASSERTF(false, "Can't initialize Graphics without the views");
		return result;
	}

	return result;
}

eae6320::cResult eae6320::Graphics::sContext::CleanUp()
{
	auto result = Results::Success;

	if ( direct3dImmediateContext )
	{
		direct3dImmediateContext->Release();
		direct3dImmediateContext = nullptr;
	}
	if ( direct3dDevice )
	{
		direct3dDevice->Release();
		direct3dDevice = nullptr;
	}
	if ( swapChain )
	{
		swapChain->Release();
		swapChain = nullptr;
	}
	if (s_renderTargetView)
	{
		s_renderTargetView->Release();
		s_renderTargetView = nullptr;
	}
	if (s_depthStencilView)
	{
		s_depthStencilView->Release();
		s_depthStencilView = nullptr;
	}

	windowBeingRenderedTo = NULL;

	return result;
}

void eae6320::Graphics::sContext::ClearScreen() {
	EAE6320_ASSERT(direct3dImmediateContext);

	// Every frame an entirely new image will be created.
	// Before drawing anything, then, the previous image will be erased
	// by "clearing" the image buffer (filling it with a solid color)
	{
		EAE6320_ASSERT(s_renderTargetView);

		// Black is usually used
		constexpr float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		direct3dImmediateContext->ClearRenderTargetView(s_renderTargetView, clearColor);
	}
	// In addition to the color buffer there is also a hidden image called the "depth buffer"
	// which is used to make it less important which order draw calls are made.
	// It must also be "cleared" every frame just like the visible color buffer.
	{
		EAE6320_ASSERT(s_depthStencilView);

		constexpr float clearToFarDepth = 1.0f;
		constexpr uint8_t stencilValue = 0;	// Arbitrary if stencil isn't used
		direct3dImmediateContext->ClearDepthStencilView(s_depthStencilView, D3D11_CLEAR_DEPTH, clearToFarDepth, stencilValue);
	}
}

void eae6320::Graphics::sContext::ClearScreen(float p_Background[3], float p_Alpha) {
	EAE6320_ASSERT(direct3dImmediateContext);

	// Every frame an entirely new image will be created.
	// Before drawing anything, then, the previous image will be erased
	// by "clearing" the image buffer (filling it with a solid color)
	{
		EAE6320_ASSERT(s_renderTargetView);

		// Black is usually used
		float clearColor[4] = { p_Background[0], p_Background[1], p_Background[2], p_Alpha };
		direct3dImmediateContext->ClearRenderTargetView(s_renderTargetView, clearColor);
	}
	// In addition to the color buffer there is also a hidden image called the "depth buffer"
	// which is used to make it less important which order draw calls are made.
	// It must also be "cleared" every frame just like the visible color buffer.
	{
		EAE6320_ASSERT(s_depthStencilView);

		constexpr float clearToFarDepth = 1.0f;
		constexpr uint8_t stencilValue = 0;	// Arbitrary if stencil isn't used
		direct3dImmediateContext->ClearDepthStencilView(s_depthStencilView, D3D11_CLEAR_DEPTH, clearToFarDepth, stencilValue);
	}
}

void eae6320::Graphics::sContext::Display() {
	EAE6320_ASSERT(swapChain);
	constexpr unsigned int swapImmediately = 0;
	constexpr unsigned int presentNextFrame = 0;
	const auto result = swapChain->Present(swapImmediately, presentNextFrame);
	EAE6320_ASSERT(SUCCEEDED(result));
}

// Helper Definitions
//===================

namespace
{
	eae6320::cResult CreateDevice( const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight )
	{
		auto& g_context = eae6320::Graphics::sContext::g_context;

		IDXGIAdapter* const useDefaultAdapter = nullptr;
		constexpr D3D_DRIVER_TYPE useHardwareRendering = D3D_DRIVER_TYPE_HARDWARE;
		constexpr HMODULE dontUseSoftwareRendering = NULL;
		constexpr unsigned int flags = D3D11_CREATE_DEVICE_SINGLETHREADED
#ifdef EAE6320_GRAPHICS_ISDEVICEDEBUGINFOENABLED
			| D3D11_CREATE_DEVICE_DEBUG
#endif
			;
		constexpr D3D_FEATURE_LEVEL* const useDefaultFeatureLevels = nullptr;
		constexpr unsigned int requestedFeatureLevelCount = 0;
		constexpr unsigned int sdkVersion = D3D11_SDK_VERSION;
		const auto swapChainDescription = [i_resolutionWidth, i_resolutionHeight, &g_context]
		{
			DXGI_SWAP_CHAIN_DESC swapChainDescription{};
			{
				DXGI_MODE_DESC& bufferDescription = swapChainDescription.BufferDesc;

				bufferDescription.Width = i_resolutionWidth;
				bufferDescription.Height = i_resolutionHeight;
				{
					DXGI_RATIONAL& refreshRate = bufferDescription.RefreshRate;

					refreshRate.Numerator = 0;	// Refresh as fast as possible
					refreshRate.Denominator = 1;
				}
				bufferDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				bufferDescription.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
				bufferDescription.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			}
			{
				DXGI_SAMPLE_DESC& multiSamplingDescription = swapChainDescription.SampleDesc;

				multiSamplingDescription.Count = 1;
				multiSamplingDescription.Quality = 0;	// Anti-aliasing is disabled
			}
			swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDescription.BufferCount = 1;
			swapChainDescription.OutputWindow = g_context.windowBeingRenderedTo;
			swapChainDescription.Windowed = TRUE;
			swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapChainDescription.Flags = 0;

			return swapChainDescription;
		}();
		D3D_FEATURE_LEVEL highestSupportedFeatureLevel;
		const auto result_create = D3D11CreateDeviceAndSwapChain( useDefaultAdapter, useHardwareRendering, dontUseSoftwareRendering,
			flags, useDefaultFeatureLevels, requestedFeatureLevelCount, sdkVersion, &swapChainDescription,
			&g_context.swapChain, &g_context.direct3dDevice, &highestSupportedFeatureLevel, &g_context.direct3dImmediateContext );
		if ( SUCCEEDED( result_create ) )
		{
			return eae6320::Results::Success;
		}
		else
		{
			EAE6320_ASSERT( false );
			eae6320::Logging::OutputError( "Direct3D failed to create a Direct3D11 device with HRESULT %#010x", result_create );
			return eae6320::Results::Failure;
		}
	}


	eae6320::cResult InitializeViews(const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight)
	{
		auto result = eae6320::Results::Success;

		ID3D11Texture2D* backBuffer = nullptr;
		ID3D11Texture2D* depthBuffer = nullptr;
		eae6320::cScopeGuard scopeGuard([&backBuffer, &depthBuffer]
			{
				// Regardless of success or failure the two texture resources should be released
				// (if the function is successful the views will hold internal references to the resources)
				if (backBuffer)
				{
					backBuffer->Release();
					backBuffer = nullptr;
				}
				if (depthBuffer)
				{
					depthBuffer->Release();
					depthBuffer = nullptr;
				}
			});

		auto& g_context = eae6320::Graphics::sContext::g_context;
		auto* const direct3dDevice = g_context.direct3dDevice;
		EAE6320_ASSERT(direct3dDevice);
		auto* const direct3dImmediateContext = g_context.direct3dImmediateContext;
		EAE6320_ASSERT(direct3dImmediateContext);

		// Create a "render target view" of the back buffer
		// (the back buffer was already created by the call to D3D11CreateDeviceAndSwapChain(),
		// but a "view" of it is required to use as a "render target",
		// meaning a texture that the GPU can render to)
		{
			// Get the back buffer from the swap chain
			{
				constexpr unsigned int bufferIndex = 0;	// This must be 0 since the swap chain is discarded
				const auto d3dResult = g_context.swapChain->GetBuffer(bufferIndex, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
				if (FAILED(d3dResult))
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, "Couldn't get the back buffer from the swap chain (HRESULT %#010x)", d3dResult);
					eae6320::Logging::OutputError("Direct3D failed to get the back buffer from the swap chain (HRESULT %#010x)", d3dResult);
					return result;
				}
			}
			// Create the view
			{
				constexpr D3D11_RENDER_TARGET_VIEW_DESC* const accessAllSubResources = nullptr;
				const auto d3dResult = direct3dDevice->CreateRenderTargetView(backBuffer, accessAllSubResources, &s_renderTargetView);
				if (FAILED(d3dResult))
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, "Couldn't create render target view (HRESULT %#010x)", d3dResult);
					eae6320::Logging::OutputError("Direct3D failed to create the render target view (HRESULT %#010x)", d3dResult);
					return result;
				}
			}
		}
		// Create a depth/stencil buffer and a view of it
		{
			// Unlike the back buffer no depth/stencil buffer exists until and unless it is explicitly created
			{
				const auto textureDescription = [i_resolutionWidth, i_resolutionHeight]
				{
					auto textureDescription = []() constexpr
					{
						D3D11_TEXTURE2D_DESC textureDescription{};

						textureDescription.MipLevels = 1;	// A depth buffer has no MIP maps
						textureDescription.ArraySize = 1;
						textureDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 24 bits for depth and 8 bits for stencil
						{
							DXGI_SAMPLE_DESC& sampleDescription = textureDescription.SampleDesc;

							sampleDescription.Count = 1;	// No multisampling
							sampleDescription.Quality = 0;	// Doesn't matter when Count is 1
						}
						textureDescription.Usage = D3D11_USAGE_DEFAULT;	// Allows the GPU to write to it
						textureDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
						textureDescription.CPUAccessFlags = 0;	// The CPU doesn't need access
						textureDescription.MiscFlags = 0;

						return textureDescription;
					}();

					textureDescription.Width = i_resolutionWidth;
					textureDescription.Height = i_resolutionHeight;

					return textureDescription;
				}();
				// The GPU renders to the depth/stencil buffer and so there is no initial data
				// (like there would be with a traditional texture loaded from disk)
				constexpr D3D11_SUBRESOURCE_DATA* const noInitialData = nullptr;
				const auto d3dResult = direct3dDevice->CreateTexture2D(&textureDescription, noInitialData, &depthBuffer);
				if (FAILED(d3dResult))
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, "Couldn't create depth buffer (HRESULT %#010x)", d3dResult);
					eae6320::Logging::OutputError("Direct3D failed to create the depth buffer resource (HRESULT %#010x)", d3dResult);
					return result;
				}
			}
			// Create the view
			{
				constexpr D3D11_DEPTH_STENCIL_VIEW_DESC* const noSubResources = nullptr;
				const auto d3dResult = direct3dDevice->CreateDepthStencilView(depthBuffer, noSubResources, &s_depthStencilView);
				if (FAILED(d3dResult))
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, "Couldn't create depth stencil view (HRESULT %#010x)", d3dResult);
					eae6320::Logging::OutputError("Direct3D failed to create the depth stencil view (HRESULT %#010x)", d3dResult);
					return result;
				}
			}
		}

		// Bind the views
		{
			constexpr unsigned int renderTargetCount = 1;
			direct3dImmediateContext->OMSetRenderTargets(renderTargetCount, &s_renderTargetView, s_depthStencilView);
		}
		// Specify that the entire render target should be visible
		{
			const auto viewPort = [i_resolutionWidth, i_resolutionHeight]
			{
				auto viewPort = []() constexpr
				{
					D3D11_VIEWPORT viewPort{};

					viewPort.TopLeftX = viewPort.TopLeftY = 0.0f;
					viewPort.MinDepth = 0.0f;
					viewPort.MaxDepth = 1.0f;

					return viewPort;
				}();
				viewPort.Width = static_cast<float>(i_resolutionWidth);
				viewPort.Height = static_cast<float>(i_resolutionHeight);

				return viewPort;
			}();
			constexpr unsigned int viewPortCount = 1;
			direct3dImmediateContext->RSSetViewports(viewPortCount, &viewPort);
		}

		return result;
	}
}
