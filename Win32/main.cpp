
//Include and link appropriate libraries and headers//
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <xnamath.h>

//Global Declarations//
IDXGISwapChain* SwapChain;
ID3D11Device* d3d11Device;
ID3D11DeviceContext* d3d11DevCon;
ID3D11RenderTargetView* renderTargetView;

ID3D11Buffer* squareIndexBuffer;
ID3D11Buffer* squareVertBuffer;

ID3D11VertexShader* VS;
ID3D11PixelShader* PS;
ID3D10Blob* VS_Buffer;
ID3D10Blob* PS_Buffer;
ID3D11InputLayout* vertLayout;

//Define variables/constants//
LPCTSTR WndClassName = L"firstwindow";	//Define our window class name
HWND hwnd = NULL;	//Sets our windows handle to NULL
HRESULT hr;

const int Width  = 800;	//window width
const int Height = 600;	//window height

//Function Prototypes//
bool InitializeDirect3d11App(HINSTANCE hInstance);
void ReleaseObjects();
bool InitScene();
void UpdateScene();
void DrawScene();

bool InitializeWindow(HINSTANCE hInstance,	//Initialize our window
		 int ShowWnd,
		 int width, int height,
		 bool windowed);

int messageloop();	//Main part of the program

LRESULT CALLBACK WndProc(HWND hWnd,	//Windows callback procedure
		 UINT msg,
		 WPARAM wParam,
		 LPARAM lParam);

//Vertex Structure and Vertex Layout (Input Layout)//
struct Vertex    //Overloaded Vertex Structure
{
    Vertex(){}
    Vertex(float x, float y, float z,
        float cr, float cg, float cb, float ca)
        : pos(x,y,z), color(cr, cg, cb, ca){}

    XMFLOAT3 pos;
    XMFLOAT4 color;
};

D3D11_INPUT_ELEMENT_DESC layout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
UINT numElements = ARRAYSIZE(layout);

int WINAPI WinMain(HINSTANCE hInstance,	//Main windows function
		   HINSTANCE hPrevInstance, 
		   LPSTR lpCmdLine,
		   int nShowCmd)
{
	//Initialize Window//
	if(!InitializeWindow(hInstance, nShowCmd, Width, Height, true))
	{
		//If initialization failed, display an error message
		MessageBox(0, L"Window Initialization - Failed",
        			L"Error", MB_OK);
		return 0;
	}

	if (!InitializeDirect3d11App(hInstance))    //Initialize Direct3D
	{
		MessageBox(0, L"Direct3D Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}

	if (!InitScene())    //Initialize our scene
	{
		MessageBox(0, L"Scene Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}

	messageloop();	//Jump into the heart of our program

	ReleaseObjects();

	return 0;
}

bool InitializeDirect3d11App(HINSTANCE hInstance)
{
	//Describe our Buffer
	DXGI_MODE_DESC bufferDesc;

	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

	bufferDesc.Width = Width;
	bufferDesc.Height = Height;
	bufferDesc.RefreshRate.Numerator = 60;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//Describe our SwapChain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapChainDesc.BufferDesc = bufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;


	//Create our SwapChain
	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
		D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &d3d11Device, NULL, &d3d11DevCon);

	//Create our BackBuffer
	ID3D11Texture2D* BackBuffer;
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);

	//Create our Render Target
	d3d11Device->CreateRenderTargetView(BackBuffer, NULL, &renderTargetView);
	BackBuffer->Release();

	//Set our Render Target
	d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, NULL);

	return true;
}

void ReleaseObjects()
{
	//Release the COM Objects we created
	SwapChain->Release();
	d3d11Device->Release();
	d3d11DevCon->Release();
	squareVertBuffer->Release();
	squareIndexBuffer->Release();
	VS->Release();
	PS->Release();
	VS_Buffer->Release();
	PS_Buffer->Release();
	vertLayout->Release();
}
bool InitScene()
{
	//Compile Shaders from shader file
	hr = D3DX11CompileFromFile(L"Effects.fx", 0, 0, "VS", "vs_4_0", 0, 0, 0, &VS_Buffer, 0, 0);
	hr = D3DX11CompileFromFile(L"Effects.fx", 0, 0, "PS", "ps_4_0", 0, 0, 0, &PS_Buffer, 0, 0);

	//Create the Shader Objects
	hr = d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
	hr = d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);

	//Set Vertex and Pixel Shaders
	d3d11DevCon->VSSetShader(VS, 0, 0);
	d3d11DevCon->PSSetShader(PS, 0, 0);

	//Create the vertex buffer
	Vertex v[] =
	{
		Vertex(-0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f),
		Vertex(-0.5f,  0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f),
		Vertex(0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f),
		Vertex(0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f),
	};

	DWORD indices[] = {
		0, 1, 2,
		0, 2, 3,
	};

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * 2 * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = indices;
	d3d11Device->CreateBuffer(&indexBufferDesc, &iinitData, &squareIndexBuffer);

	d3d11DevCon->IASetIndexBuffer(squareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * 4;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = v;
	hr = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &squareVertBuffer);

	//Set the vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	d3d11DevCon->IASetVertexBuffers(0, 1, &squareVertBuffer, &stride, &offset);

	//Create the Input Layout
	d3d11Device->CreateInputLayout(layout, numElements, VS_Buffer->GetBufferPointer(),
		VS_Buffer->GetBufferSize(), &vertLayout);

	//Set the Input Layout
	d3d11DevCon->IASetInputLayout(vertLayout);

	//Set Primitive Topology
	d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Create the Viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = Width;
	viewport.Height = Height;

	//Set the Viewport
	d3d11DevCon->RSSetViewports(1, &viewport);

	return true;
}

