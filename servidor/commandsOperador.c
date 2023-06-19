#include "commandsOperador.h"

BOOL changeLineDirection(SharedGame* sharedGame, DWORD line) {
	
	for (DWORD p = 0; p < MAX_PLAYERS; p++) {
		for (DWORD c = 0; c < sharedGame->gameData[p]->carrosInGame; c++) {
			if (sharedGame->gameData[p]->cars[c].x == line) {
				if (sharedGame->gameData[p]->cars[c].direction == 0)
					sharedGame->gameData[p]->cars[c].direction = 1;
					
				else 
					sharedGame->gameData[p]->cars[c].direction = 0;
			}
		}
	}
	return TRUE;
}

BOOL insertObstacle(SharedGame* sharedGame, DWORD line, DWORD column) {
	for (DWORD p = 0; p < MAX_PLAYERS; p++) {
		for (DWORD i = 0; i < sharedGame->gameSettings.nLines; i++) {
			for (DWORD j = 0; j < sharedGame->gameSettings.nColumns; j++) {
				if (i == line && j == column) {
					sharedGame->gameData[p]->gameMap[i][j] = OBST�CULO;
				}
			}
		}
	}
	
}


void checkCommandFromOperador(SharedGame* sharedGame, TCHAR* receivedCommand) {
	TCHAR* next_param = NULL;
	TCHAR* command = _tcstok_s(receivedCommand, TEXT(" "), &next_param);

	if (_tcscmp(command, TEXT("md")) == 0) {	// MUDAR DIRECAO DOS CARROS DE UMA LINHA
		TCHAR* lineString = _tcstok_s(NULL, TEXT(" "), &next_param);
		
		if (lineString == NULL) {	// NAO COLOCOU A LINHA
			_tprintf(TEXT("\nComando n�o reconhecido!\n"));
			return;
		}	

		DWORD linha = _ttoi(lineString);

		changeLineDirection(sharedGame, linha);

	}
	else if (_tcscmp(command, TEXT("stop")) == 0) {
		TCHAR* segundosString = _tcstok_s(NULL, TEXT(" "), &next_param);

		if (segundosString == NULL) {
			_tprintf(TEXT("\nComando n�o reconhecido!\n"));
			return;
		}

		DWORD segundos = _ttoi(segundosString);

		for (DWORD p = 0; p < MAX_PLAYERS; p++) {
			sharedGame->gameData[p]->carrosStop += segundos;
		}
	}
	else if (_tcscmp(command, TEXT("co")) == 0) {

		TCHAR* lineString = _tcstok_s(NULL, TEXT(" "), &next_param);
		TCHAR* columnString = _tcstok_s(NULL, TEXT(" "), &next_param);

		if (lineString == NULL || columnString == NULL) {
			_tprintf(TEXT("\nComando n�o reconhecido!\n"));
			return;
		}

		DWORD linha = _ttoi(lineString);
		DWORD coluna = _ttoi(columnString);

		linha -= 1;
		coluna -= 1;
		
		if (linha == 0 || linha == sharedGame->gameSettings.nLines - 1) {
			_tprintf(TEXT("\nN�mero da linha inv�lido! N�o � poss�vel colocar obst�culos na partida e/ou chegada!\n"));
			return;

			//if (coluna == 0 || coluna == sharedGame->gameSettings.nColumns - 1) {
				//_tprintf(TEXT("\nN�mero da coluna inv�lido! N�o � poss�vel colocar obst�culos nas margens!\n"));
				//return;
			//}
		}

		insertObstacle(sharedGame, linha, coluna);
	}
}