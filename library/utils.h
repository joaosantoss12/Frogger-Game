#pragma once

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#ifndef UTILS_SO2_TP_H
#define UTILS_SO2_TP_H

#endif

#define PIPE_NAME_MAP TEXT("\\\\.\\pipe\\pipeMapa")
#define PIPE_NAME_MESSAGES TEXT("\\\\.\\pipe\\pipeMensagens")

#define FULL_PATH_DLL TEXT("..\\library\\DLL\\library.dll")	// ALTERAR
#define REGISTRY_PATH TEXT("SO2\\TrabalhoPrático\\")
#define DEFAULT_VALUE_FAIXAS_RODAGEM 5
#define DEFAULT_VALUE_VELOCIDADE_CARROS 2
#define MAX_TAM 50
#define SHARED_MEMORY_GAME TEXT("sharedMemoryFrogger")

#define CIRCULAR_BUFFER_SIZE 10

#define MAX_PLAYERS 2

#define MAX_CARS_PER_LANE 8

#define TAM_BITMAP 50

#define FROGGER 0
#define PARTIDA 1
#define ESTRADA 2
#define CHEGADA 3
#define CARRO 4
#define OBSTÁCULO 5

typedef struct {
	DWORD x;
	DWORD y;

	DWORD xInicial;
	DWORD yInicial;
}Frogger;

typedef struct {
	long x;
	long y;
	long direction;
}Car;



#define MAX_TAM_BUFFER 10
typedef struct{
	TCHAR message[MAX_TAM];
}BufferCell;

typedef struct {
	DWORD wP;
	DWORD rP;

	BufferCell buffer[MAX_TAM_BUFFER];
}BufferCircular;

typedef struct {
	TCHAR nickname[MAX_TAM];
	DWORD idPlayer;

	BOOL inGame;
	BOOL pausedGame;

	BOOL won;
	BOOL lost;

	BOOL endGame;

	DWORD tipoJogo;

	DWORD level;

	DWORD points;
	DWORD secondsInGame;

	DWORD nLines;
	DWORD nColumns;

	DWORD nFaixasRodagem;
	DWORD velocidadeInicialCarros;

	DWORD carrosPerLane;
	DWORD carrosInGame;

	DWORD carrosStop;

	DWORD bitMap;

	Frogger frogger;
	Car cars[MAX_CARS_PER_LANE * 8];	// 8 x 8 é o número máximo de carros possivel num mapa de jogo (8 máx por linha) e (8 linhas de estrada)

	DWORD gameMap[10][20];	// MAX 10 por 20


	// VERIFICAR SE PAROU DE MEXER DURANTE 10 SEGUNDOS
	int inactiveSeconds;

}GameData;

typedef struct {
	DWORD nLines;
	DWORD nColumns;

}GameSettings;

typedef struct {
	HANDLE hMapFileGame[MAX_PLAYERS];

	GameData* gameData[MAX_PLAYERS];

	GameSettings gameSettings;

	HANDLE hEventCloseAll;

	HANDLE hReadWriteMutexUpdateGame;

	HANDLE hEventUpdateGame;
	HANDLE hSemaforoSendGameUpdate;
	HANDLE hEventInGame;

	// BUFFER CIRCULAR MONITOR PARA SERVER
	HANDLE hMapFileBufferCircular_OperadorServidor;
	HANDLE hMutexBufferCircular_OperadorServidor;

	HANDLE hSemaforoReadBufferCircular_OperadorServidor;
	HANDLE hSemaforoWriteBufferCircular_OperadorServidor;

	BufferCircular* bufferCircular_OperadorServidor;


}SharedGame;

//Struct para os namedPipe
typedef struct {
	HANDLE hInstancia;

	OVERLAPPED overlap; //estrutura overlapped para uso asincrono

	BOOL active; //para ver se já tem cliente ou não
} DadosPipe;

typedef struct {
	DadosPipe hPipes[MAX_PLAYERS];

	GameData* gameData[MAX_PLAYERS];

	HANDLE hEvents[MAX_PLAYERS];

	HANDLE hReadWriteMutexUpdateGame;

	HANDLE hEventUpdateGame;

	int deveContinuar;

} PartilhaJogoServidorCliente;


#define N_THREADS_SERVER 4
typedef struct {
	HANDLE hThreads[N_THREADS_SERVER];
	HANDLE hEventCloseAllThreads;
}ThreadsServer;

typedef struct {
	DadosPipe hPipes[MAX_PLAYERS];

	HANDLE hEvents[MAX_PLAYERS];

	HANDLE hMutex;

	int* deveContinuar;
} ThreadsCliente;

#define N_THREADS_OPERADOR 2
typedef struct {
	HANDLE hThreads[N_THREADS_OPERADOR];
	HANDLE hEventCloseAllThread;
}ThreadsOperador;

typedef struct {
	HANDLE hMutex;
	int* xBitmap;
	int* yBitmap;
	HWND hWnd;
	BITMAP bmp;
	HDC bmpDC;
	HDC* memDC;

	GameData gameData;
} threadDataDrawMap;



// keyword para importar funções da DLL
typedef BOOL(*PFUNC_TypeBool_NoArguments) ();
typedef BOOL(*PFUNC_TypeBool_PointerSharedGame) (SharedGame* sharedGame);