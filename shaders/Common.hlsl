/* Copyright (c) 2018-2019, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// ---[ Structures ]---

struct HitInfo
{
	float4 HitNormal;
	float4 HitPos;
	float4 HitColor;
};

cbuffer ViewCB : register(b0)
{
	matrix view;
	matrix projMatrix;
	matrix projMatrixInv;
	matrix viewMatrix;	
	matrix viewMatrixInv;
	float4 viewOriginAndTanHalfFovY;
	float4 light;
	float2 resolution;
	uint debug;
	uint frame;
};

RWTexture2D<float4> RTOutput				: register(u0);

RaytracingAccelerationStructure SceneBVH	: register(t0, space0);

ByteAddressBuffer indices					: register(t1, space0);
ByteAddressBuffer vertices					: register(t2, space0);
Texture2D<float4> albedoTex					: register(t3, space0);
Texture2D<float4> normalTex					: register(t4, space0);
Texture2D<float2> velocityTex				: register(t5, space0);
Texture2D<float4> lastFrameTex				: register(t6, space0);
Texture2D<float> depth						: register(t7, space0);
Texture2D<float> lastDepth					: register(t8, space0);
Texture2DArray<float4> NoiseTex				: register(t9);

struct VertexAttributes
{
	float3 position;
	float3 normal;
	float2 uv;
};

uint3 GetIndices(uint triangleIndex, uint indexOffSet)
{
	uint baseIndex = (triangleIndex * 3) + indexOffSet;
	uint address = (baseIndex * 4);
	return indices.Load3(address);
}

#define NUM_ELEMENTS_PER_VERTEX 8 //see dxr_acceleration_model::Vertex
#define SIZE_OF_VERTEX_ELEMENT 4 //4 bytes

VertexAttributes GetVertexAttributes(uint triangleIndex, float3 barycentrics, uint indexOffSet, uint vertexOffset)
{
	uint3 indices = GetIndices(triangleIndex, indexOffSet);
	VertexAttributes v;
	v.position = float3(0, 0, 0);
	v.normal = float3(0, 0, 0);
	v.uv = float2(0, 0);

	for (uint i = 0; i < 3; i++)
	{
		uint address = ((indices[i]+ vertexOffset) * NUM_ELEMENTS_PER_VERTEX) * SIZE_OF_VERTEX_ELEMENT;
		v.position += asfloat(vertices.Load3(address)) * barycentrics[i];
		address += (3 * SIZE_OF_VERTEX_ELEMENT);
		v.normal += asfloat(vertices.Load3(address)) * barycentrics[i];
		address += (3 * SIZE_OF_VERTEX_ELEMENT);
		v.uv += asfloat(vertices.Load3(address)) * barycentrics[i];
	}

	return v;
}