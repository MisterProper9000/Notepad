#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "menu.h"
#include "text_functions.h"
//#include "dialog.h"


/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
WCHAR szClassName[ ] = L"TextEdit";
#define WND_CLASS_NAME "1"

INT WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   CHAR *CmdLine,
                   INT ShowCmd)
{
    WNDCLASS wc;
    HWND hWnd;
    MSG msg;

    wc.style = CS_DBLCLKS | CS_CLASSDC | CS_HREDRAW | CS_VREDRAW;

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

    wc.hInstance = hInstance;
    wc.lpfnWndProc = WindowProcedure;
    wc.lpszClassName = WND_CLASS_NAME;
    wc.lpszMenuName = MAKEINTRESOURCE(MYMENU);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    RegisterClass(&wc);

    hWnd = CreateWindowA("1", "1", WS_OVERLAPPEDWINDOW , 0, 0, 700, 700, NULL, NULL, hInstance, CmdLine);
    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);

    while (GetMessage(&msg, NULL, 0, 0) != 0 )
    {
        //if (!IsWindow(getDialogHWND()) || !IsDialogMessage(getDialogHWND(), &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return msg.wParam;
}

/*    IN:
        hMemDC - DC for Rectangle
      OUT: -

draw big white Rectangle in window */

void FillWhiteBack(const HDC hMemDC)
{
    static HBRUSH hBrush;
    static HPEN hPen;

    hPen = CreatePen(0, 1, RGB(255, 255, 255));
    hBrush = CreateSolidBrush(RGB(255, 255, 255));
    SelectObject(hMemDC, hPen);
    SelectObject(hMemDC, hBrush);
    Rectangle(hMemDC, 0, 0, maxWidth, maxHeight);
    DeleteObject(hBrush);
    DeleteObject(hPen);
}

