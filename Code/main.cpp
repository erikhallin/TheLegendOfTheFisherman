#define _WIN32_WINNT 0x0500 //Needed for GetConsoleWindow()
#include <iostream>
#include <fcntl.h> //for console
#include <stdio.h>
#include <windows.h>
#include <gl/gl.h>
#include <ctime>

#include "definitions.h"
#include "game.h"
#include "resource.h"

using namespace std;

int g_wind_size[2]={960,540};
bool g_keys[256],g_mouse_but[4];
int  g_mouse_pos[2];

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    string command_line=lpCmdLine;
    bool debug_mode=false;
    if(command_line=="debug") debug_mode=true;

    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICO));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "LD32";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex)) return 0;

    if(!debug_mode)
    {
       RECT desktop;
       const HWND hDesktop = GetDesktopWindow();
       GetWindowRect(hDesktop, &desktop);
       g_wind_size[0] = desktop.right;
       g_wind_size[1] = desktop.bottom;
    }

    //if debug mode start console
    if(debug_mode)
    {
        //Open a console window
        AllocConsole();
        //Connect console output
        HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
        int hCrt          = _open_osfhandle((long) handle_out, _O_TEXT);
        FILE* hf_out      = _fdopen(hCrt, "w");
        setvbuf(hf_out, NULL, _IONBF, 1);
        *stdout = *hf_out;
        //Connect console input
        HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
        hCrt             = _open_osfhandle((long) handle_in, _O_TEXT);
        FILE* hf_in      = _fdopen(hCrt, "r");
        setvbuf(hf_in, NULL, _IONBF, 128);
        *stdin = *hf_in;
        //Set console title
        SetConsoleTitle("Debug Console");
        HWND hwnd_console=GetConsoleWindow();
        MoveWindow(hwnd_console,g_wind_size[0],0,680,510,TRUE);

        cout<<"Software started\n";
        cout<<"Version: "<<_version<<endl;
    }
    else
    {
        ShowCursor(FALSE);//hide cursor
    }

    hwnd = CreateWindowEx(0,
                          "LD32",
                          "LD32",
                          WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, //WS_OVERLAPPEDWINDOW for window
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          g_wind_size[0],
                          g_wind_size[1],
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    EnableOpenGL(hwnd, &hDC, &hRC);

    //init game
    game Game;
    if( !Game.init(g_wind_size,g_keys,g_mouse_but,g_mouse_pos) )
    {
        cout<<"ERROR: Could not init game\n";
        //bQuit=true;
    }
    clock_t time_now=clock();
    clock_t time_last=time_now;
    clock_t time_step=_game_update_step;
    bool update_screen=true;

    while (!bQuit)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            //update only when 20ms have passed, and until game have catched up with the current time
            time_now=clock();
            while( time_last+time_step <= time_now )
            {
                time_last+=time_step;
                Game.update();//static update
                update_screen=true;
            }
            //draw, if anything updated
            if(update_screen)
            {
                update_screen=false;

                //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
                //glLoadIdentity();
                Game.draw();
                //glFlush();
                SwapBuffers(hDC);
            }
        }
    }

    DisableOpenGL(hwnd, hDC, hRC);
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE: PostQuitMessage(0); break;

        case WM_DESTROY: return 0;

        case WM_MOUSEMOVE:
        {
             g_mouse_pos[0]=LOWORD(lParam);
             g_mouse_pos[1]=HIWORD(lParam)+28;//-22 pixel shift to get same coord as drawing
        }break;

        case WM_LBUTTONDOWN:
        {
             g_mouse_but[0]=true;
        }
        break;

        case WM_LBUTTONUP:
        {
             g_mouse_but[0]=false;
        }
        break;

        case WM_RBUTTONDOWN:
        {
             g_mouse_but[1]=true;
             cout<<"x: "<<g_mouse_pos[0]<<", y: "<<g_mouse_pos[1]<<endl; //temp
        }
        break;

        case WM_RBUTTONUP:
        {
             g_mouse_but[1]=false;
        }
        break;

        case WM_MOUSEWHEEL:
        {
            if(HIWORD(wParam)>5000) {g_mouse_but[2]=true;}
            if(HIWORD(wParam)>100&&HIWORD(wParam)<5000) {g_mouse_but[3]=true;}
        }
        break;

		case WM_KEYDOWN:
		{
			g_keys[wParam]=true;
			cout<<"Key pressed: "<<(int)wParam<<endl; //temp

			if((int)wParam==27) PostQuitMessage(0);//ESC
		}
		break;

		case WM_KEYUP:
		{
			g_keys[wParam]=false;
		}
		break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;
    *hDC = GetDC(hwnd);
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);

    //set 2D mode
    glClearColor(0.0,0.0,0.0,0.0);  //Set the cleared screen colour to black
    glViewport(0,0,g_wind_size[0],g_wind_size[1]);   //This sets up the viewport so that the coordinates (0, 0) are at the top left of the window

    //Set up the orthographic projection so that coordinates (0, 0) are in the top left
    //and the minimum and maximum depth is -10 and 10. To enable depth just put in
    //glEnable(GL_DEPTH_TEST)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,g_wind_size[0],g_wind_size[1],0,-1,1);

    //Back to the modelview so we can draw stuff
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //Enable antialiasing
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

