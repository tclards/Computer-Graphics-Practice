#include <d3dcompiler.h>	// required for compiling shaders on the fly, consider pre-compiling instead
#include "FSLogo.h"
#pragma comment(lib, "d3dcompiler.lib") 

void PrintLabeledDebugString(const char* label, const char* toPrint)
{
	std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
	OutputDebugStringA(label);
	OutputDebugStringA(toPrint);
#endif
}

// TODO: Part 2B - DONE
// TODO: Part 4E - DONE
struct lightColor
{
	float r, g, b, a;
};

__declspec(align(16)) struct SceneData
{
	GW::MATH::GMATRIXF viewMat, projMat;

	GW::MATH::GVECTORF lightDir, lightColor, camWorldPos, sunAmbient;
};

__declspec(align(16)) struct MeshData
{
	GW::MATH::GMATRIXF worldMat;
	OBJ_ATTRIBUTES material;
};

// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	// what we need at a minimum to draw a triangle
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		indexBuffer;

	// TODO: Part 2A - DONE
	GW::MATH::GMATRIXF worldMat;
	GW::MATH::GMATRIXF FSLogo_WorldMat;
	GW::MATH::GMATRIXF FSLogo_Text_WorldMat;
	GW::MATH::GMATRIXF viewMat;
	GW::MATH::GMATRIXF perspectiveMat;
	GW::MATH::GMATRIXF camMatrix;

	GW::MATH::GVECTORF lightDir;
	GW::MATH::GVECTORF lightColor;
	GW::MATH::GVECTORF sunAmbient;

	// TODO: Part 2B - DONE
	SceneData cbuffSceneData;
	MeshData cbuffMeshData;

	// TODO: Part 2C - DONE
	Microsoft::WRL::ComPtr<ID3D11Buffer> cbuffScene;
	Microsoft::WRL::ComPtr<ID3D11Buffer> cbuffMesh;

	GW::INPUT::GInput inputProxy;
	GW::INPUT::GController controllerInputProxy;

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;

		// TODO: Part 2A - DONE
		// TODO: Part 2B - DONE
		// TODO: Part 4E - DONE
		InitializeMatricesAndVariables();
		InitializeConstantBufferData();
		IntializeGraphics();

		inputProxy.Create(win);
		controllerInputProxy.Create();
	}

