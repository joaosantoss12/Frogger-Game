#include "threads.h"
#include "showMap.h"

void resetFroggerPosition(threadDataDrawMap* dados) {
    DWORD initialX = dados->gameData.frogger.xInicial;
    DWORD initialY = dados->gameData.frogger.yInicial;

    DWORD currentX = dados->gameData.frogger.x;
    DWORD currentY = dados->gameData.frogger.y;


    if (0 < dados->gameData.frogger.x < dados->gameData.nLines)
        dados->gameData.gameMap[currentX][currentY] = ESTRADA;

    dados->gameData.gameMap[initialX][initialY] = FROGGER;
}



HANDLE hPipe = INVALID_HANDLE_VALUE; // Global or static variable

void connectToPipe() {
    if (hPipe != INVALID_HANDLE_VALUE) {
        return; // PIPE JA ESTA CONECTADO
    }

    if (!WaitNamedPipe(PIPE_NAME_MESSAGES, NMPWAIT_WAIT_FOREVER)) {
        _tprintf(TEXT("[ERRO] Ao esperar pelo pipe!"));
        exit(-1);
    }

    hPipe = CreateFile(
        PIPE_NAME_MESSAGES,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        _tprintf(TEXT("[ERRO] Impossivel abrir o pipe!"));
        exit(-1);
    }

    // ?
    DWORD dwMode = PIPE_READMODE_MESSAGE;
    if (!SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL)) {
        _tprintf(TEXT("[ERRO] Impossivel definir modo do pipe!"));
        exit(-1);
    }
}

void sendMessage(TCHAR msg) {
    connectToPipe();

    DWORD bytesWritten;
    WriteFile(hPipe, &msg, sizeof(TCHAR), &bytesWritten, NULL);
}



HFONT loadFont() {
    // Definir as informações da fonte
    LOGFONT lf;
    ZeroMemory(&lf, sizeof(LOGFONT));
    lf.lfHeight = 24; // Tamanho da fonte
    lf.lfWeight = FW_BOLD; // Peso da fonte (negrito)
    strcpy_s(lf.lfFaceName, LF_FACESIZE, "Calibri"); // Nome da fonte

    // Criar a fonte a partir das informações
    return CreateFontIndirect(&lf);
}

DWORD WINAPI getMapFromServer(LPVOID p) {

    threadDataDrawMap* data = (threadDataDrawMap*)p;

    GameData receivedData;

    HANDLE hPipe;
    BOOL ret;
    DWORD n;

    RECT rect;
    HDC hdc;
    PAINTSTRUCT ps;

    if (!WaitNamedPipe(PIPE_NAME_MAP, NMPWAIT_WAIT_FOREVER)) {
        exit(-1);
    }

    hPipe = CreateFile(
        PIPE_NAME_MAP, GENERIC_READ, 0, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hPipe == NULL) {
        exit(-1);
    }

    while (TRUE) {
        ret = ReadFile(hPipe, &receivedData, sizeof(GameData), &n, NULL);

        if (!ret || !n) {
            if (GetLastError() == ERROR_BROKEN_PIPE) {
                if (MessageBox(data->hWnd, TEXT("O Servidor terminou a execução!"),
                    TEXT("Sair do Jogo"), MB_ICONWARNING | MB_OK) == IDOK)
                {
                    PostMessage(data->hWnd, WM_DESTROY, 0, 0);
                }
                break;
            }
            break;
        }

        copyReceivedGameMap(receivedData, &data->gameData);

        if(data->gameData.endGame)
            if (MessageBox(data->hWnd, TEXT("Perdeu o jogo :("),
                TEXT("Perdeu"), MB_ICONSTOP | MB_OK) == IDOK)
            {
                PostMessage(data->hWnd, WM_DESTROY, 0, 0);
            }

        for (int j = 0; j < data->gameData.nLines; j++)
            for (int k = 0; k < data->gameData.nColumns; k++)
                data->gameData.gameMap[j][k] = receivedData.gameMap[j][k];

                WaitForSingleObject(data->hMutex, INFINITE);

                if (*data->memDC != NULL) {
                    FillRect(*data->memDC, &rect, CreateSolidBrush(RGB(220, 220, 220)));

                    for (int j = 0; j < data->gameData.nLines; j++) {
                        for (int k = 0; k < data->gameData.nColumns; k++) {
                            SelectObject(data->bmpDC, getMapElement(data->gameData.bitMap, data->gameData.gameMap[j][k]));

                            GetClientRect(data->hWnd, &rect);
                            *data->xBitmap = rect.left + (k * TAM_BITMAP);
                            *data->yBitmap = rect.top + (j * TAM_BITMAP); // Apply the yOffset

                            BitBlt(*data->memDC, *data->xBitmap, *data->yBitmap,
                                data->bmp.bmWidth, data->bmp.bmHeight, data->bmpDC, 0, 0, SRCCOPY);
                        }
                    }

                    *data->xBitmap = rect.left;
                    *data->yBitmap = rect.top + (data->gameData.nLines * TAM_BITMAP); // Apply the yOffset
  
                  
                    // Criar a fonte a partir das informações
                    HFONT hFont = loadFont();

                    // Selecionar a fonte no contexto de dispositivo
                    SelectObject(*data->memDC, hFont);

                    // Definir a cor e o fundo do texto
                    SetTextColor(*data->memDC, RGB(0, 0, 0)); // Cor do texto
                    SetBkMode(*data->memDC, TRANSPARENT); // Fundo transparente

                    // Calcular as coordenadas para a pontuação e os segundos
                    RECT textRect;
                    textRect.left = rect.left;
                    textRect.top = rect.top + ((data->gameData.nLines + 1) * TAM_BITMAP);
                    textRect.right = rect.right;
                    textRect.bottom = textRect.top + 65; // Altura desejada para o texto da pontuação


                    // Desenhar o texto da pontuação
                    char text[100];
                    sprintf_s(text, sizeof(text), "Pontuação: %d                                    Segundos: %d", data->gameData.points, data->gameData.secondsInGame);
                    DrawTextA(*data->memDC, text, -1, &textRect, DT_CENTER | DT_VCENTER);

                    sprintf_s(text, sizeof(text), "Nivel: %d", data->gameData.level);
                    DrawTextA(*data->memDC, text, -1, &textRect, DT_LEFT | DT_VCENTER);

   

                    // Lembre-se de excluir a fonte criada após o uso
                    DeleteObject(hFont);

                }

                ReleaseMutex(data->hMutex);

                InvalidateRect(data->hWnd, NULL, TRUE);

                Sleep(500);
            }


            CloseHandle(hPipe);

            return 0;
        }
 