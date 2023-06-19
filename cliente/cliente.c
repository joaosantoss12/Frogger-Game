#include "cliente.h"
#include "threads.h"
#include <windowsx.h>
#include "framework.h"
#include "..\\library\utils.h"


// ID timer
#define TIMER_ID 1
#define TIMER_INTERVAL 300 // 0.3 segundos

#define MAX_LOADSTRING 100

// Variáveis Globais:
HINSTANCE hInst;                                // instância atual
WCHAR szTitle[MAX_LOADSTRING];                  // O texto da barra de título
WCHAR szWindowClass[MAX_LOADSTRING];            // o nome da classe da janela principal

HWND hButtonLeft, hButtonRight, hTextBox;
HBITMAP hMenuImage;

// Declarações de encaminhamento de funções incluídas nesse módulo de código:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Coloque o código aqui.

    // Inicializar cadeias de caracteres globais
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLIENTE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Realize a inicialização do aplicativo:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENTE));

    MSG msg;

    // Loop de mensagem principal:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }


    return (int)msg.wParam;
}



//
//  FUNÇÃO: MyRegisterClass()
//
//  FINALIDADE: Registra a classe de janela.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENTE));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CLIENTE);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNÇÃO: InitInstance(HINSTANCE, int)
//
//   FINALIDADE: Salva o identificador de instância e cria a janela principal
//
//   COMENTÁRIOS:
//
//        Nesta função, o identificador de instâncias é salvo em uma variável global e
//        crie e exiba a janela do programa principal.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Armazenar o identificador de instância em nossa variável global

    HWND hWnd = CreateWindowW(szWindowClass, TEXT("FROGGER"), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNÇÃO: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  FINALIDADE: Processa as mensagens para a janela principal.
//
//  WM_COMMAND  - processar o menu do aplicativo
//  WM_PAINT    - Pintar a janela principal
//  WM_DESTROY  - postar uma mensagem de saída e retornar
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    RECT rect;
    PAINTSTRUCT ps;
    MINMAXINFO* mmi;
    static int xpto = 0;

    static HDC bmpDC;
    HBITMAP hBmp;
    static BITMAP bmp;
    static int xBitmap;
    static int yBitmap;
    static HANDLE hMutex;

    static HDC memDC = NULL;

    static threadDataDrawMap drawMap;
    static TRACKMOUSEEVENT tme;

    static HWND hBorda2, hNome, hIndividual, hCompeticao, hBorda;

    TCHAR buffer[MAX_TAM];

    switch (message)
    {

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_BITMAP1:
            drawMap.gameData.bitMap = 1;
            break;
        case IDM_BITMAP2:
            drawMap.gameData.bitMap = 2;
            break;
        case IDM_EXIT:
            if (MessageBox(hWnd, TEXT("   Tem a certeza que quer sair?"),
                TEXT("Sair do Jogo"), MB_ICONQUESTION | MB_YESNO) == IDYES)
            {
                DestroyWindow(hWnd);
            }
            break;

        case IDM_AUTORES:
            MessageBox(hWnd, TEXT("   Beatriz Maia - a2020128841\n\n   João Santos - a2020136093"),
                TEXT("Autores"), MB_ICONINFORMATION | MB_OK);

            break;
        }

        switch (wParam) {
            case 1:
                // INDIVIDUAL
                DestroyWindow(hBorda);
                DestroyWindow(hNome);
                DestroyWindow(hBorda2);
                DestroyWindow(hIndividual);
                DestroyWindow(hCompeticao);

                sendMessage('S');       // S -> começar jogo

                SetTimer(hWnd, TIMER_ID, TIMER_INTERVAL, NULL);

                CreateThread(NULL, 0, getMapFromServer, &drawMap, 0, NULL);
                break;
            case 2:
                // COMPETIÇÃO
                break;
        }
        break;

    case WM_CREATE:
        HINSTANCE hAuxLibrary;
        hAuxLibrary = LoadLibrary(FULL_PATH_DLL);

        if (hAuxLibrary == NULL) {
            _tprintf(TEXT("Erro ao abrir a DLL!\n"));
            exit(-1);
        }

        PFUNC_TypeBool_NoArguments checkIfServerRunning;
        checkIfServerRunning = (PFUNC_TypeBool_NoArguments)GetProcAddress(hAuxLibrary, "checkIfServerRunning");

        if (checkIfServerRunning() != TRUE) {
            MessageBox(hWnd, TEXT("Não existe um servidor aberto!"), TEXT("Erro"), MB_OK | MB_ICONERROR);
            PostQuitMessage(0);
            return -1;
           
        }

        HANDLE hSemaphore = CreateSemaphore(NULL, 2, 2, TEXT("MyNamedSemaphore"));
        if (hSemaphore == NULL)
        {
            MessageBox(hWnd, TEXT("Erro ao iniciar o programa!"), TEXT("Erro"), MB_OK | MB_ICONERROR);
            return -1;
        }
        if (WaitForSingleObject(hSemaphore, 0) == WAIT_TIMEOUT)
        {
            MessageBox(hWnd, TEXT("Número máximo de jogadores atingido!"), TEXT("Erro"), MB_OK | MB_ICONERROR);
            PostQuitMessage(0);
            return -1;
        }

        drawMap.gameData.bitMap = 1;

        // Get the dimensions of the application window
        RECT windowRect;
        GetClientRect(hWnd, &windowRect);

        // Calculate the center position for the UI
        int centerX = (windowRect.right - windowRect.left) / 2;
        int centerY = (windowRect.bottom - windowRect.top) / 2;

        // Adjust the position of the UI elements
        hBorda = CreateWindowW(L"static", L"Insira o seu nome: ", WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER,
            centerX - 150, centerY - 225, 300, 350, hWnd, NULL, NULL, NULL);
        hNome = CreateWindowW(L"Edit", L" ", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            centerX - 135, centerY - 190, 270, 50, hWnd, NULL, NULL, NULL);
        hBorda2 = CreateWindowW(L"static", L"Tipo de jogo", WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER,
            centerX - 135, centerY - 105, 270, 200, hWnd, NULL, NULL, NULL);
        hIndividual = CreateWindowW(L"Button", L"Individual", WS_VISIBLE | WS_CHILD,
            centerX - 120, centerY - 55, 240, 50, hWnd, (HMENU)1, NULL, NULL);
        hCompeticao = CreateWindowW(L"Button", L"Competição", WS_VISIBLE | WS_CHILD,
            centerX - 120, centerY + 15, 240, 50, hWnd, (HMENU)2, NULL, NULL);

        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_HOVER;
        tme.hwndTrack = hWnd;
        tme.dwHoverTime = 2000;

        hBmp = (HBITMAP)LoadImage(NULL, TEXT("imagens/frogger.bmp"), IMAGE_BITMAP, 100, 100,
            LR_LOADFROMFILE);
        //BITMAP
        GetObject(hBmp, sizeof(bmp), &bmp);
        hdc = GetDC(hWnd);
        bmpDC = CreateCompatibleDC(hdc);
        SelectObject(bmpDC, hBmp);
        ReleaseDC(hWnd, hdc);

        GetClientRect(hWnd, &rect);

        //apanhar o meio 
        xBitmap = (rect.right / 2) - (bmp.bmWidth / 2);
        yBitmap = (rect.bottom / 2) - (bmp.bmHeight / 2);


        hMutex = CreateMutex(NULL, FALSE, NULL);
        drawMap.hMutex = hMutex;
        drawMap.hWnd = hWnd;
        drawMap.xBitmap = &xBitmap;
        drawMap.yBitmap = &yBitmap;
        drawMap.bmp = bmp;
        drawMap.bmpDC = bmpDC;
        drawMap.memDC = &memDC;

        break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        GetClientRect(hWnd, &rect);

        if (memDC == NULL) //primeira vez
        {
            memDC = CreateCompatibleDC(hdc);
            hBmp = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
            SelectObject(memDC, hBmp);
            DeleteObject(hBmp);
            FillRect(memDC, &rect, CreateSolidBrush(RGB(220, 220, 220)));
            BitBlt(memDC, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpDC, 0, 0, SRCCOPY);
        }

        BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);

        EndPaint(hWnd, &ps);
        break;

    case WM_ERASEBKGND: //Sent when the window background must be erased (for example, when a window is resized). 
        return 1;

    case WM_LBUTTONDOWN:
        POINT mousePos2;
        GetCursorPos(&mousePos2);
        ScreenToClient(hWnd, &mousePos2);

        int xx = mousePos2.x / TAM_BITMAP;
        int yy = mousePos2.y / TAM_BITMAP;

        if (xx == drawMap.gameData.frogger.y && yy == drawMap.gameData.frogger.x - 1)
            sendMessage('U');
        else if (xx == drawMap.gameData.frogger.y && yy == drawMap.gameData.frogger.x + 1)
            sendMessage('D');
        else if (xx == drawMap.gameData.frogger.y + 1 && yy == drawMap.gameData.frogger.x)
            sendMessage('R');
        else if (xx == drawMap.gameData.frogger.y - 1 && yy == drawMap.gameData.frogger.x)
            sendMessage('L');

        break;

    case WM_RBUTTONDOWN:
        POINT mousePos;
        GetCursorPos(&mousePos);
        ScreenToClient(hWnd, &mousePos);

        int x = mousePos.x / TAM_BITMAP;
        int y = mousePos.y / TAM_BITMAP;

        if (x == drawMap.gameData.frogger.y && y == drawMap.gameData.frogger.x) {
            sendMessage('X');
        }

        break;

    case WM_TIMER:
    {
        if (wParam == TIMER_ID) {
            POINT mousePos;
            GetCursorPos(&mousePos);
            ScreenToClient(hWnd, &mousePos);

            int x = mousePos.x / TAM_BITMAP;
            int y = mousePos.y / TAM_BITMAP;

            if (x == drawMap.gameData.frogger.y && y == drawMap.gameData.frogger.x) {
                HDC dc = GetDC(hWnd);
                RECT rc;
                GetClientRect(hWnd, &rc);
                TCHAR coords[MAX_TAM];
                swprintf_s(coords, MAX_TAM, TEXT("O SAPO CHEGOU AO\nFIM DA RUA %d VEZES!"), drawMap.gameData.level - 1);
                DrawText(dc, coords, -1, &rc, DT_RIGHT | DT_VCENTER | DT_WORDBREAK | DT_CENTER);
                ReleaseDC(hWnd, dc);
            }
        }
    }
    break;


    case WM_KEYDOWN:        // CLICK SETAS
    {
        HDC dc = GetDC(hWnd);
        RECT rc;
        GetClientRect(hWnd, &rc);
        TCHAR coords[MAX_TAM];

        switch (wParam) {
        case VK_LEFT:
            sendMessage('L');
            break;

        case VK_RIGHT:
            sendMessage('R');
            break;

        case VK_UP:
            sendMessage('U');
            break;

        case VK_DOWN:
            sendMessage('D');
            break;
        }

        ReleaseDC(hWnd, dc);
    }
    break;


    case WM_GETMINMAXINFO: //Sent to a window when the size or position of the window is about to change. An application can use this message to override the window's default maximized size and position, or its default minimum or maximum tracking size.
        mmi = (MINMAXINFO*)lParam;
        mmi->ptMinTrackSize.x = bmp.bmWidth + 2;
        mmi->ptMinTrackSize.y = bmp.bmHeight * 2;
        break;

    case WM_SIZE: //Sent to a window after its size has changed.
        xBitmap = (LOWORD(lParam) / 2) - (bmp.bmWidth / 2);
        yBitmap = (HIWORD(lParam) / 2) - (bmp.bmHeight / 2);
        break;

    case WM_CLOSE:
        if (MessageBox(hWnd, TEXT("Tem a certeza que quer sair?"),
            TEXT("Sair do Jogo"), MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            DestroyWindow(hWnd);
        }
        break;
    case WM_DESTROY: // Destruir a janela e terminar o programa 
        // "PostQuitMessage(Exit Status)"
        PostQuitMessage(0);
        break;
    default:
        // Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
        // não é efectuado nenhum processamento, apenas se segue o "default" do Windows
        return(DefWindowProc(hWnd, message, wParam, lParam));
        break;  // break tecnicamente desnecessário por causa do return
    }
    return 0;
}