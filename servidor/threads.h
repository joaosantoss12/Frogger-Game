#include "..\library\utils.h"

BOOL WINAPI criaNamedPipeParaClientesTabuleiroJogo(LPVOID p);
DWORD WINAPI clienteConectaNamedPipeTabuleiro(LPVOID p);
DWORD WINAPI clienteLerNamedPipeTabuleiro(LPVOID p);

BOOL WINAPI updateMapGameToOperador(LPVOID p);
BOOL WINAPI inGameClock(LPVOID p);
BOOL WINAPI receiveCommandFromOperador(LPVOID p);
BOOL WINAPI executeCommandsServidor(LPVOID p);	// FUTURAMENTE VAI SER A THREAD QUE RECEBE INDICAÇÂO DO UTILIZADOR PARA COMEÇAR JOGO

BOOL WINAPI closeAllThreads(LPVOID p);