#include "engine.hpp"
#include "particle.hpp"
#include <algorithm>
#include <d3d11.h>
#include <dxgiformat.h>
#include <minwinbase.h>
#include <synchapi.h>
#include <winerror.h>

Engine::Engine(const QWindow& window, std::vector<Particle>& initParticles, float G, float theta):
ctData{G, theta} {
    HWND windowId = reinterpret_cast<HWND>(window.winId());

    width = window.width();
    height = window.height();

    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferUsage = DXGI_USAGE_UNORDERED_ACCESS;
    swapChainDesc.OutputWindow = windowId;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;

    auto result = D3D11CreateDeviceAndSwapChain(nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &swapChain,
        &device,
        nullptr,
        &deviceContext);

    if (FAILED(result)) {
        throw(std::runtime_error("ERROR ON D3D11CreateDeviceAndSwapChain"));
    }

    particles = std::move(initParticles);

    // Initial octTree conditions
    octTree.resize(2000000);
    octTree[0].leftUpCoords[0] = 0;
    octTree[0].leftUpCoords[1] = 0;
    octTree[0].leftUpCoords[2] = 0;
    octTree[0].rightDownCoords[0] = width;
    octTree[0].rightDownCoords[1] = height;
    octTree[0].rightDownCoords[2] = width;

    Tile::updateOctTree(particles, octTree);

    createBufferTexture2D();
    createStructuredBuffer(particles, &particlesBuffer, &particlesBufferView);
    createStructuredBuffer(octTree, &octTreeBuffer, nullptr, true, &octTreeBufferView);
    createStagingBuffer(particles, &particlesStagingBuffer);
    createCTBuffer(16, &ctData);
    
    createCSShader(L"..//..//..//src//renderShader.hlsl", &CSRenderShader);
    createCSShader(L"..//..//..//src//logicShader.hlsl", &CSLogicShader);
    createCSShader(L"..//..//..//src//resetTexture.hlsl", &CSResetTextureShader);
    
    deviceContext->CSSetUnorderedAccessViews(0, 1, &backBufferView, nullptr);
    deviceContext->CSSetUnorderedAccessViews(1, 1, &particlesBufferView, nullptr);
    deviceContext->CSSetShaderResources(0, 1, &octTreeBufferView);
    deviceContext->CSSetConstantBuffers(0, 1, &ctBuffer);
}

void Engine::createBufferTexture2D() {
    auto result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
    if (FAILED(result)) {
        throw(std::runtime_error("ERROR ON GetBuffer"));
    }

    D3D11_UNORDERED_ACCESS_VIEW_DESC viewDesc;
    viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    viewDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    viewDesc.Texture2D.MipSlice = 0;

    result = device->CreateUnorderedAccessView(backBuffer, &viewDesc, &backBufferView);
    if (FAILED(result)) {
        throw(std::runtime_error("ERROR ON CreateUnorderedAccessView"));
    }
}

void Engine::createCTBuffer(uint size, void *pData) {
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = size;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = pData;

    auto result = device->CreateBuffer(&desc, &data, &ctBuffer);;

    if (FAILED(result)) {
        throw(std::runtime_error("ERROR ON createCTBuffer->CreateBuffer"));
    }
}

void Engine::createCSShader(LPCWSTR fileName, ID3D11ComputeShader **shader) {
    ID3DBlob *code, *error;

    D3DCompileFromFile(fileName,
        nullptr,
        nullptr,
        "CSMain",
        "cs_5_0",
        D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG,
        0,
        &code,
        &error);

    if (error) {
        throw(std::runtime_error((char *)(error->GetBufferPointer())));
        return;
    }

    auto result = device->CreateComputeShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, shader);
    if (FAILED(result)) {
        throw(std::runtime_error("ERROR ON CreateComputeShader"));
    }
}

void Engine::setupViewport() {
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(viewport));

    viewport.TopLeftX = viewport.TopLeftY = 0;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;

    deviceContext->RSSetViewports(1, &viewport);
}

void Engine::display() {
    // Swap front buffer with back buffer
    auto result = swapChain->Present(0, 0);
    if (FAILED(result)) {
        throw(std::runtime_error("ERROR ON Present"));
    }
}

void Engine::iterate() {
    static const auto MAX_RESIZE = (2 * particles.size() > 10000) ? 2 * particles.size() : 10000;
    deviceContext->CopyResource(particlesStagingBuffer, particlesBuffer);

    // Update CPU vector with GPU data
    D3D11_MAPPED_SUBRESOURCE particlesSubresource;
    ZeroMemory(&particlesSubresource, sizeof(particlesSubresource));
    auto result = deviceContext->Map(particlesStagingBuffer, 0, D3D11_MAP_READ, 0, &particlesSubresource);
    if (FAILED(result)) {
        throw(std::runtime_error("ERROR ON Map"));
    }
    std::copy((Particle *)particlesSubresource.pData, (Particle *)particlesSubresource.pData + particles.size(), particles.data());
    deviceContext->Unmap(particlesStagingBuffer, 0);

    octTree.clear();
    // octTree.push_back(Tile());
    octTree.resize(MAX_RESIZE);
    octTree[0].leftUpCoords[0] = 0;
    octTree[0].leftUpCoords[1] = 0;
    octTree[0].leftUpCoords[2] = 0;
    octTree[0].rightDownCoords[0] = width;
    octTree[0].rightDownCoords[1] = height;
    octTree[0].rightDownCoords[2] = width;

    Tile::updateOctTree(particles, octTree);

    // Update GPU data with CPU vector
    D3D11_MAPPED_SUBRESOURCE octTreeSubrecourse;
    ZeroMemory(&octTreeSubrecourse, sizeof(octTreeSubrecourse));
    result = deviceContext->Map(octTreeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &octTreeSubrecourse);
    if (FAILED(result)) {
        throw(std::runtime_error("ERROR ON Map"));
    }
    std::copy(octTree.data(), octTree.data() + octTree.size(), (Tile *)(octTreeSubrecourse.pData));
    deviceContext->Unmap(octTreeBuffer, 0);

    deviceContext->CSSetShader(CSResetTextureShader, nullptr, 0);
    deviceContext->Dispatch(width / 16, height / 16, 1);
    deviceContext->CSSetShader(CSLogicShader, nullptr, 0);
    deviceContext->Dispatch(65535 , 1, 1);
    deviceContext->CSSetShader(CSRenderShader, nullptr, 0);
    deviceContext->Dispatch(65535, 1, 1);

    setupViewport();
    display();
}