/*  IN:
        hwnd - dialog window descriptor
        message - message from dialog window
        wParam - word parameters
        lParam - long parameters
    OUT: -  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static winParam windowInfo;
    static textParams text;
    static caret_t caret;

    static HBITMAP hBm;
    HDC hdc = GetDC(hwnd);
    HFONT font;
    HMENU hMenu;

    windowInfo.modeChecked = NORMAL_MODE;
    switch (message)
    {
        case WM_CREATE:

            windowInfo.hMemDC = CreateCompatibleDC(hdc);
            hBm = CreateCompatibleBitmap(hdc, maxWidth, maxHeight);
            SelectObject(windowInfo.hMemDC, hBm);

            ReleaseDC(hwnd, hdc);

            font = CreateFont(18,          // height 18pt
                8,                         // symbol width
                0,                         // angle 0.1
                0,                         // angle 0.01
                FW_BOLD,                   // thickness: 0 to 1000
                0,                         // italic
                0,                         // underline
                0,                         // strikeout
                DEFAULT_CHARSET,           // character set identifier
                OUT_DEVICE_PRECIS,         // output precision
                CLIP_DEFAULT_PRECIS,       // clipping precision
                DEFAULT_QUALITY,           // output quality
                DEFAULT_PITCH,             // pitch and family
                "Courier");
            SelectObject(windowInfo.hMemDC, font);

                    CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
                    FILE *pFile = fopen((char *)cs->lpCreateParams, "rb");
                    if(pFile)
                    {
                        InitText(&text, &caret, pFile);
                    }

        case WM_COMMAND:
            hMenu = GetMenu(hwnd);
            switch (wParam)
            {
            case OPEN: //open file
            {
                OPENFILENAME ofn;
                char nameFile[100] = { 0 };

                ZeroMemory(&ofn, sizeof(ofn));
                static CHAR szFilter[] = "Text Files(*.txt)\0*.txt\0All Files(*.*)\0*.*\0\0";
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = "\0";
                ofn.nMaxFile = 100;
                ofn.lpstrFilter = (LPCSTR)szFilter;
                ofn.nFilterIndex = 1;
                ofn.lpstrTitle = TEXT("Open");
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

                ofn.lpstrFile = nameFile;

                if (GetOpenFileName(&ofn))
                {

                    free(text.pDefaultStartLineIdx);
                    free(text.pWrapStartLineIdx);
                    free(text.String);
                    //free(&text);
                    FILE *pFile;
                    pFile = fopen(ofn.lpstrFile, "rb");

                    InitText(&text, &caret, pFile);

                    fclose(pFile);
                }
                SendMessage(hwnd, WM_SIZE, 0, 0L);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

            case NORMAL_MODE: //standart view
            {
                CheckMenuItem(hMenu, windowInfo.modeChecked, MF_UNCHECKED);
                windowInfo.modeChecked = LOWORD(wParam);
                CheckMenuItem(hMenu, windowInfo.modeChecked, MF_CHECKED);

                ResizeText(&text, &caret, text.maxLineLen * text.letterWidth);
                text.wrap = 0;

                SendMessage(hwnd, WM_SIZE, 0, 0L);

                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

            case WRAP_MODE: // format view
            {
                CheckMenuItem(hMenu, windowInfo.modeChecked, MF_UNCHECKED);
                windowInfo.modeChecked = LOWORD(wParam);
                CheckMenuItem(hMenu, windowInfo.modeChecked, MF_CHECKED);

                text.wrap = 1;
                text.textOffset = 0;

                SendMessage(hwnd, WM_SIZE, 0, 0L);

                InvalidateRect(hwnd, NULL, TRUE);

            }
            break;

            case EXIT:
            {
                PostQuitMessage(0);
            }
            }
            break;

        case WM_ERASEBKGND:
            break;

        case WM_PAINT:
        {
            HDC hdc = BeginPaint(hwnd, &windowInfo.ps);
            GetClientRect(hwnd, &windowInfo.rect);

            FillWhiteBack(windowInfo.hMemDC);
            //PrintText(&text, windowInfo.hMemDC, windowInfo.rect.bottom);

            int i;

            for (i = 0; text.letterHeight * i < windowInfo.rect.bottom && text.workLine + i < text.wrapLinesSize; ++i)
            {
                TextOutA(windowInfo.hMemDC,
                text.textOffset * text.letterWidth,
                text.letterHeight * i,
                text.String + text.pWrapStartLineIdx[text.workLine + i],
                (text.pWrapStartLineIdx[text.workLine + i + 1] - text.pWrapStartLineIdx[text.workLine + i]));
            }

            BitBlt(hdc, 0, 0, maxWidth, maxHeight, windowInfo.hMemDC, 0, 0, SRCCOPY);
            EndPaint(hwnd, &windowInfo.ps);
            return 0;
        }

        case WM_SIZE :

            ResizeText(&text, &caret, windowInfo.rect.right);

            MoveCaretIntoView(&text, &caret, &windowInfo, hwnd);
            SetCaretPos((caret.xPos + text.textOffset) * text.letterWidth, (caret.yPos - text.workLine) * text.letterHeight);

            break;

        case WM_SETFOCUS:
            CreateCaret(hwnd, (HBITMAP)0, 2, 16);
            SetCaretPos((caret.xPos + text.textOffset) * text.letterWidth, (caret.yPos - text.workLine) * text.letterHeight);
            ShowCaret(hwnd);
            break;

        case WM_KILLFOCUS:
            HideCaret(hwnd);
            DestroyCaret();
            break;

        case WM_KEYDOWN:
            switch (wParam)
            {

            case VK_UP:
                MoveCaretUp(&windowInfo, &caret, &text, hwnd);
                break;
            case VK_DOWN:
                MoveCaretDown(&windowInfo, &caret, &text, hwnd);
                break;
            case VK_LEFT:
                MoveCaretLeft(&windowInfo, &caret, &text, hwnd);
                break;
            case VK_RIGHT:
                MoveCaretRight(&windowInfo, &caret, &text, hwnd);
                break;


            }
            //SetHorizontalScroll(&text, &windowInfo, -1, &wParam, hwnd);
            InvalidateRect(hwnd, NULL, FALSE);
            break;


        case WM_DESTROY:
            if(text.String){
                free(text.pDefaultStartLineIdx);
                free(text.pWrapStartLineIdx);
                free(text.String);
                free(&text);
                free(&caret);
            }
            DeleteDC(windowInfo.hMemDC);
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;

        default: /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }
    return 0;
}