void UpdateScene()
{

}

void DrawScene()
{
	//Clear our backbuffer
	float bgColor[4] = { (0.0f, 0.0f, 0.0f, 0.0f) };
	d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);

	//Draw the triangle
	d3d11DevCon->DrawIndexed(6, 0, 0);

	//Present the backbuffer to the screen
	SwapChain->Present(0, 0);
}

bool InitializeWindow(HINSTANCE hInstance,	//Initialize our window
						 int ShowWnd,
						 int width, int height,
						 bool windowed)
{
	//Start creating the window//

	WNDCLASSEX wc;	//Create a new extended windows class

	wc.cbSize = sizeof(WNDCLASSEX);	//Size of our windows class
	wc.style = CS_HREDRAW | CS_VREDRAW;	//class styles
	wc.lpfnWndProc = WndProc;	//Default windows procedure function
	wc.cbClsExtra = NULL;	//Extra bytes after our wc structure
	wc.cbWndExtra = NULL;	//Extra bytes after our windows instance
	wc.hInstance = hInstance;	//Instance to current application
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);	//Title bar Icon
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);	//Default mouse Icon
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);	//Window bg color
	wc.lpszMenuName = NULL;	//Name of the menu attached to our window
	wc.lpszClassName = WndClassName;	//Name of our windows class
	wc.hIconSm = LoadIcon(NULL, IDI_WINLOGO); //Icon in your taskbar

	if (!RegisterClassEx(&wc))	//Register our windows class
	{
    		//if registration failed, display error
		MessageBox(NULL, L"Error registering class",	
			L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	hwnd = CreateWindowEx(	//Create our Extended Window
		NULL,	//Extended style
		WndClassName,	//Name of our windows class
		L"Window Title",	//Name in the title bar of our window
		WS_OVERLAPPEDWINDOW,	//style of our window
		CW_USEDEFAULT, CW_USEDEFAULT,	//Top left corner of window
		width,	//Width of our window
		height,	//Height of our window
		NULL,	//Handle to parent window
		NULL,	//Handle to a Menu
		hInstance,	//Specifies instance of current program
		NULL	//used for an MDI client window
		);

	if (!hwnd)	//Make sure our window has been created
	{
    	//If not, display error
		MessageBox(NULL, L"Error creating window",
			L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	ShowWindow(hwnd, ShowWnd);	//Shows our window
	UpdateWindow(hwnd);	//Its good to update our window

	return true;	//if there were no errors, return true
}

int messageloop(){	//The message loop

	MSG msg;	//Create a new message structure
	ZeroMemory(&msg, sizeof(MSG));	//clear message structure to NULL

	while(true)	//while there is a message
    {
  		//if there was a windows message
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)	//if the message was WM_QUIT
                break;	//Exit the message loop

            TranslateMessage(&msg);	//Translate the message
            
            //Send the message to default windows procedure
            DispatchMessage(&msg);
        }

		else{	//Otherewise, keep the flow going
			UpdateScene();
			DrawScene();
		}
		

	}

	return (int)msg.wParam;		//return the message

}

LRESULT CALLBACK WndProc(HWND hwnd,	//Default windows procedure
			 UINT msg,
			 WPARAM wParam,
			 LPARAM lParam)
{
switch( msg )	//Check message
{

	case WM_KEYDOWN:	//For a key down
 	  	//if escape key was pressed, display popup box
		if( wParam == VK_ESCAPE ){
			if(MessageBox(0, L"Are you sure you want to exit?",
               	L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
                    
    	       	//Release the windows allocated memory  
				DestroyWindow(hwnd);
		}
	return 0;

	case WM_DESTROY:	//if x button in top right was pressed
		PostQuitMessage(0);
	return 0;
}
    //return the message for windows to handle it
	return DefWindowProc(hwnd,	
			 msg,
			 wParam,
			 lParam);
}
