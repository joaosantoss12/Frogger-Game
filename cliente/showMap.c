#include "showMap.h"

void copyReceivedGameMap(GameData receivedGameData, GameData* resultGameData) {
    resultGameData->inGame = receivedGameData.inGame;
    resultGameData->idPlayer = receivedGameData.idPlayer;
    resultGameData->pausedGame = receivedGameData.pausedGame;
    resultGameData->won = receivedGameData.won;
    resultGameData->lost = receivedGameData.lost;
    resultGameData->points = receivedGameData.points;
    resultGameData->secondsInGame = receivedGameData.secondsInGame;
    resultGameData->nLines = receivedGameData.nLines;
    resultGameData->nColumns = receivedGameData.nColumns;
    resultGameData->nFaixasRodagem = receivedGameData.nFaixasRodagem;
    resultGameData->velocidadeInicialCarros = receivedGameData.velocidadeInicialCarros;
    resultGameData->carrosPerLane = receivedGameData.carrosPerLane;
    resultGameData->carrosInGame = receivedGameData.carrosInGame;
    resultGameData->carrosStop = receivedGameData.carrosStop;
    resultGameData->level = receivedGameData.level;
    resultGameData->endGame = receivedGameData.endGame;

    //for (DWORD i = 0; i < MAX_PLAYERS; i++) {
        resultGameData->frogger = receivedGameData.frogger;
    //}
    
    for (DWORD i = 0; i < receivedGameData.carrosInGame; i++) {
        resultGameData->cars[i] = receivedGameData.cars[i];
    }
    

}

HBITMAP getMapElement(DWORD b, DWORD n) {

    switch (n){

        case FROGGER:
            switch (b) {
                case 1:
                    return (HBITMAP)LoadImage(NULL, TEXT("imagens/frogger.bmp"), IMAGE_BITMAP, TAM_BITMAP, TAM_BITMAP, LR_LOADFROMFILE);
                case 2:
                    return (HBITMAP)LoadImage(NULL, TEXT("imagens/frogger2.bmp"), IMAGE_BITMAP, TAM_BITMAP, TAM_BITMAP, LR_LOADFROMFILE);
            }
            

        case PARTIDA:
            switch (b) {
                case 1:
                    return (HBITMAP)LoadImage(NULL, TEXT("imagens/partida.bmp"), IMAGE_BITMAP, TAM_BITMAP, TAM_BITMAP, LR_LOADFROMFILE);
                case 2:
                    return (HBITMAP)LoadImage(NULL, TEXT("imagens/partida2.bmp"), IMAGE_BITMAP, TAM_BITMAP, TAM_BITMAP, LR_LOADFROMFILE);
            }

        case ESTRADA:
            switch (b) {
                case 1:
                    return (HBITMAP)LoadImage(NULL, TEXT("imagens/estrada.bmp"), IMAGE_BITMAP, TAM_BITMAP, TAM_BITMAP, LR_LOADFROMFILE);
                case 2:
                    return (HBITMAP)LoadImage(NULL, TEXT("imagens/estrada2.bmp"), IMAGE_BITMAP, TAM_BITMAP, TAM_BITMAP, LR_LOADFROMFILE);
            }

        case CHEGADA:
            switch (b) {
                case 1:
                    return (HBITMAP)LoadImage(NULL, TEXT("imagens/chegada.bmp"), IMAGE_BITMAP, TAM_BITMAP, TAM_BITMAP, LR_LOADFROMFILE);
                case 2:
                    return (HBITMAP)LoadImage(NULL, TEXT("imagens/chegada2.bmp"), IMAGE_BITMAP, TAM_BITMAP, TAM_BITMAP, LR_LOADFROMFILE);
            }

        case CARRO:
            switch (b) {
                case 1:
                    return (HBITMAP)LoadImage(NULL, TEXT("imagens/carro.bmp"), IMAGE_BITMAP, TAM_BITMAP, TAM_BITMAP, LR_LOADFROMFILE);
                case 2:
                    return (HBITMAP)LoadImage(NULL, TEXT("imagens/carro2.bmp"), IMAGE_BITMAP, TAM_BITMAP, TAM_BITMAP, LR_LOADFROMFILE);
            }

        case OBSTÁCULO:
            switch (b) {
                case 1:
                    return (HBITMAP)LoadImage(NULL, TEXT("imagens/obstáculo.bmp"), IMAGE_BITMAP, TAM_BITMAP, TAM_BITMAP, LR_LOADFROMFILE);
                case 2:
                    return (HBITMAP)LoadImage(NULL, TEXT("imagens/obstáculo2.bmp"), IMAGE_BITMAP, TAM_BITMAP, TAM_BITMAP, LR_LOADFROMFILE);
            }

    }
}