private:
	//constructor helper functions
	void IntializeGraphics()
	{
		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);

		// TODO: Part 1G  - DONE
		// TODO: Part 2C - DONE
		InitializeVertexBuffer(creator);
		InitializeIndexBuffer(creator);
		InitializeConstantBuffers(creator);
		InitializePipeline(creator);

		// free temporary handle
		creator->Release();
	}

	void InitializeConstantBufferData()
	{
		cbuffSceneData.projMat = perspectiveMat;
		cbuffSceneData.viewMat = viewMat;
		cbuffSceneData.lightDir = lightDir;
		cbuffSceneData.lightColor = lightColor;
		cbuffSceneData.sunAmbient = sunAmbient;

		cbuffMeshData.worldMat = FSLogo_WorldMat;
		cbuffMeshData.material = FSLogo_materials[0].attrib;
	}

	void InitializeConstantBuffers(ID3D11Device* creator)
	{
		CreateConstantBufferScene(creator, &cbuffSceneData, sizeof(cbuffSceneData));
		CreateConstantBufferMesh(creator, &cbuffMeshData, sizeof(cbuffMeshData));
	}

	void CreateConstantBufferScene(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

		creator->CreateBuffer(&bDesc, &bData, cbuffScene.GetAddressOf());
	}
	void CreateConstantBufferMesh(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

		creator->CreateBuffer(&bDesc, &bData, cbuffMesh.GetAddressOf());
	}

	void InitializeIndexBuffer(ID3D11Device* creator)
	{
		CreateIndexBuffer(creator, &FSLogo_indices, sizeof(FSLogo_indices));
	}

	void CreateIndexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_INDEX_BUFFER);

		creator->CreateBuffer(&bDesc, &bData, &indexBuffer);
	}

	void InitializeVertexBuffer(ID3D11Device* creator)
	{
		// TODO: Part 1C - DONE
		CreateVertexBuffer(creator, FSLogo_vertices, sizeof(FSLogo_vertices));
	}

	void CreateVertexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&bDesc, &bData, vertexBuffer.GetAddressOf());
	}

	void InitializeWorldMatrix()
	{
		GW::MATH::GMATRIXF tempMat = GW::MATH::GIdentityMatrixF;

		worldMat = tempMat;
	}

	void InitializeFSLogoWorldMatrix()
	{
		GW::MATH::GMATRIXF tempMat = GW::MATH::GIdentityMatrixF;

		FSLogo_WorldMat = tempMat;
	}

	void InitializeFSLogoTextWorldMatrix()
	{
		GW::MATH::GMATRIXF tempMat = GW::MATH::GIdentityMatrixF;

		FSLogo_Text_WorldMat = tempMat;
	}

	void InitializePerspectiveMatrix()
	{
		GW::MATH::GMATRIXF tempMat = GW::MATH::GIdentityMatrixF;

		float aspect = 0.0f;
		d3d.GetAspectRatio(aspect);

		float fov = G_DEGREE_TO_RADIAN_F(65.0f);

		GW::MATH::GMatrix::ProjectionDirectXLHF(fov, aspect, 0.1f, 100.0f, tempMat);

		perspectiveMat = tempMat;
	}

	void InitializeCameraMatrix()
	{
		GW::MATH::GMATRIXF tempMat = GW::MATH::GIdentityMatrixF;

		// inverse view into cam
		GW::MATH::GMatrix::InverseF(viewMat, tempMat);

		camMatrix = tempMat;
	}

	void InitializeViewMatrix()
	{
		GW::MATH::GMATRIXF tempMat = GW::MATH::GIdentityMatrixF;

		GW::MATH::GVECTORF eye =
		{
			0.75f, 0.25f, -1.5f, 1.0f
		};

		GW::MATH::GVECTORF at =
		{
			0.15f, 0.75f, 0.0f, 1.0f
		};
		GW::MATH::GVECTORF up =
		{
			0.0f, 1.0f, 0.0f, 0.0f
		};

		GW::MATH::GMatrix::LookAtLHF(eye, at, up, tempMat);

		viewMat = tempMat;
	}

	void InitializeMatricesAndVariables()
	{
		// initialize matrices
		InitializeWorldMatrix();
		InitializeViewMatrix();
		InitializePerspectiveMatrix();
		InitializeFSLogoWorldMatrix();
		InitializeFSLogoTextWorldMatrix();
		InitializeCameraMatrix();

		// initialize variables
		lightDir =
		{
			-1.0f, -1.0f, 2.0f, 1
		};
		GW::MATH::GVector::NormalizeF(lightDir, lightDir);
		lightColor =
		{
			229.0f / 255.0f,
			229.0f / 255.0f,
			255.0f / 255.0f,
			255.0f / 255.0f
		};
		sunAmbient =
		{
			63.75f / 255.0f,
			63.75f / 255.0f,
			89.25f / 255.0f,
			0
		};
	}

	void InitializePipeline(ID3D11Device* creator)
	{
		UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		compilerFlags |= D3DCOMPILE_DEBUG;
#endif
		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = CompileVertexShader(creator, compilerFlags);
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob = CompilePixelShader(creator, compilerFlags);

		CreateVertexInputLayout(creator, vsBlob);
	}

	Microsoft::WRL::ComPtr<ID3DBlob> CompileVertexShader(ID3D11Device* creator, UINT compilerFlags)
	{
		std::string vertexShaderSource = ReadFileIntoString("../Shaders/VertexShader.hlsl");

		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;

		HRESULT compilationResult =
			D3DCompile(vertexShaderSource.c_str(), vertexShaderSource.length(),
				nullptr, nullptr, nullptr, "main", "vs_4_0", compilerFlags, 0,
				vsBlob.GetAddressOf(), errors.GetAddressOf());

		if (SUCCEEDED(compilationResult))
		{
			creator->CreateVertexShader(vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(), nullptr, vertexShader.GetAddressOf());
		}
		else
		{
			PrintLabeledDebugString("Vertex Shader Errors:\n", (char*)errors->GetBufferPointer());
			abort();
			return nullptr;
		}

		return vsBlob;
	}

	Microsoft::WRL::ComPtr<ID3DBlob> CompilePixelShader(ID3D11Device* creator, UINT compilerFlags)
	{
		std::string pixelShaderSource = ReadFileIntoString("../Shaders/PixelShader.hlsl");

		Microsoft::WRL::ComPtr<ID3DBlob> psBlob, errors;

		HRESULT compilationResult =
			D3DCompile(pixelShaderSource.c_str(), pixelShaderSource.length(),
				nullptr, nullptr, nullptr, "main", "ps_4_0", compilerFlags, 0,
				psBlob.GetAddressOf(), errors.GetAddressOf());

		if (SUCCEEDED(compilationResult))
		{
			creator->CreatePixelShader(psBlob->GetBufferPointer(),
				psBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());
		}
		else
		{
			PrintLabeledDebugString("Pixel Shader Errors:\n", (char*)errors->GetBufferPointer());
			abort();
			return nullptr;
		}

		return psBlob;
	}

	void CreateVertexInputLayout(ID3D11Device* creator, Microsoft::WRL::ComPtr<ID3DBlob>& vsBlob)
	{
		// TODO: Part 1E - DONE
		D3D11_INPUT_ELEMENT_DESC attributes[3];

		attributes[0].SemanticName = "POS";
		attributes[0].SemanticIndex = 0;
		attributes[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[0].InputSlot = 0;
		attributes[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[0].InstanceDataStepRate = 0;

		attributes[1].SemanticName = "UVM";
		attributes[1].SemanticIndex = 0;
		attributes[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[1].InputSlot = 0;
		attributes[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[1].InstanceDataStepRate = 0;

		attributes[2].SemanticName = "NRM";
		attributes[2].SemanticIndex = 0;
		attributes[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[2].InputSlot = 0;
		attributes[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[2].InstanceDataStepRate = 0;

		creator->CreateInputLayout(attributes, ARRAYSIZE(attributes),
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
			vertexFormat.GetAddressOf());
	}

public:
	void Render()
	{
		PipelineHandles curHandles = GetCurrentPipelineHandles();
		SetUpPipeline(curHandles);
		// TODO: Part 1H - DONE
		// TODO: Part 3B - DONE
		// TODO: Part 3C - DONE
		// TODO: Part 4D - DONE
		D3D11_MAPPED_SUBRESOURCE sub1 = { 0 };
		D3D11_MAPPED_SUBRESOURCE sub2 = { 0 };

		curHandles.context->Map(cbuffScene.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &sub1);
		memcpy(sub1.pData, &cbuffSceneData, sizeof(cbuffSceneData));
		curHandles.context->Unmap(cbuffScene.Get(), 0);

		for (int i = 0; i < std::size(FSLogo_meshes); i++)
		{
			int index = FSLogo_meshes[i].materialIndex;
			cbuffMeshData.material = FSLogo_materials[index].attrib;

			if (index == 0)
			{
				cbuffMeshData.worldMat = FSLogo_Text_WorldMat;
			}
			else if (index == 1)
			{
				cbuffMeshData.worldMat = FSLogo_WorldMat;
			}

			curHandles.context->Map(cbuffMesh.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &sub2);
			memcpy(sub2.pData, &cbuffMeshData, sizeof(cbuffMeshData));
			curHandles.context->Unmap(cbuffMesh.Get(), 0);

			curHandles.context->DrawIndexed(FSLogo_meshes[i].indexCount, FSLogo_meshes[i].indexOffset, 0); // TODO: Part 1D - DONE
		}

		ReleasePipelineHandles(curHandles);
	}

	void Update()
	{
		// Time keeping code
		std::chrono::high_resolution_clock::time_point lastUpdate;
		float deltaTime;
		auto now = std::chrono::high_resolution_clock::now();
		deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - lastUpdate).count() / 1000000.0f;

		// Camera movement variables
		float totalY = 0.0f;
		float totalZ = 0.0f;
		float totalX = 0.0f;
		float totalPitch = 0.0f;
		float totalYaw = 0.0f;
		float totalRot = 0.0f;
		const float camMoveSpeed = 0.3f;
		float perFrameSpeed = camMoveSpeed * deltaTime;
		float thumbstickSpeed = 3.14 * deltaTime;
		UINT screenHeight = 0;
		win.GetClientHeight(screenHeight);
		UINT screenWidth = 0;
		win.GetClientWidth(screenWidth);
		float aspect = 0.0f;
		d3d.GetAspectRatio(aspect);

		// Camera Input variables
		float spaceKeyState = 0.0f;
		float leftShiftKeyState = 0.0f;
		float rightTriggerState = 0.0f;
		float leftTriggerState = 0.0f;
		float wState = 0.0f;
		float sState = 0.0f;
		float dState = 0.0f;
		float aState = 0.0f;
		float leftStickX = 0.0f;
		float leftStickY = 0.0f;
		float rightStickY = 0.0f;
		float rightStickX = 0.0f;
		float mouseDeltaY = 0.0f;
		float mouseDeltaX = 0.0f;
		float qKeyState = 0.0f;
		float eKeyState = 0.0f;
		float leftButtonKeyState = 0.0f;
		float rightButtonKeyState = 0.0f;

		// get user input:
		// up/down input
		inputProxy.GetState(G_KEY_SPACE, spaceKeyState);
		inputProxy.GetState(G_KEY_LEFTSHIFT, leftShiftKeyState);
		controllerInputProxy.GetState(0, G_RIGHT_TRIGGER_AXIS, rightTriggerState);
		controllerInputProxy.GetState(0, G_LEFT_TRIGGER_AXIS, leftTriggerState);
		totalY = spaceKeyState - leftShiftKeyState + rightTriggerState - leftTriggerState;
		// local translations input for forward/back & strafe left/right
		inputProxy.GetState(G_KEY_W, wState);
		inputProxy.GetState(G_KEY_S, sState);
		controllerInputProxy.GetState(0, G_LX_AXIS, leftStickX);
		controllerInputProxy.GetState(0, G_LY_AXIS, leftStickY);
		inputProxy.GetState(G_KEY_D, dState);
		inputProxy.GetState(G_KEY_A, aState);
		totalZ = wState - sState + leftStickY;
		totalX = dState - aState + leftStickX;
		// Rotation input 1 (mouse/sticks)
		if (inputProxy.GetMouseDelta(mouseDeltaX, mouseDeltaY) == GW::GReturn::REDUNDANT)
		{
			mouseDeltaX = 0.0f;
			mouseDeltaY = 0.0f;
		}
		if (controllerInputProxy.GetState(0, G_RY_AXIS, rightStickY) == GW::GReturn::REDUNDANT)
		{
			rightStickY = 0.0f;;
		}
		if (controllerInputProxy.GetState(0, G_RX_AXIS, rightStickX) == GW::GReturn::REDUNDANT)
		{
			rightStickX = 0.0f;;
		}
		totalPitch = (65.0f * mouseDeltaY) / screenHeight + rightStickY * thumbstickSpeed * -1;
		totalYaw = (65.0f * aspect * mouseDeltaX) / screenWidth + rightStickX * thumbstickSpeed;
		// rotation input 2 (Q/E & left/right buttons)
		inputProxy.GetState(G_KEY_Q, qKeyState);
		inputProxy.GetState(G_KEY_E, eKeyState);
		controllerInputProxy.GetState(0, G_WEST_BTN, leftButtonKeyState);
		controllerInputProxy.GetState(0, G_EAST_BTN, rightButtonKeyState);
		totalRot = qKeyState - eKeyState + leftButtonKeyState - rightButtonKeyState;

		// final values to apply to camera
		float appliedY = 0.0f;
		float appliedZ = 0.0f;
		float appliedX = 0.0f;
		float appliedPitch = 0.0f;
		float appliedYaw = 0.0f;
		float appliedRot = 0.0f;

		// manipulate camera matrix by applying user input:
		// Up/Down
		appliedY += 0.000005f * (totalY * camMoveSpeed * deltaTime);
		GW::MATH::GVECTORF camTranslateY =
		{
			0, appliedY, 0, 0
		};
		GW::MATH::GMatrix::TranslateGlobalF(camMatrix, camTranslateY, camMatrix);

		// forward/backwards & left/right strafing
		appliedZ = 0.000005f * (totalZ * perFrameSpeed);
		appliedX = 0.000005f * (totalX * perFrameSpeed);
		GW::MATH::GVECTORF camTranslateZXVec =
		{
			appliedX, 0, appliedZ, 0
		};
		GW::MATH::GMatrix::TranslateLocalF(camMatrix, camTranslateZXVec, camMatrix);

		//rotate left/right
		appliedPitch = 0.05f * totalPitch;
		GW::MATH::GMatrix::RotateXLocalF(camMatrix, appliedPitch, camMatrix);

		// turn left/right
		appliedYaw = 0.05f * totalYaw;

		GW::MATH::GMatrix::RotateYLocalF(camMatrix, appliedYaw, camMatrix);

		// apply rotation2
		appliedRot = 0.05f * totalRot;
		GW::MATH::GMatrix::RotateZLocalF(camMatrix, appliedRot, camMatrix);

		// record cameras world position for use in pixel shader
		GW::MATH::GMATRIXF tempMat = camMatrix;
		GW::MATH::GVECTORF tempVec = camMatrix.row4;
		cbuffSceneData.camWorldPos = tempVec;

		// inverse camera matrix and assign it back into view matrix
		GW::MATH::GMatrix::InverseF(camMatrix, viewMat);

		// Apply changes to constantBuffer
		cbuffSceneData.viewMat = viewMat; 

		// Logo Rotation Vars
		float degreeOfChangePerFrame = G2D_DEGREE_TO_RADIAN_F(1.0f);
		float appliedRotationPerFrame = (0.000009 * (degreeOfChangePerFrame * deltaTime));

		GW::MATH::GMatrix::RotateYGlobalF(FSLogo_WorldMat, appliedRotationPerFrame, FSLogo_WorldMat);
	}

private:
	struct PipelineHandles
	{
		ID3D11DeviceContext* context;
		ID3D11RenderTargetView* targetView;
		ID3D11DepthStencilView* depthStencil;
	};

	PipelineHandles GetCurrentPipelineHandles()
	{
		PipelineHandles retval;
		d3d.GetImmediateContext((void**)&retval.context);
		d3d.GetRenderTargetView((void**)&retval.targetView);
		d3d.GetDepthStencilView((void**)&retval.depthStencil);
		return retval;
	}

	void SetUpPipeline(PipelineHandles handles)
	{
		SetRenderTargets(handles);
		SetVertexBuffers(handles);
		SetShaders(handles);

		handles.context->IASetInputLayout(vertexFormat.Get());
		handles.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		handles.context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		ID3D11Buffer* const constantBuffers[] = { cbuffScene.Get(), cbuffMesh.Get() };

		handles.context->VSSetConstantBuffers(0, 2, constantBuffers);
		handles.context->PSSetConstantBuffers(0, 2, constantBuffers);
	}

	void SetRenderTargets(PipelineHandles handles)
	{
		ID3D11RenderTargetView* const views[] = { handles.targetView };
		handles.context->OMSetRenderTargets(ARRAYSIZE(views), views, handles.depthStencil);
	}

	void SetVertexBuffers(PipelineHandles handles)
	{
		const UINT strides[] = { sizeof(OBJ_VERT) }; // TODO: Part 1E - DONE
		const UINT offsets[] = { 0 };
		ID3D11Buffer* const buffs[] = { vertexBuffer.Get() };
		handles.context->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
	}

	void SetShaders(PipelineHandles handles)
	{
		handles.context->VSSetShader(vertexShader.Get(), nullptr, 0);
		handles.context->PSSetShader(pixelShader.Get(), nullptr, 0);
	}

	void ReleasePipelineHandles(PipelineHandles toRelease)
	{
		toRelease.depthStencil->Release();
		toRelease.targetView->Release();
		toRelease.context->Release();
	}


public:
	~Renderer()
	{
		// ComPtr will auto release so nothing to do here yet 
	}
};