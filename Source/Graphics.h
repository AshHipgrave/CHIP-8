#pragma once

#include "d3dUtils.h"
#include <DirectXColors.h>

using namespace std;

using namespace DirectX;
using namespace Microsoft::WRL;

class Graphics
{
public:
	Graphics();
	~Graphics();

	Graphics(const Graphics& rhs) = delete;
	Graphics& operator=(const Graphics& rhs) = delete;

public:
	bool Initialise(HWND windowHandle, int windowWidth, int windowHeight);

	void Update();
	void Draw();

	void OnResize(int windowWidth, int windowHeight);

private:
	bool InitDirect3D(HWND windowHandle, int windowWidth, int windowHeight);

	void CreateRtvAndDescriptorHeaps();

	void CreateCommandObjects();
	void CreateSwapChain(HWND windowHandle, int windowWidth, int windowHeight);
	void FlushCommandQueue();

private:
	ID3D12Resource* GetCurrentBackBuffer() const
	{
		return m_SwapChainBuffer[m_CurrentBackBufferIdx].Get();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView() const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			m_RtvHeap->GetCPUDescriptorHandleForHeapStart(),
			m_CurrentBackBufferIdx,
			m_RtvDescriptorSize);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const
	{
		return m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
	}

private:
	ComPtr<IDXGIFactory4> m_DxgiFactory;
	ComPtr<IDXGISwapChain> m_SwapChain;
	ComPtr<ID3D12Device> m_D3dDevice;

	ComPtr<ID3D12Fence> m_Fence;
	UINT64 m_CurrentFence = 0;

	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12CommandAllocator> m_CmdAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;

	static const int SwapChainBufferCount = 2;
	int m_CurrentBackBufferIdx = 0;

	ComPtr<ID3D12Resource> m_SwapChainBuffer[SwapChainBufferCount];
	ComPtr<ID3D12Resource> m_DepthStencilBuffer;

	ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

	D3D12_VIEWPORT m_ScreenViewport;
	D3D12_RECT m_ScissorRect;

	UINT m_RtvDescriptorSize = 0;
	UINT m_DsvDescriptorSize = 0;
	UINT m_CbvSrvDescriptorSize = 0;

	const D3D_DRIVER_TYPE m_DriverType = D3D_DRIVER_TYPE_HARDWARE;

	// These will likely need changing as they're probably not compatable with the CHIP-8 graphics. For now for just getting shit up and running it'll do.
	const DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	const DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
};

