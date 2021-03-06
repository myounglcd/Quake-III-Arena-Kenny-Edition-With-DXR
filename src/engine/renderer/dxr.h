#pragma once

#define ENABLE_DXR

#include "dx.h"
#include "dxc/dxcapi.h"
#include "dxc/dxcapi.use.h"
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <vector>
#include <algorithm>

#include "dxr_acceleration_structure_manager.h"
#include "dxr_heapManager.h"
#include "dx_shaders.h"

#define SAFE_RELEASE( x ) { if ( x ) { x->Release(); x = NULL; } }
#define SAFE_DELETE( x ) { if( x ) delete x; x = NULL; }
#define SAFE_DELETE_ARRAY( x ) { if( x ) delete[] x; x = NULL; }
#define ALIGN(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment)

static const D3D12_HEAP_PROPERTIES UploadHeapProperties =
{
	D3D12_HEAP_TYPE_UPLOAD,
	D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	D3D12_MEMORY_POOL_UNKNOWN,
	0, 0
};

static const D3D12_HEAP_PROPERTIES DefaultHeapProperties =
{
	D3D12_HEAP_TYPE_DEFAULT,
	D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	D3D12_MEMORY_POOL_UNKNOWN,
	0, 0
};

//--------------------------------------------------------------------------------------
// Standard D3D12
//--------------------------------------------------------------------------------------
/*
struct MaterialCB
{
	DirectX::XMFLOAT4 resolution;
};
*/
struct ViewCB
{
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projMatrix;
	DirectX::XMMATRIX projMatrixInv;
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX viewMatrixInv;
	DirectX::XMFLOAT4 viewOriginAndTanHalfFovY;
	DirectX::XMFLOAT4 light;
	DirectX::XMFLOAT2 resolution;
	UINT32 debug;
	UINT32 frame;

	ViewCB()
	{
		view = DirectX::XMMatrixIdentity();
		projMatrixInv = DirectX::XMMatrixIdentity();
		viewMatrixInv = DirectX::XMMatrixIdentity();
		viewOriginAndTanHalfFovY = DirectX::XMFLOAT4(0, 0.f, 0.f, 0.f);
		resolution = DirectX::XMFLOAT2(1280, 720);
		debug = 0;
		frame = 0;
	}
};

struct D3D12Resources
{	
	ID3D12Resource*								vertexBufferMega;
	ID3D12Resource*								indexBufferMega;

	UINT32	currentVertexBufferMegaCount;
	UINT32	currentIndexBufferMegaCount;
};

//--------------------------------------------------------------------------------------
//  DXR
//--------------------------------------------------------------------------------------

struct AccelerationStructureBuffer
{
	ID3D12Resource* pScratch;
	ID3D12Resource* pResult;
	ID3D12Resource* pInstanceDesc;			// only used in top-level AS

	AccelerationStructureBuffer()
	{
		pScratch = NULL;
		pResult = NULL;
		pInstanceDesc = NULL;
	}
};

struct HitProgram
{
	HitProgram(LPCWSTR name) : exportName(name)
	{
		desc = {};
		desc.HitGroupExport = exportName.c_str();
		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
		subobject.pDesc = &desc;
	}	

	HitProgram() {}
	D3DShaders::RtProgram chs; //ClosestHit

	std::wstring exportName;
	D3D12_HIT_GROUP_DESC desc = {};
	D3D12_STATE_SUBOBJECT subobject = {};
};

struct DXRGlobal
{
	AccelerationStructureBuffer						TLAS;//Top level Acceleration Structure
	std::vector<AccelerationStructureBuffer>					BLASs;//Bottom level Acceleration Structure

	ID3D12Resource*									sbt;//Shader table
	uint32_t										sbtEntrySize;

	D3DShaders::RtProgram										rgs;//RayGen Shader
	D3DShaders::RtProgram										miss;
	HitProgram										hit;

	ID3D12RootSignature*							globalSignature;

	ID3D12StateObject*								rtpso;//Pipeline State
	ID3D12StateObjectProperties*					rtpsoInfo;

	dxr_acceleration_structure_manager				acceleration_structure_manager;
	dxr_heapManager*								dxr_heapManager;
};

namespace DXR
{
	#define MEGA_INDEX_VERTEX_BUFFER_SIZE 0x0000000000ffffff //big ;) got to hold all the polys

	void SetupDXR(Dx_Instance &d3d, Dx_World &world, int vidWidth, int vidHeight);
	void BuildAccelerationStructure(Dx_Instance &d3d, DXRGlobal &dxr, Dx_World &world);

	void Create_View_CB(Dx_Instance &d3d, DXRGlobal &dxr, Dx_World &world);

	void Update_View_CB(Dx_Instance &d3d, DXRGlobal &dxr, Dx_World &world);

	void Create_Index_Vertex_Buffers(Dx_Instance &d3d, DXRGlobal &dxr, Dx_World &world);
	void Create_Bottom_Level_AS(Dx_Instance &d3d, DXRGlobal &dxr, Dx_World &world);
	void Create_Top_Level_AS(Dx_Instance &d3d, DXRGlobal &dxr);
	void Update_Top_Level_AS(Dx_Instance &d3d, DXRGlobal &dxr);
	void UpdateAccelerationStructures(Dx_Instance &d3d, DXRGlobal &dxr, Dx_World &world);

	void Create_RayGen_Program(Dx_Instance &d3d, DXRGlobal &dxr, D3DShaders::D3D12ShaderCompilerInfo &shaderCompiler);
	void Create_Miss_Program(Dx_Instance &d3d, DXRGlobal &dxr, D3DShaders::D3D12ShaderCompilerInfo &shaderCompiler);
	void Create_Closest_Hit_Program(Dx_Instance &d3d, DXRGlobal &dxr, D3DShaders::D3D12ShaderCompilerInfo &shaderCompiler);
	void Create_Global_RootSignature(Dx_Instance &d3d, DXRGlobal &dxr, D3DShaders::D3D12ShaderCompilerInfo &shaderCompiler);
	void Create_Pipeline_State_Object(Dx_Instance &d3d, DXRGlobal &dxr);
	void Create_Shader_Table(Dx_Instance &d3d, DXRGlobal &dxr, Dx_World &world);
	void Update_Shader_Table(Dx_Instance &d3d, DXRGlobal &dxr, Dx_World &world);

	void Build_Command_List(Dx_Instance &d3d, DXRGlobal &dxr, Dx_World &world);

	void Destroy(Dx_Instance &d3d, DXRGlobal &dxr);
}