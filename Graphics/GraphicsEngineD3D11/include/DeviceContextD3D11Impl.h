/*     Copyright 2015-2018 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
 *
 *  In no event and under no legal theory, whether in tort (including negligence), 
 *  contract, or otherwise, unless required by applicable law (such as deliberate 
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental, 
 *  or consequential damages of any character arising as a result of this License or 
 *  out of the use or inability to use the software (including but not limited to damages 
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and 
 *  all other commercial damages or losses), even if such Contributor has been advised 
 *  of the possibility of such damages.
 */

#pragma once

/// \file
/// Declaration of Diligent::DeviceContextD3D11Impl class

#include "DeviceContextD3D11.h"
#include "DeviceContextBase.h"
#include "ShaderD3D11Impl.h"
#include "BufferD3D11Impl.h"
#include "TextureViewD3D11Impl.h"
#include "PipelineStateD3D11Impl.h"

#ifdef _DEBUG
#   define VERIFY_CONTEXT_BINDINGS
#endif

namespace Diligent
{

/// Implementation of the Diligent::IDeviceContextD3D11 interface
class DeviceContextD3D11Impl : public DeviceContextBase<IDeviceContextD3D11, BufferD3D11Impl, TextureViewD3D11Impl, PipelineStateD3D11Impl>
{
public:
    using TDeviceContextBase = DeviceContextBase<IDeviceContextD3D11, BufferD3D11Impl, TextureViewD3D11Impl, PipelineStateD3D11Impl>;

    DeviceContextD3D11Impl(IReferenceCounters*              pRefCounters,
                           IMemoryAllocator&                Allocator,
                           IRenderDevice*                   pDevice,
                           ID3D11DeviceContext*             pd3d11DeviceContext,
                           const struct EngineD3D11Attribs& EngineAttribs,
                           bool                             bIsDeferred);
    virtual void QueryInterface( const Diligent::INTERFACE_ID &IID, IObject **ppInterface )override final;

    virtual void SetPipelineState(IPipelineState* pPipelineState)override final;

    virtual void TransitionShaderResources(IPipelineState* pPipelineState, IShaderResourceBinding* pShaderResourceBinding)override final;

    virtual void CommitShaderResources(IShaderResourceBinding* pShaderResourceBinding, Uint32 Flags)override final;

    virtual void SetStencilRef(Uint32 StencilRef)override final;

    virtual void SetBlendFactors(const float* pBlendFactors = nullptr)override final;

    virtual void SetVertexBuffers( Uint32 StartSlot, Uint32 NumBuffersSet, IBuffer **ppBuffers, Uint32* pOffsets, Uint32 Flags )override final;
    
    virtual void InvalidateState()override final;

    virtual void SetIndexBuffer( IBuffer* pIndexBuffer, Uint32 ByteOffset )override final;

    virtual void SetViewports( Uint32 NumViewports, const Viewport* pViewports, Uint32 RTWidth, Uint32 RTHeight )override final;

    virtual void SetScissorRects( Uint32 NumRects, const Rect* pRects, Uint32 RTWidth, Uint32 RTHeight )override final;

    virtual void SetRenderTargets( Uint32 NumRenderTargets, ITextureView* ppRenderTargets[], ITextureView* pDepthStencil )override final;

    virtual void Draw( DrawAttribs &DrawAttribs )override final;

    virtual void DispatchCompute( const DispatchComputeAttribs &DispatchAttrs )override final;

    virtual void ClearDepthStencil( ITextureView* pView, Uint32 ClearFlags, float fDepth, Uint8 Stencil)override final;

    virtual void ClearRenderTarget( ITextureView* pView, const float *RGBA )override final;

    virtual void Flush()override final;

    virtual void FinishFrame(bool ForceRelease)override final;
    
    void FinishCommandList(class ICommandList **ppCommandList)override final;

    virtual void ExecuteCommandList(class ICommandList* pCommandList)override final;

    virtual void SignalFence(IFence* pFence, Uint64 Value)override final;

    ID3D11DeviceContext* GetD3D11DeviceContext(){ return m_pd3d11DeviceContext; }
    
    void CommitRenderTargets();

    /// Clears the committed shader resource cache. This function 
    /// is called once per frame (before present) to release all 
    /// outstanding objects that are only kept alive by references 
    /// in the cache. The function does not release cached vertex and
    /// index buffers, input layout, depth-stencil, rasterizer, and blend
    /// states.
    void ReleaseCommittedShaderResources();

    /// Number of different shader types (Vertex, Pixel, Geometry, Domain, Hull, Compute)
    static constexpr int NumShaderTypes = 6;

private:
    
    /// Commits d3d11 index buffer to the d3d11 device context.
    void CommitD3D11IndexBuffer(VALUE_TYPE IndexType);

