#ifndef APPLICATION_H
#define APPLICATION_H
#include <windows.h>

class Application
{
    public:
        Application(HINSTANCE hInst = NULL);
        ~Application();

        HRESULT Initialize();

        void RunMessageLoop();

    private:
        BOOL OnCreate(HWND, LPCREATESTRUCT);
        VOID OnCommand(HWND, INT, HWND, UINT);
        VOID OnTimer(HWND, UINT);

        static LRESULT CALLBACK WndProc(
            HWND hWnd,
            UINT message,
            WPARAM wParam,
            LPARAM lParam
            );

    private:
        HINSTANCE hInstance;
        HWND hMainWindow;
        HWND hPlot1, hPlot2, hPlot3, hPlot4;
};

#endif // APPLICATION_H
