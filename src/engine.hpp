#pragma once
#include "octTree.hpp"
#include "particle.hpp"
#include <iostream>
#include <QWindow>
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3d11sdklayers.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <dxgiformat.h>
#include <dxgi1_3.h>
#include <minwinbase.h>
#include <stdexcept>
#include <vector>
#include <winerror.h>
#include <combaseapi.h>
#include <cstddef>
#include <dxgidebug.h>

struct cbuffer {
    float G;
    float theta;
    float padding[2];
};

class Engine {
    public:
        Engine(const QWindow&, std::vector<Particle>&, float G, float theta);
        Engine(const Engine&) = delete;
        ~Engine() {
            device->Release();
            deviceContext->Release();
            swapChain->Release();
            backBuffer->Release();
            backBufferView->Release();

            particlesBuffer->Release();
            particlesBufferView->Release();
            particlesStagingBuffer->Release();

            octTreeBuffer->Release();
            octTreeBufferView->Release();

            CSRenderShader->Release();
            CSLogicShader->Release();
            CSResetTextureShader->Release();
        }

        void iterate();
        void setupViewport();
        void display();
    
    private:
        void createBufferTexture2D();
        void createCTBuffer(uint size, void *pData);

        template<class T>
        void createStructuredBuffer(const std::vector<T>& data,
                                    ID3D11Buffer **ppBuffer,
                                    ID3D11UnorderedAccessView **ppUAView,
                                    bool dynamic = false,
                                    ID3D11ShaderResourceView **ppSRView = nullptr) {
            D3D11_BUFFER_DESC desc;
            desc.ByteWidth = data.size() * sizeof(data[0]);
            desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
            desc.StructureByteStride = sizeof(data[0]);
            if (!dynamic) {
                desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
                desc.CPUAccessFlags = 0;
                desc.Usage = D3D11_USAGE_DEFAULT;
            } else {
                desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                desc.Usage = D3D11_USAGE_DYNAMIC;
            }

            D3D11_SUBRESOURCE_DATA initData;
            initData.pSysMem = data.data();

            auto result = device->CreateBuffer(&desc, &initData, ppBuffer);
            if (FAILED(result)) {
                throw(std::runtime_error("ERROR ON CreateBuffer"));
            }

            if (!dynamic) {
                D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
                uavDesc.Format = DXGI_FORMAT_UNKNOWN;
                uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
                uavDesc.Buffer.FirstElement = 0;
                uavDesc.Buffer.NumElements = data.size();
                uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
                
                result = device->CreateUnorderedAccessView(*ppBuffer, &uavDesc, ppUAView);
                if (FAILED(result)) {
                    throw(std::runtime_error("ERROR ON CreateUnorderedAccessView"));
                }
            } else {
                D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
                srDesc.Format = DXGI_FORMAT_UNKNOWN;
                srDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
                srDesc.Buffer.FirstElement = 0;
                srDesc.Buffer.NumElements = data.size();
                
                result = device->CreateShaderResourceView(*ppBuffer, &srDesc, ppSRView);
                if (FAILED(result)) {
                    throw(std::runtime_error("ERROR ON CreateShaderResourceView"));
                }
            }
        }

        template<class T>
        void createStagingBuffer(const std::vector<T>& data,
                                 ID3D11Buffer **ppBuffer) {
            D3D11_BUFFER_DESC desc;
            desc.ByteWidth = data.size() * sizeof(data[0]);
            desc.MiscFlags = 0;
            desc.StructureByteStride = sizeof(data[0]);
            desc.BindFlags = 0;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            desc.Usage = D3D11_USAGE_STAGING;

            auto result = device->CreateBuffer(&desc, nullptr, ppBuffer);
            if (FAILED(result)) {
                throw(std::runtime_error("ERROR ON CreateBuffer"));
            }
        }

        void createCSShader(LPCWSTR fileName, ID3D11ComputeShader **shader);

        int width, height;

        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        IDXGISwapChain *swapChain = nullptr;
        ID3D11Device *device = nullptr;
        ID3D11DeviceContext *deviceContext = nullptr;

        ID3D11Texture2D *backBuffer = nullptr;
        ID3D11Buffer *particlesBuffer = nullptr;
        ID3D11Buffer *octTreeBuffer = nullptr;
        ID3D11Buffer *particlesStagingBuffer = nullptr;
        ID3D11Buffer *ctBuffer = nullptr;

        ID3D11UnorderedAccessView *backBufferView = nullptr;
        ID3D11UnorderedAccessView *particlesBufferView = nullptr;
        ID3D11ShaderResourceView *octTreeBufferView = nullptr;

        ID3D11ComputeShader *CSRenderShader = nullptr;
        ID3D11ComputeShader *CSLogicShader = nullptr;
        ID3D11ComputeShader *CSResetTextureShader = nullptr;

        IDXGIDebug *debugDev = nullptr;

        std::vector<Particle> particles;
        std::vector<float> floats;
        std::vector<Tile> octTree;

        cbuffer ctData {};
};