    /// Commits d3d11 vertex buffers to the d3d11 device context.
    void CommitD3D11VertexBuffers(class PipelineStateD3D11Impl* pPipelineStateD3D11);

    /// Helper template function used to facilitate resource unbinding
    template<typename TD3D11ResourceViewType,
             typename TSetD3D11View,
             size_t NumSlots>
    void UnbindResourceView(TD3D11ResourceViewType CommittedD3D11ViewsArr[][NumSlots], 
                            ID3D11Resource*        CommittedD3D11ResourcesArr[][NumSlots], 
                            Uint8                  NumCommittedResourcesArr[],
                            ID3D11Resource*        pd3d11ResToUndind,
                            TSetD3D11View          SetD3D11ViewMethods[]);

    /// Unbinds a texture from the shader resource view slots.
    /// \note The function only unbinds the texture from d3d11 device
    ///       context. All shader bindings are retained.
    void UnbindTextureFromInput(TextureBaseD3D11* pTexture, ID3D11Resource* pd3d11Resource);

    /// Unbinds a buffer from the input (shader resource views slots, index 
    /// and vertex buffer slots).
    /// \note The function only unbinds the buffer from d3d11 device
    ///       context. All shader bindings are retained.
    void UnbindBufferFromInput(BufferD3D11Impl* pBuffer, ID3D11Resource* pd3d11Buffer);

    /// Unbinds a resource from the UAV slots.
    /// \note The function only unbinds the texture from the device
    ///       context. All shader bindings are retained.
    void UnbindResourceFromUAV(IDeviceObject* pResource, ID3D11Resource* pd3d11Resource);

    /// Unbinds a texture from render target slots.
    void UnbindTextureFromRenderTarget(TextureBaseD3D11* pResource);

    /// Unbinds a texture from depth-stencil.
    void UnbindTextureFromDepthStencil(TextureBaseD3D11* pTexD3D11);

    template<bool TransitionResources,
             bool CommitResources>
    void TransitionAndCommitShaderResources(IPipelineState* pPSO, IShaderResourceBinding* pShaderResourceBinding);

    void ClearStateCache();

    CComPtr<ID3D11DeviceContext> m_pd3d11DeviceContext; ///< D3D11 device context

