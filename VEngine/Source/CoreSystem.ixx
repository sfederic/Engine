module;
#include <Windows.h>
#include <stdint.h>
#include <stdio.h>
import Debug;
import CoreSystem;
import UISystem;
import Input;
import RenderSystem;
import Camera;
import GlobalDefines;
import EditorSystem;
export module CoreSystem;

LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

export class CoreSystem
{
public:
	HWND mainWindow;
	MSG msg;
	int windowWidth;
	int windowHeight;

	__int64 frameStartTime;
	__int64 frameEndTime; 
	__int64 tickFrequency;
	double deltaTime;
	double deltaAccum;
	double ticks;
	double timeSinceStartup;
	uint32_t frameCount;
	uint32_t finalFrameCount;

	bool bMainLoop = true;
	bool bGamePaused = false;
	bool bGamePlayOn = false; //Runs the game state in editor within the viewport.

	void Init()
	{
#ifndef EDITOR
		bGamePlayOn = true;
#endif
	}

	void Tick()
	{
		//Start play in editor
		if (gInputSystem.GetAsyncKey(Keys::Ctrl))
		{
			if (gInputSystem.GetKeyUpState(Keys::P))
			{
				bGamePlayOn = !bGamePlayOn;
			}
		}

		//Pause game (freezes actor ticks)
		if (bGamePlayOn)
		{
			if (gInputSystem.GetKeyUpState(Keys::P, Keys::Ctrl))
			{
				bGamePaused = !bGamePaused;
			}
		}
	}

	void SetupWindow(HINSTANCE instance, int cmdShow)
	{
		WNDCLASS wc = {};
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpszClassName = "Window";
		wc.lpfnWndProc = WndProc;
		wc.hInstance = instance;
		wc.hCursor = LoadCursor(0, IDC_CROSS);

		RegisterClass(&wc);
		mainWindow = CreateWindow("Window", "VEngine", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight, 0, 0, instance, 0);
		if (!mainWindow)
		{
			HR(GetLastError());
		}
		else
		{
			ShowWindow(mainWindow, cmdShow);
			UpdateWindow(mainWindow);

			RECT window_r; RECT desktop_r;
			GetWindowRect(mainWindow, &window_r);
			GetWindowRect(GetDesktopWindow(), &desktop_r);
			int xPos = (desktop_r.right - (window_r.right - window_r.left)) / 2;
			int yPos = (desktop_r.bottom - (window_r.bottom - window_r.top)) / 2;

			SetWindowPos(mainWindow, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}
	}

	void SetTimerFrequency()
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&tickFrequency);
		ticks = 1.0 / (double)tickFrequency;
		deltaAccum = 0.0;
	}

	void StartTimer()
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&frameStartTime);
	}

	void EndTimer()
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&frameEndTime);
		deltaTime = ticks * (double)(frameEndTime - frameStartTime);

		if (deltaTime < 0.0)
		{
			deltaTime = 0.0;
		}
		else if (deltaTime > 1.0)
		{
			deltaTime = 1.0f / 60.f; //Just in case of breakpoint stuff
		}

		deltaAccum += deltaTime;
		timeSinceStartup += deltaTime;
		frameCount++;

		if (deltaAccum > 1.0)
		{
			//Win32 FPS windowtext
			char fpsText[128];
			snprintf(fpsText, sizeof(fpsText), "d3d11 | FPS: %d mspf %f.12", frameCount, deltaTime);
			SetWindowText(mainWindow, fpsText);

			finalFrameCount = frameCount;

			frameCount = 0;
			deltaAccum = 0.0;
		}
	}

	void HandleMessages()
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void HandleWin32MessagePump(UINT message, WPARAM wparam, LPARAM lparam)
	{
		switch (message)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_KEYDOWN:
			gInputSystem.StoreKeyDownInput(wparam);
			break;

		case WM_KEYUP:
			gInputSystem.StoreKeyUpInput(wparam);
			break;

		case WM_LBUTTONUP:
			gInputSystem.StoreMouseLeftUpInput();
			break;

		case WM_LBUTTONDOWN:
			gInputSystem.StoreMouseLeftDownInput();
			break;

		case WM_RBUTTONUP:
			gInputSystem.StoreMouseRightUpInput();
			break;

		case WM_RBUTTONDOWN:
			gInputSystem.StoreMouseRightDownInput();
			break;

		case WM_MBUTTONDOWN:
			gInputSystem.StoreMouseMiddleDownInput();
			break;

		case WM_MBUTTONUP:
			gInputSystem.StoreMouseMiddleUpInput();
			break;

		case WM_MOUSEWHEEL:
			if (GET_WHEEL_DELTA_WPARAM(wparam) < 0)
			{
				gInputSystem.StoreMouseWheelDown();
			}
			else
			{
				gInputSystem.StoreMouseWheelUp();
			}
			break;

		case WM_SIZE: //Careful here, D3D11's swapchain and D2D's rendertarget like to fuck around
			if (gRenderSystem.swapchain)
			{
				gRenderSystem.context->OMSetRenderTargets(0, 0, 0);

				// Release all outstanding references to the swap chain's buffers.
				for (int rtvIndex = 0; rtvIndex < gRenderSystem.frameCount; rtvIndex++)
				{
					gRenderSystem.rtvs[rtvIndex]->Release();
				}

				gRenderSystem.dsv->Release();

				gUISystem.Cleanup();

				//ID3D11Debug* debug;
				//gRenderSystem.device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debug));
				//debug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
				//gRenderSystem.context->ClearState();
				//gRenderSystem.context->Flush();

				// Preserve the existing buffer count and format.
				// Automatically choose the width and height to match the client rect for HWNDs.

#ifdef EDITOR
				gEditorSystem->SetWindowWidthHeight();
#else
				UINT width = LOWORD(lparam);
				UINT height = HIWORD(lparam);
				gCoreSystem.windowWidth = width;
				gCoreSystem.windowHeight = height;
#endif
				HR(gRenderSystem.swapchain->ResizeBuffers(gRenderSystem.frameCount,
					gCoreSystem.windowWidth, gCoreSystem.windowHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

				gRenderSystem.CreateRTVAndDSV();

				gUISystem.Init();

				gRenderSystem.matrices.proj = XMMatrixPerspectiveFovLH(XM_PI / 3, gCoreSystem.GetAspectRatio(), 0.01f, 1000.f);
				GetActiveCamera()->proj = gRenderSystem.matrices.proj;

				// Set up the viewport.
				gRenderSystem.viewport.Width = gCoreSystem.windowWidth;
				gRenderSystem.viewport.Height = gCoreSystem.windowHeight;
				gRenderSystem.viewport.MinDepth = 0.0f;
				gRenderSystem.viewport.MaxDepth = 1.0f;
				gRenderSystem.viewport.TopLeftX = 0;
				gRenderSystem.viewport.TopLeftY = 0;
				gRenderSystem.context->RSSetViewports(1, &gRenderSystem.viewport);
			}

			break;
		}
	}

	float GetAspectRatio()
	{
		return (float)((float)windowWidth / (float)windowHeight);
	}

	//Helper for exiting the engine in the main loop
	void Exit()
	{
		msg.message = WM_QUIT;
	}

	LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
	{
		gCoreSystem.HandleWin32MessagePump(message, wparam, lparam);

		return DefWindowProc(window, message, wparam, lparam);
	}
};

export CoreSystem gCoreSystem;