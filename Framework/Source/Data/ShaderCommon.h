/***************************************************************************
# Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/

/*******************************************************************
                    Common OGL bindings
*******************************************************************/

#ifndef _FALCOR_SHADER_COMMON_H_
#define _FALCOR_SHADER_COMMON_H_

#include "HostDeviceData.h"

#ifndef MAX_LIGHT_SOURCES
#define MAX_LIGHT_SOURCES 16
#endif

cbuffer InternalPerFrameCB : register(b10)
{
    CameraData gCam;
    uint32_t gLightsCount;
    vec3 pad;
    LightData gLights[MAX_LIGHT_SOURCES];
};

cbuffer InternalPerMeshCB : register(b11)
{
    mat4 gWorldMat[64]; // If the mesh has bones, these are the bones matrices
    mat3 gWorldInvTransposeMat[64]; // Per-instance matrices for transforming normals
    uint32_t gDrawId[64]; // Zero-based order/ID of Mesh Instances drawn per SceneRenderer::renderScene call.
    uint32_t gMeshId;
};

#ifdef _VERTEX_BLENDING
mat4 getBlendedWorldMat(vec4 weights, uint4 ids)
{
    mat4 worldMat = gWorldMat[ids.x] * weights.x;
    worldMat += gWorldMat[ids.y] * weights.y;
    worldMat += gWorldMat[ids.z] * weights.z;
    worldMat += gWorldMat[ids.w] * weights.w;

    return worldMat;
}

mat3 getBlendedInvTransposeWorldMat(vec4 weights, uint4 ids)
{
    mat3 mat = gWorldInvTransposeMat[ids.x] * weights.x;
    mat += gWorldInvTransposeMat[ids.y] * weights.y;
    mat += gWorldInvTransposeMat[ids.z] * weights.z;
    mat += gWorldInvTransposeMat[ids.w] * weights.w;

    return mat;
}

#endif

cbuffer InternalPerMaterialCB : register(b12)
{
    MaterialData gMaterial;
    MaterialData gTemporalMaterial;
    float gTemporalLODThreshold;
    bool gEnableTemporalNormalMaps;
    bool gDebugTemporalMaterial;
};

float2 calcMotionVector(float2 pixelCrd, float4 prevPosH, float2 renderTargetDim)
{
    float2 prevCrd = prevPosH.xy / prevPosH.w;
    prevCrd *= float2(0.5, -0.5);
    prevCrd += 0.5f;
    float2 normalizedCrd = pixelCrd / renderTargetDim;
    return prevCrd - normalizedCrd;
}

/*******************************************************************
                    GLSL Evaluation routines
*******************************************************************/
#if defined(FALCOR_GL) || defined(FALCOR_GLSL)
bool isSamplerBound(in sampler2D sampler)
{
    return any(uvec2(sampler) != 0);
}

vec4 fetchTextureIfFound(in sampler2D sampler, in vec2 uv, in vec2 duvdx, in vec2 duvdy)
{
    vec4 ret = vec4(1.0f);
    if(isSamplerBound(sampler)) 
    {
        ret = textureGrad(sampler, uv, duvdx, duvdy);
    }
    return ret;
}
#endif // defined(FALCOR_GL) || defined(FALCOR_GLSL)
#endif  // _FALCOR_SHADER_COMMON_H_