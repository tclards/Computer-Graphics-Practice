#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib") //needed for runtime shader compilation. Consider compiling shaders before runtime 

void PrintLabeledDebugString(const char* label, const char* toPrint)
{
	std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
	OutputDebugStringA(label);
	OutputDebugStringA(toPrint);
#endif
}

// TODO: Part 1C - DONE
// TODO: Part 2B - DONE
// TODO: Part 2G - DONE
struct Vertex
{
	float x, y, z, w;
};

struct SHADER_VARS
{
	GW::MATH::GMATRIXF world;
	GW::MATH::GMATRIXF view;
	GW::MATH::GMATRIXF perspective;
};

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

	// TODO: Part 2A - DONE
	// TODO: Part 2G - DONE
	// TODO: Part 3A - DONE
	// TODO: Part 3C - DONE
	GW::MATH::GMATRIXF worldMatrix_1;
	GW::MATH::GMATRIXF worldMatrix_2;
	GW::MATH::GMATRIXF worldMatrix_3;
	GW::MATH::GMATRIXF worldMatrix_4;
	GW::MATH::GMATRIXF worldMatrix_5;
	GW::MATH::GMATRIXF worldMatrix_6;
	GW::MATH::GMATRIXF perspectiveMatrix;
	GW::MATH::GMATRIXF viewMatrix;

	// TODO: Part 2C - DONE
	// TODO: Part 2D - DONE
	SHADER_VARS SV;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

	// TODO: Part 4A - DONE
	GW::INPUT::GInput inputProxy;
	GW::INPUT::GController controllerInputProxy;

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;

		// TODO: Part 2A - DONE
		// TODO: Part 3C - DONE
		// TODO: Part 2C - DONE
		// TODO: Part 2G - DONE
		// TODO: Part 3A - DONE
		// TODO: Part 3B - DONE

		InitializeMatrices();
		SV.world = worldMatrix_1;
		SV.view = viewMatrix;
		SV.perspective = perspectiveMatrix;

		// TODO: Part 4A - DONE
		inputProxy.Create(win);
		controllerInputProxy.Create();
		InitializeGraphics();
	}
