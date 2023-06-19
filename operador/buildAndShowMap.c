#include "buildAndShowMap.h"

void printElement(DWORD element){
	switch(element) {
		case FROGGER:
			_tprintf(TEXT("@"));
			break;
		case PARTIDA:
			_tprintf(TEXT("="));
			break;
		case ESTRADA:
			_tprintf(TEXT(" "));
			break;
		case CHEGADA:
			_tprintf(TEXT("#"));
			break;
		case CARRO:
			_tprintf(TEXT("~"));
			break;
		case OBSTÁCULO:
			_tprintf(_T("|"));
	}
}

void buildAndShowMap(GameData* gameData) {
	_tprintf(TEXT("|"));
	for (DWORD i = 0; i < gameData->nLines; i++) {
		for (DWORD j = 0; j < gameData->nColumns; j++) {
			printElement(gameData->gameMap[i][j]);
		}
		if (i != gameData->nLines - 1)
			_tprintf(TEXT("|\n|"));
		else
			_tprintf(TEXT("|"));
	}

	_tprintf(TEXT("\nPontuação do Jogador %d: %d pontos"), gameData->idPlayer, gameData->points);
	_tprintf(TEXT("\nSegundos: %d\n"), gameData->secondsInGame);
}