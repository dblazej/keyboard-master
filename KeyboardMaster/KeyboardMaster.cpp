// KeyboardMaster.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "KeyboardMaster.h"

#define MAX_LOADSTRING 100
#define MAX_SQUARES    100
#define SQUARE_SIZE    25

// Global Variables:
HINSTANCE hInst;                      // current instance
BOOL gameState = TRUE;                // TRUE - active, FALSE - pause
int wrongKeys = 0, missedKeys = 0;
HBRUSH blackBackground = CreateSolidBrush(RGB(0, 0, 0));

// Main window related variables:
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
HWND hWndMain = NULL;
RECT mainWindowSize;
int mainWindowHeight, mainWindowLength;

// Squares related variables:
HWND hWndSquare[MAX_SQUARES + 1];
int squareCount = 0, squareSpeed[MAX_SQUARES + 1];
std::map<HWND, WCHAR> squareLetter;

// Background related variables:
int backgroundMode = BACKGROUND_DEFAULT;
HBRUSH backgroundColor = DEFAULT_BKG_COLOR;
WCHAR bitmapFilePath[MAX_PATH];

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WndProcSquare(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL                ShowSquareWindow();
void                CloseAllSquareWindows();
void                CloseSquareWindow(int id);
void                MoveSquaresDown();
void                CloseMostBottomWindow();
int                 RandomXPos();
void                ResetGame();
void                PauseGame(int forceState = 0);
void                CloseMostBottomSquareWithLetter(TCHAR letter);
void                SetWindowTitle();
HBRUSH              ShowColorPickerDialog(HWND hWnd);
void                SetBackgroundColor(HWND hWnd, HBRUSH pickedcolor);
void                ShowOpenFileDialog(HWND hWnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_KEYBOARDMASTER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KEYBOARDMASTER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KEYBOARDMASTER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_INACTIVECAPTION + 1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_KEYBOARDMASTER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    WNDCLASSEXW wcexSquare;

    wcexSquare.cbSize = sizeof(WNDCLASSEX);

    wcexSquare.style = CS_HREDRAW | CS_VREDRAW;
    wcexSquare.lpfnWndProc = WndProcSquare;
    wcexSquare.cbClsExtra = 0;
    wcexSquare.cbWndExtra = 0;
    wcexSquare.hInstance = hInstance;
    wcexSquare.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KEYBOARDMASTER));
    wcexSquare.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcexSquare.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    wcexSquare.lpszMenuName = MAKEINTRESOURCEW(IDC_KEYBOARDMASTER);
    wcexSquare.lpszClassName = L"square";
    wcexSquare.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));


    RegisterClassExW(&wcex);
    return RegisterClassExW(&wcexSquare);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
   srand(time(NULL));

   // Check screen dimensions
   const int ScreenX = GetSystemMetrics(SM_CXSCREEN);
   const int ScreenY = GetSystemMetrics(SM_CYSCREEN);

   // Create main window
   hWndMain = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME | WS_VISIBLE | WS_EX_TOPMOST,
       ScreenX / 4, ScreenY / 4, ScreenX / 2, ScreenY / 2, nullptr, nullptr, hInstance, nullptr);

   // Save dimensions of the main window
   GetWindowRect(hWndMain, &mainWindowSize);
   mainWindowHeight = ScreenX / 2;
   mainWindowLength = ScreenY / 2;

   // Keep the main window on top
   SetWindowPos(hWndMain, HWND_TOPMOST, NULL, NULL, NULL, NULL, SWP_NOMOVE | SWP_NOSIZE);

   if (!hWndMain)
   {
      return FALSE;
   }

   ShowWindow(hWndMain, nCmdShow);
   UpdateWindow(hWndMain);
   SetWindowTitle();

   // Prepare square tiles
   for (int i = 0; i <= MAX_SQUARES; i++)
   {
       hWndSquare[i] = NULL;
       squareSpeed[i] = 0;
   }
   ShowSquareWindow();

   // Start game
   PauseGame(FORCE_RESUME);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            // Parse main menu options
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_NEWGAME:
                ResetGame();
                break;
            case IDM_PAUSEGAME:
                PauseGame();
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            // Parse background options
            case IDM_BKG_BITMAP:
                {
                    backgroundMode = BACKGROUND_BITMAP;
                    ShowOpenFileDialog(hWnd);
                    RECT rc;
                    GetWindowRect(hWnd, &rc);
                    InvalidateRect(NULL, &rc, TRUE);
                }
                break;
            case IDM_BKG_TILE:
                {
                    backgroundMode = BACKGROUND_TILE;
                    HBITMAP hBitMap = (HBITMAP)LoadImage(NULL, bitmapFilePath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
                    if (hBitMap != NULL) backgroundColor = CreatePatternBrush(hBitMap);
                    RECT rc;
                    GetWindowRect(hWnd, &rc);
                    InvalidateRect(NULL, &rc, TRUE);
                }
                break;
            case IDM_BKG_STREACH:
                {
                    backgroundMode = BACKGROUND_STREACH;
                    HBITMAP hBitMap = (HBITMAP)LoadImage(NULL, bitmapFilePath, IMAGE_BITMAP, mainWindowHeight, mainWindowLength, LR_LOADFROMFILE);
                    if (hBitMap != NULL) backgroundColor = CreatePatternBrush(hBitMap);
                    RECT rc;
                    GetWindowRect(hWnd, &rc);
                    InvalidateRect(NULL, &rc, TRUE);
                }
                break;
            case IDM_BKG_COLOR:
                {
                    backgroundMode = BACKGROUND_COLOR;
                    HBRUSH pickedColor = ShowColorPickerDialog(hWndMain);
                    SetBackgroundColor(hWndMain, pickedColor);
                }
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // Draw background
            FillRect(hdc, &ps.rcPaint, backgroundColor);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_CREATE:
        // Create a timer
        SetTimer(hWnd, 7, (rand() % 700) + 300, NULL);
        SetTimer(hWnd, 8, 100, NULL);
        break;
    case WM_TIMER:
        { // Do something in response for the WM_TIMER message
            if (wParam == 7) // check timer id
            {
                KillTimer(hWnd, 7);
                if(squareCount < MAX_SQUARES && gameState) ShowSquareWindow();
                SetTimer(hWnd, 7, (rand() % 700) + 300, NULL);
            }
            else if (wParam == 8)
            {
                if(gameState) MoveSquaresDown();
            }
        }
        break;
    case WM_KEYDOWN:
        {
            //CloseMostBottomSquareWithLetter(MapVirtualKeyEx(wParam, MAPVK_VK_TO_CHAR, GetKeyboardLayout(0)));
            if ('A' <= wParam && wParam <= 'Z')
            {
                CloseMostBottomSquareWithLetter(wParam - 'A' + 'a');
            }
            else
            {
                wrongKeys++;
                SetWindowTitle();
            }
        }
        break;
    case WM_ACTIVATE:
        { // Pausing game if main window is not active
            if (wParam == WA_INACTIVE) PauseGame(FORCE_PAUSE);
            else PauseGame(FORCE_RESUME);
        }
        break;
    case WM_RBUTTONDOWN:
    { // Source: https://cpp.hotexamples.com/examples/-/-/CreatePopupMenu/cpp-createpopupmenu-function-examples.html
        // Prepare to open popup menu
        HMENU popupMenu;
        POINT mouseClick;
        mouseClick.x = LOWORD(lParam);
        mouseClick.y = HIWORD(lParam);

        // Create popup menu
        popupMenu = CreatePopupMenu();
        ClientToScreen(hWnd, &mouseClick);

        // Add and parse options
        AppendMenu(popupMenu, MF_STRING, IDM_BKG_BITMAP, L"Bitmap...\tCtrl+B");
        AppendMenu(popupMenu, MF_STRING | (backgroundMode != BACKGROUND_DEFAULT ? MF_ENABLED : MF_GRAYED) | (backgroundMode == BACKGROUND_TILE ? MF_CHECKED : MF_UNCHECKED), IDM_BKG_TILE, L"Tile\tCtrl+T");
        AppendMenu(popupMenu, MF_STRING | (backgroundMode != BACKGROUND_DEFAULT ? MF_ENABLED : MF_GRAYED) | (backgroundMode == BACKGROUND_STREACH ? MF_CHECKED : MF_UNCHECKED), IDM_BKG_STREACH, L"Streach\tCtrl+S");
        AppendMenu(popupMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(popupMenu, MF_STRING, IDM_BKG_COLOR, L"Color...\tCtrl+C");

        // Close popup menu
        TrackPopupMenu(popupMenu, TPM_RIGHTBUTTON, mouseClick.x, mouseClick.y, 0, hWnd, NULL);
        DestroyMenu(popupMenu);
        break;
    }
    case WM_DESTROY:
        CloseAllSquareWindows();
        DestroyWindow(hWndMain);
        PostQuitMessage(0);
        DeleteObject(blackBackground);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK WndProcSquare(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        {
        // Paint letter inside each square
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            TCHAR s[] = L"W";
            s[0] = squareLetter[hWnd];
            RECT rc;
            GetClientRect(hWnd, &rc);
            SetBkMode(hdc, RGB(0, 0, 0));
            SetTextColor(hdc, RGB(255, 255, 255));
            FillRect(hdc, &ps.rcPaint, blackBackground);
            DrawText(hdc, s, (int)_tcslen(s), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            EndPaint(hWnd, &ps);
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

BOOL ShowSquareWindow()
{
    if (squareCount < MAX_SQUARES)
    {
        // Create square tile
        squareCount++;
        hWndSquare[squareCount] = CreateWindowW(L"square", szTitle, WS_CHILDWINDOW | WS_CLIPSIBLINGS,
            RandomXPos(), 0, SQUARE_SIZE, SQUARE_SIZE, hWndMain, nullptr, hInst, nullptr);
        squareSpeed[squareCount] = rand() % 3 + 3;

        // Remove decorations and styles
        SetWindowLong(hWndSquare[squareCount], GWL_STYLE, 0);
        SetMenu(hWndSquare[squareCount], NULL);

        // Prepare letter
        squareLetter[hWndSquare[squareCount]] = (rand() % 26 + L'a');

        if (!hWndSquare[squareCount])
        {
            return FALSE;
        }

        ShowWindow(hWndSquare[squareCount], SW_SHOWNOACTIVATE);
        return TRUE;
    }
    else return FALSE;
}

void CloseSquareWindow(int id)
{
    DestroyWindow(hWndSquare[id]);
    for (int i = id; i <= squareCount - 1; i++)
    {
        hWndSquare[i] = hWndSquare[i + 1];
        squareSpeed[i] = squareSpeed[i + 1];
    }
    squareCount--;
}

void CloseAllSquareWindows()
{
    for (int i = 1; i <= squareCount; i++)
    {
        DestroyWindow(hWndSquare[0]);
        hWndSquare[i] = 0;
        squareSpeed[i] = 0;
    }
    squareCount = 0;
}

void CloseMostBottomWindow()
{
    RECT rc;
    int id = 1, yMaxPos = 0;
    for (int i = 1; i <= squareCount; i++)
    {
        GetClientRect(hWndSquare[i], &rc);
        if (yMaxPos < rc.bottom)
        {
            yMaxPos = rc.bottom;
            id = 1;
        }
    }
    CloseSquareWindow(id);
}

void CloseMostBottomSquareWithLetter(TCHAR letter)
{
    RECT rc;
    int id = -1, yMaxPos = 0;
    for (int i = 1; i <= squareCount; i++)
    {
        if (squareLetter[hWndSquare[i]] != letter) continue;
        GetClientRect(hWndSquare[i], &rc);
        if (yMaxPos < rc.bottom)
        {
            yMaxPos = rc.bottom;
            id = 1;
        }
    }
    if (id != -1) CloseSquareWindow(id);
    else
    {
        wrongKeys++;
        SetWindowTitle();
    }
}

void MoveSquaresDown()
{
    RECT rc, rcMain;
    GetClientRect(hWndMain, &rcMain);
    for (int i = 1; i <= squareCount; i++)
    {
        GetWindowRect(hWndSquare[i], &rc);
        MapWindowPoints(HWND_DESKTOP, hWndMain, (LPPOINT)&rc, 2);
        if (rc.bottom + squareSpeed[i] >= rcMain.bottom)
        {
           CloseSquareWindow(i);
           missedKeys++;
           SetWindowTitle();
        }
        else
        {
            MoveWindow(hWndSquare[i], rc.left, rc.top + squareSpeed[i], SQUARE_SIZE, SQUARE_SIZE, TRUE);
        }
    }
}

int RandomXPos()
{
    RECT rcMain;
    GetClientRect(hWndMain, &rcMain);
    return rand() % (rcMain.right - rcMain.left - SQUARE_SIZE + 1);
}

void ResetGame()
{
    gameState = FALSE;
    for (int id = squareCount; id >= 1; id--)
    {
        CloseSquareWindow(id);
    }
    PauseGame(FORCE_RESUME);
}

void PauseGame(int forceState)
{
    if (forceState != 0)
    {
        if (forceState == FORCE_PAUSE)
        {
            gameState = FALSE;
            CheckMenuItem(GetMenu(hWndMain), IDM_PAUSEGAME, MF_BYCOMMAND | MF_CHECKED);
            return;
        }
        else if (forceState == FORCE_RESUME)
        {
            gameState = TRUE;
            CheckMenuItem(GetMenu(hWndMain), IDM_PAUSEGAME, MF_BYCOMMAND | MF_UNCHECKED);
            return;
        }
    }
    else
    {
        gameState = !gameState;
        if(gameState) CheckMenuItem(GetMenu(hWndMain), IDM_PAUSEGAME, MF_BYCOMMAND | MF_UNCHECKED);
        else CheckMenuItem(GetMenu(hWndMain), IDM_PAUSEGAME, MF_BYCOMMAND | MF_CHECKED);
        return;
    }

}

void SetWindowTitle()
{
    WCHAR buff[MAX_LOADSTRING];
    swprintf_s(buff, MAX_LOADSTRING, L"Keyboard Master: WinAPI_2021, Missed: %d, Wrong keys: %d", missedKeys, wrongKeys);
    SetWindowText(hWndMain, buff);
}

HBRUSH ShowColorPickerDialog(HWND hWnd)
// Source: https://docs.microsoft.com/pl-pl/windows/win32/dlgbox/using-common-dialog-boxes?redirectedfrom=MSDN
{
    CHOOSECOLOR cc;                     // common dialog box structure 
    static COLORREF acrCustClr[16];     // array of custom colors 
    HBRUSH hbrush = backgroundColor;    // brush handle
    static DWORD rgbCurrent;            // initial color selection

    // Initialize CHOOSECOLOR 
    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hWnd;
    cc.lpCustColors = (LPDWORD) acrCustClr;
    cc.rgbResult = rgbCurrent;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (ChooseColor(&cc) == TRUE)
    {
        hbrush = CreateSolidBrush(cc.rgbResult);
        rgbCurrent = cc.rgbResult;
    }

    return hbrush;
}

void SetBackgroundColor(HWND hWnd, HBRUSH pickedcolor)
{
    RECT rc;
    backgroundColor = pickedcolor;
    GetWindowRect(hWnd, &rc);
    InvalidateRect(NULL, &rc, TRUE);
}

void ShowOpenFileDialog(HWND hWnd)
// Source 1: https://docs.microsoft.com/en-us/windows/win32/dlgbox/using-common-dialog-boxes
// Source 2: https://stackoverflow.com/questions/18265813/getting-a-file-folder-from-open-file-dialog
{
    OPENFILENAME ofn;       // common dialog box structure
    char szFile[MAX_PATH];  // buffer for file name
    HANDLE hf = NULL;       // file handle

    for (int i = 0; i < MAX_PATH; i++)
    {
        szFile[i] = '\0';
    }

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = (LPWSTR)szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Bitmap files\0*.bmp;.BMP\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Parse file and bitmap
    GetOpenFileName(&ofn);
    size_t outputLength = 0;
    for (int i = 0; i < MAX_PATH; i++)
    {
        if (szFile[i] == '\0') continue;
        bitmapFilePath[outputLength++] = szFile[i];
    }
    bitmapFilePath[outputLength] = '\0';
    HBITMAP hBitMap = (HBITMAP)LoadImage(NULL, bitmapFilePath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if(hBitMap != NULL) backgroundColor = CreatePatternBrush(hBitMap);
}

