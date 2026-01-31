#define _WIN32_WINNT 0x0600  // 指定Windows Vista或更高版本
#define WINVER 0x0600
#include <iostream>
#include <easyx.h>
#include <windows.h>
using namespace std;
int main()
{
    // 设置DPI感知 - 关键！确保获取正确的逻辑分辨率
    SetProcessDPIAware();

    // 获取实际的桌面工作区（排除任务栏）
    int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    int screenX = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int screenY = GetSystemMetrics(SM_YVIRTUALSCREEN);

    cout << "桌面逻辑宽度: " << screenWidth << endl;
    cout << "桌面逻辑高度: " << screenHeight << endl;
    cout << "桌面位置: (" << screenX << "," << screenY << ")" << endl;

    // 创建窗口，建议按实际分辨率或等比例缩小
    int windowWidth = 1600;
    int windowHeight = 900; // 16:9比例

    // 如果屏幕分辨率小于窗口，适应屏幕
    if (screenWidth < windowWidth) windowWidth = screenWidth;
    if (screenHeight < windowHeight) windowHeight = screenHeight;

    initgraph(windowWidth, windowHeight);
    setbkcolor(WHITE);
    cleardevice();

    // 创建桌面截图缓冲区（使用实际物理分辨率保证清晰度）
    HWND hDeskWnd = GetDesktopWindow();
    HDC hDeskDc = GetWindowDC(hDeskWnd);

    // 获取物理分辨率（用于BitBlt）
    HDC hScreenDC = CreateDC(L"DISPLAY", NULL, NULL, NULL);
    int physWidth = GetDeviceCaps(hScreenDC, HORZRES);
    int physHeight = GetDeviceCaps(hScreenDC, VERTRES);
    DeleteDC(hScreenDC);

    // 创建图像缓冲区（按物理分辨率，保证截图清晰）
    IMAGE screenImage(physWidth, physHeight);
    HDC hImageDc = GetImageHDC(&screenImage);

    // 计算缩放比例（让桌面完整显示在窗口内）
    double scaleX = (double)windowWidth / physWidth;
    double scaleY = (double)windowHeight / physHeight;
    double scale = min(scaleX, scaleY); // 保持比例，取小的那个

    int displayWidth = (int)(physWidth * scale);
    int displayHeight = (int)(physHeight * scale);
    int offsetX = (windowWidth - displayWidth) / 2;
    int offsetY = (windowHeight - displayHeight) / 2;

    cout << "显示尺寸: " << displayWidth << "x" << displayHeight << endl;
    cout << "缩放比例: " << (scale * 100) << "%" << endl;
    cout << "按 ESC 退出程序" << endl;

    // 主循环
    while (true)
    {
        // 捕获桌面（从物理屏幕复制到内存）
        BitBlt(hImageDc, 0, 0, physWidth, physHeight, hDeskDc, 0, 0, SRCCOPY);

        // 清空画布
        cleardevice();

        // 等比例缩放显示（使用StretchBlt或EasyX的putimage）
        // 方法1：使用Windows API StretchBlt进行高质量缩放
        SetStretchBltMode(GetImageHDC(), HALFTONE);
        StretchBlt(
            GetImageHDC(),
            offsetX, offsetY, displayWidth, displayHeight,  // 目标位置和大小
            hImageDc, 0, 0, physWidth, physHeight,          // 源图像
            SRCCOPY
        );

        // 方法2：如果希望按原像素显示（1:1），使用putimage，但会超出屏幕
        // putimage(0, 0, &screenImage);

        // 显示帧率信息（可选）
        settextcolor(BLACK);
        outtextxy(10, 10, _T("监控中... 按ESC退出"));

        // 刷新显示
        FlushBatchDraw();

        // 简单的退出检测（实际应用中应使用消息循环）
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;

        Sleep(10); // 限制20fps，减少CPU占用
    }

    // 释放资源
    ReleaseDC(hDeskWnd, hDeskDc);
    closegraph();
    return 0;
}