private:
	//Constructor helper functions 
	void InitializeGraphics()
	{
		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);

		//TODO: Part 2D - DONE
		InitializeVertexBuffer(creator);
		InitializeConstantBuffer(creator);
		InitializePipeline(creator);

		// free temporary handle
		creator->Release();
	}

	void InitializeMatrices()
	{
		InitializeWorldMatrix01();
		InitializeWorldMatrix02();
		InitializeWorldMatrix03();
		InitializeWorldMatrix04();
		InitializeWorldMatrix05();
		InitializeWorldMatrix06();
		InitializeViewMatrix();
		InitializePerspectiveMatrix();
	}

	// helper function for constant buffer
	void InitializeConstantBuffer(ID3D11Device* creator)
	{
		CreateConstantBuffer(creator, &SV, sizeof(SV));
	}

	void InitializeVertexBuffer(ID3D11Device* creator)
	{
		// TODO: Part 1B - DONE
		// TODO: Part 1C - DONE
		// TODO: Part 1D - DONE

		Vertex Triangle_Vertices[] =
		{
			{0, 0.5f, 0, 1},
			{0.5f, -0.5f, 0, 1},

			{0.5f, -0.5f, 0, 1},
			{-0.5f, -0.5f, 0, 1},

			{-0.5f, -0.5f, 0, 1},
			{0, 0.5f, 0, 1}
		};

		// Grid Data
		Vertex verts[104];
		CreateGridData(verts);

		CreateVertexBuffer(creator, &verts[0], sizeof(verts));
	}

	void InitializePerspectiveMatrix()
	{
		GW::MATH::GMATRIXF tempMat = GW::MATH::GIdentityMatrixF;

		float aspect = 0.0f;
		d3d.GetAspectRatio(aspect);

		float fov = G_DEGREE_TO_RADIAN_F(65.0f);

		GW::MATH::GMatrix::ProjectionDirectXLHF(fov, aspect, 0.1f, 100.0f, tempMat);

		perspectiveMatrix = tempMat;
	}

	// Helper function to initialize view matrix 01
	void InitializeViewMatrix()
	{
		GW::MATH::GMATRIXF tempMat = GW::MATH::GIdentityMatrixF;

		GW::MATH::GVECTORF viewMat_01_Translate =
		{
			0.25f, -0.125f, -0.25, 1
		};
		GW::MATH::GMatrix::TranslateGlobalF(tempMat, viewMat_01_Translate, tempMat);

		GW::MATH::GVECTORF eye =
		{
			0.25f, -0.125f, -0.25, 1
		};

		GW::MATH::GVECTORF at =
		{
			0, 0, 0, 0
		};

		GW::MATH::GVECTORF up =
		{
			0, 1, 0, 0
		};

		GW::MATH::GMatrix::LookAtLHF(eye, at, up, tempMat);

		viewMatrix = tempMat;
	}

	// Helper function to initialize World Matrix 01
	void InitializeWorldMatrix01()
	{
		GW::MATH::GMATRIXF tempMat = GW::MATH::GIdentityMatrixF;

		GW::MATH::GVECTORF worldMat_01_yTranslate =
		{
			0, -0.5f, 0, 1
		};
		GW::MATH::GMatrix::TranslateGlobalF(tempMat, worldMat_01_yTranslate, tempMat);

		GW::MATH::GMatrix::RotateXGlobalF(tempMat, 1.5708f, tempMat);

		worldMatrix_1 = tempMat;
	}

	void InitializeWorldMatrix02()
	{
		GW::MATH::GMATRIXF tempMat = GW::MATH::GIdentityMatrixF;

		GW::MATH::GVECTORF worldMat_02_Translate =
		{
			0, 0, 0.5f, 1
		};
		GW::MATH::GMatrix::TranslateGlobalF(tempMat, worldMat_02_Translate, tempMat);

		GW::MATH::GMatrix::RotateZGlobalF(tempMat, 1.5708f, tempMat);

		worldMatrix_2 = tempMat;
	}

	void InitializeWorldMatrix03()
	{
		GW::MATH::GMATRIXF tempMat = GW::MATH::GIdentityMatrixF;

		GW::MATH::GVECTORF worldMat_03_Translate =
		{
			0, 0, -0.5f, 1
		};
		GW::MATH::GMatrix::TranslateGlobalF(tempMat, worldMat_03_Translate, tempMat);

		GW::MATH::GMatrix::RotateZGlobalF(tempMat, 1.5708f, tempMat);

		worldMatrix_3 = tempMat;
	}

	void InitializeWorldMatrix04()
	{
		GW::MATH::GMATRIXF tempMat = GW::MATH::GIdentityMatrixF;

		GW::MATH::GVECTORF worldMat_04_Translate =
		{
			-0.5f, 0, 0, 1
		};
		GW::MATH::GMatrix::TranslateGlobalF(tempMat, worldMat_04_Translate, tempMat);

		GW::MATH::GMatrix::RotateYGlobalF(tempMat, 1.5708f, tempMat);

		worldMatrix_4 = tempMat;
	}

	void InitializeWorldMatrix05()
	{
		GW::MATH::GMATRIXF tempMat = GW::MATH::GIdentityMatrixF;

		GW::MATH::GVECTORF worldMat_05_Translate =
		{
			0.5f, 0, 0, 1
		};
		GW::MATH::GMatrix::TranslateGlobalF(tempMat, worldMat_05_Translate, tempMat);

		GW::MATH::GMatrix::RotateYGlobalF(tempMat, 1.5708f, tempMat);

		worldMatrix_5 = tempMat;
	}

	void InitializeWorldMatrix06()
	{
		GW::MATH::GMATRIXF tempMat = GW::MATH::GIdentityMatrixF;

		GW::MATH::GVECTORF worldMat_01_yTranslate =
		{
			0, 0.5f, 0, 1
		};
		GW::MATH::GMatrix::TranslateGlobalF(tempMat, worldMat_01_yTranslate, tempMat);

		GW::MATH::GMatrix::RotateXGlobalF(tempMat, 1.5708f, tempMat);

		worldMatrix_6 = tempMat;
	}

	// Helper function to build grid data
	void CreateGridData(Vertex* verts)
	{
		float posChange = 1.0f / 25;
		float xPos = -0.5f;
		float yPos = 0.5f;

		int yCounter = 0;
		int rowNum = 0;

		int xCounter = 0;
		int colNum = 0;

		// ROWS:
		// Generate 52 vertices to make up the 26 rows of x-axis aligned lines
		for (int v = 0; v < 52; v++)
		{
			// update yPosition every 2 vertices (every 1 line)
			if (yCounter == 2)
			{
				rowNum++;
				yPos = (0.5f - (rowNum * posChange));
				yCounter = 0;
			}

			// update xPosition
			if (v % 2 == 0) // left side vertex
			{
				xPos = -0.5f;
			}
			else // right side vertex
			{
				xPos = 0.5f;
			}

			// Create vertex
			verts[v] = { xPos, yPos, 0, 1 };

			// update yCounter
			yCounter++;
		}

		xPos = -0.5f;
		yPos = 0.5f;

		// COLUMNS
		// Generate 52 vertices to make up the 26 columns of y-axis aligned lines
		for (int v = 52; v < 104; v++)
		{
			// update xPosition every 2 vertices (every 1 line)
			if (xCounter == 2)
			{
				colNum++;
				xPos = (-0.5f + (colNum * posChange));
				xCounter = 0;
			}

			// update yPosition
			if (v % 2 == 0) // top side vertex
			{
				yPos = 0.5f;
			}
			else // bottom side vertex
			{
				yPos = -0.5f;
			}

			// Create vertex
			verts[v] = { xPos, yPos, 0, 1 };

			// update xCounter
			xCounter++;
		}
	}

	void CreateVertexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&bDesc, &bData, vertexBuffer.GetAddressOf());
	}

	// Helper Function to create Constant Buffer
	void CreateConstantBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		creator->CreateBuffer(&bDesc, &bData, constantBuffer.GetAddressOf());
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
		// TODO: Part 1C - DONE
		D3D11_INPUT_ELEMENT_DESC attributes[1];

		attributes[0].SemanticName = "POSITION";
		attributes[0].SemanticIndex = 0;
		attributes[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		attributes[0].InputSlot = 0;
		attributes[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[0].InstanceDataStepRate = 0;

		creator->CreateInputLayout(attributes, ARRAYSIZE(attributes),
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
			vertexFormat.GetAddressOf());
	}

public:
	void Render()
	{
		PipelineHandles curHandles = GetCurrentPipelineHandles();
		SetUpPipeline(curHandles);
		// TODO: Part 1B - DONE
		// TODO: Part 1D - DONE
		// TODO: Part 3D - DONE
		D3D11_MAPPED_SUBRESOURCE sub = { 0 };

		SV.world = worldMatrix_1;
		curHandles.context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
		memcpy(sub.pData, &SV, sizeof(SV));
		curHandles.context->Unmap(constantBuffer.Get(), 0);
		curHandles.context->Draw(104, 0);

		SV.world = worldMatrix_2;
		curHandles.context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
		memcpy(sub.pData, &SV, sizeof(SV));
		curHandles.context->Unmap(constantBuffer.Get(), 0);
		curHandles.context->Draw(104, 0);

		SV.world = worldMatrix_3;
		curHandles.context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
		memcpy(sub.pData, &SV, sizeof(SV));
		curHandles.context->Unmap(constantBuffer.Get(), 0);
		curHandles.context->Draw(104, 0);

		SV.world = worldMatrix_4;
		curHandles.context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
		memcpy(sub.pData, &SV, sizeof(SV));
		curHandles.context->Unmap(constantBuffer.Get(), 0);
		curHandles.context->Draw(104, 0);

		SV.world = worldMatrix_5;
		curHandles.context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
		memcpy(sub.pData, &SV, sizeof(SV));
		curHandles.context->Unmap(constantBuffer.Get(), 0);
		curHandles.context->Draw(104, 0);

		SV.world = worldMatrix_6;
		curHandles.context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
		memcpy(sub.pData, &SV, sizeof(SV));
		curHandles.context->Unmap(constantBuffer.Get(), 0);
		curHandles.context->Draw(104, 0);

		ReleasePipelineHandles(curHandles);
	}

	// TODO: Part 4B - DONE
	// TODO: Part 4C - DONE
	// TODO: Part 4D - DONE
	// TODO: Part 4E - DONE
	// TODO: Part 4F - DONE
	// TODO: Part 4G - DONE
	GW::MATH::GMATRIXF camMatrix = GW::MATH::GIdentityMatrixF;

	void UpdateCamera()
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

		// inverse view into cam
		GW::MATH::GMatrix::InverseF(viewMatrix, camMatrix);

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
		// Rotation input 
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
		appliedY += 0.0000005f * (totalY * camMoveSpeed * deltaTime);
		GW::MATH::GVECTORF camTranslateY =
		{
			0, appliedY, 0, 0
		};
		GW::MATH::GMatrix::TranslateLocalF(camMatrix, camTranslateY, camMatrix);

		// forward/backwards & left/right strafing
		appliedZ = 0.0000005f * (totalZ * perFrameSpeed);
		appliedX = 0.0000005f * (totalX * perFrameSpeed);
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

		// inverse camera matrix and assign it back into view matrix
		GW::MATH::GMatrix::InverseF(camMatrix, viewMatrix);

		// Apply changes to constantBuffer
		SV.view = viewMatrix;
	}

private:
	struct PipelineHandles
	{
		ID3D11DeviceContext* context;
		ID3D11RenderTargetView* targetView;
		ID3D11DepthStencilView* depthStencil;
	};
	//Render helper functions
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
		//TODO: Part 2E - DONE
		ID3D11Buffer* const constBuffs[] = { constantBuffer.Get() };
		handles.context->VSSetConstantBuffers(0, ARRAYSIZE(constBuffs), constBuffs);
		handles.context->IASetInputLayout(vertexFormat.Get());
		handles.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST); //TODO: Part 1B - DONE
	}

	void SetRenderTargets(PipelineHandles handles)
	{
		ID3D11RenderTargetView* const views[] = { handles.targetView };
		handles.context->OMSetRenderTargets(ARRAYSIZE(views), views, handles.depthStencil);
	}

	void SetVertexBuffers(PipelineHandles handles)
	{
		// TODO: Part 1C - DONE
		const UINT strides[] = { sizeof(float) * 4 };
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
