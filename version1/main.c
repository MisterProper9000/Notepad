
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <String.h>

#include "dataStruct.h"
#include "menu.h"

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
void AddMenus(HWND, HMENU, HMENU);
/*  Make the class name into a global variable  */


char szClassName[ ] = "CodeBlocksWindowsApp";



int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS | CS_CLASSDC | CS_HREDRAW | CS_VREDRAW;             /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;



    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           "Code::Blocks Template Windows App",       /* Title Text */
           WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           550,                 /* The programs width */
           400,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           lpszArgument                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}



/*  This function is called by the Windows function DispatchMessage()  */

typedef struct {
    char* str;
    int numStr;
    int* strEnds;
    int viewPosition;
};

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    HMENU hMenu, hMenuPopup;
    TEXTMETRIC tm;
    SCROLLINFO si;

    static HFONT hfont;
    static int cxClient;
    static int cyClient;
    static int xPos;
    static int yPos;

    static BOOLEAN isThereAnyErorrs = FALSE;

    static textParams tp;
    static wndParams wp;

    int i, numSymbols;

    switch (message)                  /* handle the messages */
    {
    case WM_CREATE:
        hdc = GetDC(hwnd);

        hfont = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);
        SelectObject(hdc, hfont);
        GetTextMetrics(hdc, &tm);

        wp.cxSymbol = tm.tmAveCharWidth;
        wp.cySymbol = tm.tmExternalLeading +tm.tmHeight;
        wp.mode = NORMAL_MODE;
        wp.curX = 0;
        wp.curY = 0;
        wp.startLineIdx = NULL;
        tp.startLineIdx = NULL;
        tp.String = NULL;

        ReleaseDC(hwnd, hdc);

        FileRead((char*)(((CREATESTRUCT*)lParam)->lpCreateParams), &tp, &isThereAnyErorrs);
        if(isThereAnyErorrs) {
            MessageBox( NULL, "Some error happens", "ERROR", MB_OK | MB_ICONERROR );
            PostMessage(hwnd, WM_DESTROY, 0, 0);
            break;
        }

        BackToNormalMetrics(&tp, &wp, &isThereAnyErorrs);
        if(isThereAnyErorrs) {
            MessageBox( NULL, "Some error happens", "ERROR", MB_OK | MB_ICONERROR );
            PostMessage(hwnd, WM_DESTROY, 0, 0);
            break;
        }

        SetClassLong(hwnd,GCL_HBRBACKGROUND, (LONG)CreateSolidBrush(RGB(255,255,255)));

        AddMenus(hwnd,hMenu,hMenuPopup);

        break;
    case WM_SIZE:
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);

        if(wp.mode == WRAP_MODE) {
            ReCalculate(cxClient / wp.cxSymbol, &tp, &wp, &isThereAnyErorrs);
            if(isThereAnyErorrs) {
                MessageBox( NULL, "Some error happens", "ERROR", MB_OK | MB_ICONERROR );
                PostMessage(hwnd, WM_DESTROY, 0, 0);
                break;
            }
        }
        SetScrollbars(hwnd, &si, cxClient, cyClient, &wp, &tp);
        break;
    case WM_VSCROLL:
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        GetScrollInfo(hwnd, SB_VERT, &si);
        yPos = si.nPos;
        switch(LOWORD(wParam))
        {
        case SB_LINEUP :
            si.nPos -= 1;
            break;
        case SB_LINEDOWN :
            si.nPos += 1;
            break;
        case SB_PAGEUP :
            si.nPos -= si.nPage;
            break;
        case SB_PAGEDOWN :
            si.nPos += si.nPage;
            break;
        case SB_THUMBTRACK :
            si.nPos = si.nTrackPos;
            break;
        default :
            break;
        }
        wp.curY = si.nPos;
        if(wp.mode == NORMAL_MODE) {
            tp.curLine = si.nPos;
        }
        else if(wp.mode == WRAP_MODE) {
            if(yPos - si.nPos > 0) {
                ChangeCurrentLine(&tp, &wp, SCROLL_DOWN);
            }
            else if(yPos - si.nPos < 0) {
                ChangeCurrentLine(&tp, &wp, SCROLL_UP);
            }
        }
        si.fMask = SIF_POS;
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
        GetScrollInfo(hwnd, SB_VERT, &si);

        if(si.nPos != yPos) {
            ScrollWindow(hwnd, 0, wp.cySymbol * (yPos - si.nPos), NULL, NULL);
            UpdateWindow(hwnd);
        }
        break;
    case WM_HSCROLL:
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        GetScrollInfo(hwnd, SB_HORZ, &si);
        xPos = si.nPos;
        switch(LOWORD(wParam))
        {
        case SB_LINELEFT:
            si.nPos -= 1;
            break;
        case SB_LINERIGHT:
            si.nPos += 1;
            break;
        case SB_PAGELEFT:
            si.nPos -= si.nPage;
            break;
        case SB_PAGERIGHT:
            si.nPos += si.nPage;
            break;
        case SB_THUMBTRACK:
            si.nPos = si.nTrackPos;
            break;
        default :
            break;
        }
        wp.curX = si.nPos;
        si.fMask = SIF_POS;
        SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
        GetScrollInfo (hwnd, SB_HORZ, &si);

        if(si.nPos != xPos) {
            ScrollWindow(hwnd,wp.cxSymbol * (xPos - si.nPos), 0, NULL, NULL);
            UpdateWindow(hwnd);
        }
        break;
    case WM_COMMAND:
        hMenu = GetMenu(hwnd);
        hMenuPopup = GetSubMenu(hMenu,1);
        switch(wParam)
        {
        case MENU_OPEN:
            NewFileRead(hwnd, &tp, &isThereAnyErorrs);
            if(isThereAnyErorrs) {
                MessageBox( NULL, "Some error happens", "ERROR", MB_OK | MB_ICONERROR );
                PostMessage(hwnd, WM_DESTROY, 0, 0);
                break;
            }
            wp.curX = 0;
            wp.curY = 0;
            si.nPos = 0;
            yPos = 0;
            if(wp.mode == NORMAL_MODE) {
                BackToNormalMetrics(&tp, &wp, &isThereAnyErorrs);
                if(isThereAnyErorrs) {
                    MessageBox( NULL, "Some error happens", "ERROR", MB_OK | MB_ICONERROR );
                    PostMessage(hwnd, WM_DESTROY, 0, 0);
                    break;
                }
            }
            else if(wp.mode == WRAP_MODE) {
                ReCalculate(cxClient / wp.cxSymbol, &tp, &wp, &isThereAnyErorrs);
                if(isThereAnyErorrs) {
                    MessageBox( NULL, "Some error happens", "ERROR", MB_OK | MB_ICONERROR );
                    PostMessage(hwnd, WM_DESTROY, 0, 0);
                    break;
                }
            }
            ScrollWindow(hwnd, 0, cyClient, NULL, NULL);
            SetScrollbars(hwnd, &si, cxClient, cyClient, &wp, &tp);
            SetScrollPos(hwnd, SB_HORZ, 0, TRUE);
            SetScrollPos(hwnd, SB_VERT, 0, TRUE);
            break;
        case MENU_MOD_NORMAL:
            wp.mode = NORMAL_MODE;
            BackToNormalMetrics(&tp, &wp, &isThereAnyErorrs);
            if(isThereAnyErorrs) {
                MessageBox( NULL, "Some error happens", "ERROR", MB_OK | MB_ICONERROR );
                PostMessage(hwnd, WM_DESTROY, 0, 0);
                break;
            }
            ScrollWindow(hwnd, 0, cyClient, NULL, NULL);
            SetScrollbars(hwnd, &si, cxClient, cyClient, &wp, &tp);
            EnableMenuItem(hMenuPopup, MENU_MOD_NORMAL, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(hMenuPopup, MENU_MOD_WRAPED, MF_BYCOMMAND | MF_ENABLED);
            break;
        case MENU_MOD_WRAPED:
            wp.mode = WRAP_MODE;
            ReCalculate(cxClient / wp.cxSymbol, &tp, &wp, &isThereAnyErorrs);
            if(isThereAnyErorrs) {
                MessageBox( NULL, "Some error happens", "ERROR", MB_OK | MB_ICONERROR );
                PostMessage(hwnd, WM_DESTROY, 0, 0);
                break;
            }
            ScrollWindow(hwnd, 0, cyClient, NULL, NULL);
            SetScrollbars(hwnd, &si, cxClient, cyClient, &wp, &tp);
            EnableMenuItem(hMenuPopup, MENU_MOD_NORMAL, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenuPopup, MENU_MOD_WRAPED, MF_BYCOMMAND | MF_GRAYED);
            break;
        case MENU_CLOSE:
            SendMessage(hwnd,WM_CLOSE,0,0);
            break;
        default:
            break;
        }
        break;
    case WM_KEYDOWN:
        switch(wParam) {
        case VK_PRIOR:
            PostMessage(hwnd, WM_VSCROLL, SB_PAGEUP, 0);
            break;
        case VK_NEXT:
            PostMessage(hwnd, WM_VSCROLL, SB_PAGEDOWN, 0);
            break;
        case VK_LEFT:
            PostMessage(hwnd, WM_HSCROLL, SB_LINELEFT, 0);
            break;
        case VK_RIGHT:
            PostMessage(hwnd, WM_HSCROLL, SB_LINERIGHT, 0);
            break;
        case VK_UP:
            PostMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0);
            break;
        case VK_DOWN:
            PostMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0);
            break;
        case VK_ESCAPE:
            SendMessage(hwnd,WM_CLOSE,0,0);
            break;
        default:
            break;
        }
        break;
    case WM_PAINT:
        i = wp.curY;
        hdc = BeginPaint(hwnd, &ps);

        while((i - wp.curY) < wp.numWndLines && i < wp.numLines){
            if((numSymbols = wp.startLineIdx[i+1] - wp.startLineIdx[i] - wp.curX) < 0)
                numSymbols = 0;
            TextOut(hdc, 0, wp.cySymbol * (i - wp.curY), tp.String + wp.startLineIdx[i] + wp.curX, numSymbols);
            i++;
        }

        EndPaint(hwnd,&ps);
        break;
    case WM_DESTROY:
        free(tp.String);
        free(tp.startLineIdx);
        free(wp.startLineIdx);
        DeleteObject(hfont);
        PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
        break;
    default:                      /* for the unhandled msgs */
        return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}


void AddMenus(HWND hwnd, HMENU myMenu, HMENU myMenuPopup)
{
    myMenu = CreateMenu();
    myMenuPopup = CreateMenu();

    AppendMenu(myMenuPopup, MF_STRING, MENU_OPEN, "&Open");
    AppendMenu(myMenuPopup, MF_STRING, MENU_CLOSE, "&Close");

    AppendMenu(myMenu, MF_POPUP, (UINT_PTR)myMenuPopup, "&File");

    myMenuPopup = CreateMenu();

    AppendMenu(myMenuPopup, MF_STRING | MF_GRAYED, MENU_MOD_NORMAL, "&Normal");
    AppendMenu(myMenuPopup, MF_STRING, MENU_MOD_WRAPED, "&Wraped");

    AppendMenu(myMenu, MF_POPUP, (UINT_PTR)myMenuPopup, "&Mods");

    SetMenu(hwnd, myMenu);
}