    /// An array of D3D11 constant buffers committed to the D3D11 device context,
    /// for each shader type. Device context addref's all bound resources, so we do 
    /// not need to keep strong references
    ID3D11Buffer*              m_CommittedD3D11CBs     [NumShaderTypes][D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
    
    /// An array of D3D11 shader resource views committed to the D3D11 device context,
    /// for each shader type. Device context addref's all bound resources, so we do 
    /// not need to keep strong references
    ID3D11ShaderResourceView*  m_CommittedD3D11SRVs    [NumShaderTypes][D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
    
    /// An array of D3D11 samplers committed to the D3D11 device context,
    /// for each shader type. Device context addref's all bound resources, so we do 
    /// not need to keep strong references
    ID3D11SamplerState*        m_CommittedD3D11Samplers[NumShaderTypes][D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
    
    /// An array of D3D11 UAVs committed to the D3D11 device context,
    /// for each shader type. Device context addref's all bound resources, so we do 
    /// not need to keep strong references
    ID3D11UnorderedAccessView* m_CommittedD3D11UAVs    [NumShaderTypes][D3D11_PS_CS_UAV_REGISTER_COUNT];

    /// An array of D3D11 resources commited as SRV to the D3D11 device context,
    /// for each shader type. Device context addref's all bound resources, so we do 
    /// not need to keep strong references
    ID3D11Resource*  m_CommittedD3D11SRVResources    [NumShaderTypes][D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];

    /// An array of D3D11 resources commited as UAV to the D3D11 device context,
    /// for each shader type. Device context addref's all bound resources, so we do 
    /// not need to keep strong references
    ID3D11Resource*  m_CommittedD3D11UAVResources    [NumShaderTypes][D3D11_PS_CS_UAV_REGISTER_COUNT];

    Uint8 m_NumCommittedCBs[NumShaderTypes] = {};
    Uint8 m_NumCommittedSRVs[NumShaderTypes]= {};
    Uint8 m_NumCommittedSamplers[NumShaderTypes]= {};
    Uint8 m_NumCommittedUAVs[NumShaderTypes]= {};

    /// An array of D3D11 vertex buffers committed to the D3D device context
    /// There is no need to keep strong references because D3D11 device context 
    /// already does. Buffers cannot be destroyed while bound to the context.
    /// We only mirror all bindings.
    ID3D11Buffer*  m_CommittedD3D11VertexBuffers[MaxBufferSlots] = {};
    /// An array of strides of committed vertex buffers
    UINT m_CommittedD3D11VBStrides[MaxBufferSlots];
    /// An array of offsets of committed vertex buffers
    UINT m_CommittedD3D11VBOffsets[MaxBufferSlots];
    /// Number committed vertex buffers
    UINT m_NumCommittedD3D11VBs;
    /// Flag indicating if currently committed D3D11 vertex buffers are up to date
    bool m_bCommittedD3D11VBsUpToDate = false;

    /// D3D11 input layout committed to the device object.
    /// Device context keeps the layout alive, so there is no need
    /// to keep strong reference
    ID3D11InputLayout *m_CommittedD3D11InputLayout = nullptr;

    /// Strong reference to the D3D11 buffer committed as index buffer 
    /// to the D3D device context
    CComPtr<ID3D11Buffer>  m_CommittedD3D11IndexBuffer;
    /// Format of the committed D3D11 index buffer
    VALUE_TYPE m_CommittedIBFormat;
    /// Offset of the committed D3D11 index buffer
    Uint32 m_CommittedD3D11IndexDataStartOffset;
    /// Flag indicating if currently committed D3D11 index buffer is up to date
    bool m_bCommittedD3D11IBUpToDate = false;

    D3D11_PRIMITIVE_TOPOLOGY m_CommittedD3D11PrimTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    PRIMITIVE_TOPOLOGY m_CommittedPrimitiveTopology = PRIMITIVE_TOPOLOGY_UNDEFINED;

    /// Strong references to the committed D3D11 shaders
    CComPtr<ID3D11DeviceChild> m_CommittedD3DShaders[NumShaderTypes];

    Uint32 m_DebugFlags;

    FixedBlockMemoryAllocator m_CmdListAllocator;

#ifdef VERIFY_CONTEXT_BINDINGS
    /// Verifies that bound render target formats are consistent with the PSO
    void dbgVerifyRenderTargetFormats();

    /// Helper template function used to facilitate context verification
    template<UINT MaxResources, typename TD3D11ResourceType, typename TGetD3D11ResourcesType>
    void dbgVerifyCommittedResources(TD3D11ResourceType     CommittedD3D11ResourcesArr[][MaxResources],
                                     Uint8                  NumCommittedResourcesArr[],
                                     TGetD3D11ResourcesType GetD3D11ResMethods[],
                                     const Char*            ResourceName,
                                     SHADER_TYPE            ShaderType);

    /// Helper template function used to facilitate validation of SRV and UAV consistency with D3D11 resources
    template<UINT MaxResources, typename TD3D11ViewType>
    void dbgVerifyViewConsistency(TD3D11ViewType  CommittedD3D11ViewArr[][MaxResources],
                                  ID3D11Resource* CommittedD3D11ResourcesArr[][MaxResources],
                                  Uint8           NumCommittedResourcesArr[],
                                  const Char*     ResourceName,
                                  SHADER_TYPE     ShaderType);

    /// Debug function that verifies that SRVs cached in m_CommittedD3D11SRVs 
    /// array comply with resources actually committed to the D3D11 device context
    void dbgVerifyCommittedSRVs(SHADER_TYPE ShaderType = SHADER_TYPE_UNKNOWN);

    /// Debug function that verifies that UAVs cached in m_CommittedD3D11UAVs 
    /// array comply with resources actually committed to the D3D11 device context
    void dbgVerifyCommittedUAVs(SHADER_TYPE ShaderType = SHADER_TYPE_UNKNOWN);

    /// Debug function that verifies that samplers cached in m_CommittedD3D11Samplers
    /// array comply with resources actually committed to the D3D11 device context
    void dbgVerifyCommittedSamplers(SHADER_TYPE ShaderType = SHADER_TYPE_UNKNOWN);

    /// Debug function that verifies that constant buffers cached in m_CommittedD3D11CBs 
    /// array comply with buffers actually committed to the D3D11 device context
    void dbgVerifyCommittedCBs(SHADER_TYPE ShaderType = SHADER_TYPE_UNKNOWN);

    /// Debug function that verifies that the index buffer cached in 
    /// m_CommittedD3D11IndexBuffer is the buffer actually committed to the D3D11 
    /// device context
    void dbgVerifyCommittedIndexBuffer();

    /// Debug function that verifies that vertex buffers cached in 
    /// m_CommittedD3D11VertexBuffers are the buffers actually committed to the D3D11 
    /// device context
    void dbgVerifyCommittedVertexBuffers();

    /// Debug function that verifies that shaders cached in 
    /// m_CommittedD3DShaders are the shaders actually committed to the D3D11 
    /// device context
    void dbgVerifyCommittedShaders();

#else
    #define dbgVerifyRenderTargetFormats(...)
    #define dbgVerifyCommittedSRVs(...)
    #define dbgVerifyCommittedUAVs(...)
    #define dbgVerifyCommittedSamplers(...)
    #define dbgVerifyCommittedCBs(...)
    #define dbgVerifyCommittedIndexBuffer(...)
    #define dbgVerifyCommittedVertexBuffers(...)
    #define dbgVerifyCommittedShaders(...)

#endif
};

}
