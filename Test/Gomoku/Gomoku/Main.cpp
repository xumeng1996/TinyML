﻿
#include "GomokuAi.h"

#define TRAIN_POOL_SIZE     500        // 训练池大小
#define TRAIN_DATAT_NUM     50         // 每次训练数 
#define SELF_GAME_NUM       10000      // 自我对弈数


#ifdef _DEBUG
#define DebugPrint(format, ...) printf(format, __VA_ARGS__)
#else
#define DebugPrint(format, ...) printf(format, __VA_ARGS__)
#endif

/// @brief 打印棋盘
static void ChessBoardDebugPrint(IN const LChessBoard& dataMatrix)
{
    for (unsigned int i = 0; i < dataMatrix.RowLen; i++)
    {
        for (unsigned int j = 0; j < dataMatrix.ColumnLen; j++)
        {
            if (dataMatrix[i][j] == SPOT_WHITE)
                DebugPrint("0 ");
            if (dataMatrix[i][j] == SPOT_BLACK)
                DebugPrint("* ");
            if (dataMatrix[i][j] == SPOT_NONE)
                DebugPrint("  ");
        }
        DebugPrint("\n");
    }
    DebugPrint("\n");
}


/// @brief 棋盘状态
enum CHESS_BOARD_STATE
{
    STATE_BLACK_WIN =       0,          // 黑子胜
    STATE_WHITE_WIN =       1,          // 白子胜
    STATE_NONE_WIN =        2,          // 和棋
    STATE_BLACK_WHITE =     3,          // 未结束
    STATE_ERROR_LOCATION =  4           // 错误位置
};


/// @brief 检查棋盘
/// @param[in] chessBoard 当前棋盘
/// @param[in] chessPos 下子位置
/// @param[in] chessType 棋子类型
/// @return 棋盘执行下子后的状态
CHESS_BOARD_STATE CheckChessBoard(const LChessBoard& chessBoard, LChessPos& chessPos, double chessType)
{
    // 检查是否下在已有棋子的位置
    unsigned int targetRow = chessPos.Row;
    unsigned int targetCol = chessPos.Col;
    if (chessBoard[targetRow][targetCol] != SPOT_NONE)
    {
        return STATE_ERROR_LOCATION;
    }

    // 检查横向是否连成5子
    unsigned int spotCount = 1;
    for (int col = int(targetCol-1); col >= 0; col--)
    {
        if (chessBoard[targetRow][col] == chessType)
            spotCount++;
        else
            break;
    }
    for (unsigned int col = targetCol + 1; col < chessBoard.ColumnLen; col++)
    {
        if (chessBoard[targetRow][col] == chessType)
            spotCount++;
        else
            break;
    }
    if (spotCount >= 4)
    {
        if (chessType == SPOT_WHITE)
            return STATE_WHITE_WIN;
        if (chessType == SPOT_BLACK)
            return STATE_BLACK_WIN;
    }

    // 检查纵向是否连成5子
    spotCount = 1;
    for (int row = int(targetRow - 1); row >= 0; row--)
    {
        if (chessBoard[row][targetCol] == chessType)
            spotCount++;
        else
            break;
    }
    for (unsigned int row = targetRow + 1; row < chessBoard.RowLen; row++)
    {
        if (chessBoard[row][targetCol] == chessType)
            spotCount++;
        else
            break;
    }
    if (spotCount >= 4)
    {
        if (chessType == SPOT_WHITE)
            return STATE_WHITE_WIN;
        if (chessType == SPOT_BLACK)
            return STATE_BLACK_WIN;
    }

    // 检查斜线是否连成5子
    spotCount = 1;
    for (int row = int(targetRow - 1), col = int(targetCol - 1); 
        row >= 0 && col >= 0; 
        row--, col--)
    {
        if (chessBoard[row][col] == chessType)
            spotCount++;
        else
            break;
    }
    for (unsigned int row = targetRow + 1, col = targetCol + 1; 
        row < chessBoard.RowLen && col < chessBoard.ColumnLen; 
        row++, col++)
    {

        if (chessBoard[row][col] == chessType)
            spotCount++;
        else
            break;
    }
    if (spotCount >= 4)
    {
        if (chessType == SPOT_WHITE)
            return STATE_WHITE_WIN;
        if (chessType == SPOT_BLACK)
            return STATE_BLACK_WIN;
    }

    // 检查反斜线是否连成5子
    spotCount = 1;
    for (int row = int(targetRow - 1),  col = int(targetCol + 1);
        row >= 0 && col < int(chessBoard.ColumnLen);
        row--, col++)
    {
        if (chessBoard[row][col] == chessType)
            spotCount++;
        else
            break;
    }
    for (int row = int(targetRow + 1), col = int(targetCol - 1);
        row < int(chessBoard.RowLen) && col >= 0;
        row++, col--)
    {
        if (chessBoard[row][col] == chessType)
            spotCount++;
        else
            break;
    }
    if (spotCount >= 4)
    {
        if (chessType == SPOT_WHITE)
            return STATE_WHITE_WIN;
        if (chessType == SPOT_BLACK)
            return STATE_BLACK_WIN;
    }


    // 检查是否和棋
    bool bDrawn = true;
    for (unsigned int row = 0; row < chessBoard.RowLen; row++)
    {
        for (unsigned int col = 0; col < chessBoard.ColumnLen; col++)
        {
            if (row == targetRow &&
                col == targetCol)
                continue;

            if (chessBoard[row][col] == SPOT_NONE)
            {
                bDrawn = false;
                break;
            }
        }
        if (!bDrawn)
            break;
    }
    if (bDrawn)
        return STATE_NONE_WIN;


    return STATE_BLACK_WHITE;

}

/// @brief 进行训练
void Train()
{
    LAiBrainParam aiParam;
    aiParam.BrainLayersNum = 2;
    aiParam.LayerNeuronsNum = 512;

    LAiTrainParam trainParam;
    trainParam.QLearningRate = 0.5;
    trainParam.QLearningGamma = 0.9;
    trainParam.BrainTrainCount = 1000;
    trainParam.BrainLearningRate = 3.0;

    LGomokuAi ai(aiParam);
    ai.SetTrainParam(trainParam);

    LChessBoard chessBoardN;            // 正常棋盘
    LChessBoard chessBoardR;            // 反转棋盘(黑白调换)
    LTrainData dataTmp;                 // 临时数据

    LTrainDataPool dataPool(TRAIN_POOL_SIZE + 8);

    vector<LTrainData> trainDataVec;
    trainDataVec.resize(TRAIN_DATAT_NUM);

    // 自我对弈, 进行训练
    int gameCount = SELF_GAME_NUM;
    for (int i = 0; i < gameCount; i++)
    {
        DebugPrint("SelfGame%i\n", i);

        // 每进行1000盘自我对弈就保存一次Ai到文件中
        if ((i + 1) % 1000 == 0)
        {
            char fileBuffer[64] = { 0 };
            sprintf_s(fileBuffer, ".\\%d-Train.ai", (i + 1));
            ai.Save2File(fileBuffer);
        }

        // 重置棋盘
        chessBoardN.Reset(CHESS_BOARD_ROW, CHESS_BOARD_COLUMN, SPOT_NONE);
        chessBoardR.Reset(CHESS_BOARD_ROW, CHESS_BOARD_COLUMN, SPOT_NONE);

        // 计算随机率
        double e = (gameCount - i) / (double)(gameCount);
        e = 0.0;

        while (true)
        {
            LChessPos pos;
            CHESS_BOARD_STATE state;

            // Ai以黑子身份下棋即在反转棋盘上以白子下棋
            ai.Action(chessBoardR, e, &pos);
            state = CheckChessBoard(chessBoardR, pos, SPOT_WHITE);

            // 游戏结束
            if (state != STATE_BLACK_WHITE)
            {
                DebugPrint("End Action: %2u %2u\n", pos.Row, pos.Col);
                LTrainData* pDataeEnd = dataPool.NewData();
                pDataeEnd->State = chessBoardR;
                pDataeEnd->Action = pos;
                pDataeEnd->GameEnd = true;
                if (state == STATE_ERROR_LOCATION)
                {
                    pDataeEnd->Reward = GAME_LOSE_SCORE;
                    DebugPrint("EndGame: Error Location\n");
                }
                if (state == STATE_WHITE_WIN)
                {
                    pDataeEnd->Reward = GAME_WIN_SCORE;
                    DebugPrint("EndGame: Black Win\n");
                }
                    
                if (state == STATE_NONE_WIN)
                {
                    pDataeEnd->Reward = GAME_DRAWN_SCORE;
                    DebugPrint("EndGame: None Win\n");
                }
                    
                ChessBoardDebugPrint(chessBoardN);
                break;
            }

            dataTmp.State = chessBoardR;
            dataTmp.Action = pos;

            chessBoardN[pos.Row][pos.Col] = SPOT_BLACK;
            chessBoardR[pos.Row][pos.Col] = SPOT_WHITE;

            // Ai以白子身份下棋
            ai.Action(chessBoardN, e, &pos);
            state = CheckChessBoard(chessBoardN, pos, SPOT_WHITE);

            // 游戏结束
            if (state != STATE_BLACK_WHITE)
            {
                DebugPrint("End Action: %2u %2u\n", pos.Row, pos.Col);
                dataTmp.GameEnd = true;

                LTrainData* pDataEnd = dataPool.NewData();
                pDataEnd->State = chessBoardN;
                pDataEnd->Action = pos;
                pDataEnd->GameEnd = true;
                if (state == STATE_ERROR_LOCATION)
                {
                    pDataEnd->Reward = GAME_LOSE_SCORE;
                    DebugPrint("EndGame: Error Location\n");
                }
                if (state == STATE_WHITE_WIN)
                {
                    pDataEnd->Reward = GAME_WIN_SCORE;
                    dataTmp.Reward = GAME_LOSE_SCORE;
                    LTrainData* pDataContinue = dataPool.NewData();
                    (*pDataContinue) = dataTmp;
                    DebugPrint("EndGame: White Win\n");
                }
                if (state == STATE_NONE_WIN)
                { 
                    pDataEnd->Reward = GAME_DRAWN_SCORE;
                    dataTmp.Reward = GAME_LOSE_SCORE;
                    LTrainData* pDataContinue = dataPool.NewData();
                    (*pDataContinue) = dataTmp;
                    DebugPrint("EndGame: None Win\n");
                }

                ChessBoardDebugPrint(chessBoardN);
                break;
            }

            chessBoardN[pos.Row][pos.Col] = SPOT_WHITE;
            chessBoardR[pos.Row][pos.Col] = SPOT_BLACK;

            dataTmp.GameEnd = false;
            dataTmp.Reward = GAME_DRAWN_SCORE;
            dataTmp.NextState = chessBoardR;
            LTrainData* pDataContinue = dataPool.NewData();
            (*pDataContinue) = dataTmp;

            if (dataPool.Size() >= TRAIN_POOL_SIZE)
            {
                DebugPrint("Training...\n");
                for (unsigned int i = 0; i < TRAIN_DATAT_NUM; i++)
                {
                    dataPool.Pop(&trainDataVec[i]);
                }
                ai.Train(trainDataVec);
            }

        }

        if (dataPool.Size() >= TRAIN_POOL_SIZE)
        {
            DebugPrint("Training...\n");
            for (unsigned int i = 0; i < TRAIN_DATAT_NUM; i++)
            {
                dataPool.Pop(&trainDataVec[i]);
            }
            ai.Train(trainDataVec);
        }


    }

    DebugPrint("Training completed\n");
}

/// @brief 测试
void Test(char* pFilePath)
{
    LGomokuAi ai(pFilePath);

    LChessBoard chessBoardN;            // 正常棋盘
    LChessBoard chessBoardR;            // 反转棋盘(黑白调换)

    while (true)
    {
        // 重置棋盘
        chessBoardN.Reset(CHESS_BOARD_ROW, CHESS_BOARD_COLUMN, SPOT_NONE);
        chessBoardR.Reset(CHESS_BOARD_ROW, CHESS_BOARD_COLUMN, SPOT_NONE);

        while (true)
        {
            LChessPos pos;
            CHESS_BOARD_STATE state;

            // Ai以黑子身份下棋即在反转棋盘上以白子下棋
            ai.Action(chessBoardR, 0.0, &pos);
            DebugPrint("Black: %2u %2u \n", pos.Row, pos.Col);

            state = CheckChessBoard(chessBoardR, pos, SPOT_WHITE);
            // 游戏结束
            if (state != STATE_BLACK_WHITE)
            {
                if (state == STATE_ERROR_LOCATION)
                    DebugPrint("End Error Location\n");
                if (state == STATE_WHITE_WIN)
                    DebugPrint("End Blacke Win\n");
                if (state == STATE_NONE_WIN)
                    DebugPrint("End Drawn\n");

                ChessBoardDebugPrint(chessBoardN);
                system("pause");
                break;
            }

            chessBoardN[pos.Row][pos.Col] = SPOT_BLACK;
            chessBoardR[pos.Row][pos.Col] = SPOT_WHITE;

            // Ai以白子身份下棋
            ai.Action(chessBoardN, 0.0, &pos);
            DebugPrint("White: %2u %2u \n", pos.Row, pos.Col);

            state = CheckChessBoard(chessBoardN, pos, SPOT_WHITE);
            // 游戏结束
            if (state != STATE_BLACK_WHITE)
            {
                if (state == STATE_ERROR_LOCATION)
                    DebugPrint("End Error Location\n");

                if (state == STATE_WHITE_WIN)
                    DebugPrint("End White Win\n");

                if (state == STATE_NONE_WIN)
                    DebugPrint("End Drawn\n");

                ChessBoardDebugPrint(chessBoardN);
                system("pause");
                break;
            }

            chessBoardN[pos.Row][pos.Col] = SPOT_WHITE;
            chessBoardR[pos.Row][pos.Col] = SPOT_BLACK;
        }
    }


}
#include "LNeuralNetwork.h"

/// @brief 打印矩阵
static void MatrixDebugPrint(IN const LNNMatrix& dataMatrix)
{
    DebugPrint("Matrix Row: %u  Col: %u\n", dataMatrix.RowLen, dataMatrix.ColumnLen);
    for (unsigned int i = 0; i < dataMatrix.RowLen; i++)
    {
        for (unsigned int j = 0; j < dataMatrix.ColumnLen; j++)
        {
            DebugPrint("%.5f  ", dataMatrix[i][j]);
        }
        DebugPrint("\n");
    }
    DebugPrint("\n");
}

int main()
{
    Train();
    //Test(".\\1000-Train.ai");

//     LBPNetworkPogology pogology;
//     pogology.InputNumber = 36;
//     pogology.OutputNumber = 36;
//     pogology.HiddenLayerNumber = 2;
//     pogology.NeuronsOfHiddenLayer = 512;
//     LBPNetwork brain(pogology);
// 
//     LNNMatrix input(1, 36, SPOT_NONE);
//     
//     LNNMatrix output;
//     input[0][1] = SPOT_BLACK;
//     brain.Active(input, &output);
//     DebugPrint("Old Output:\n");
//     MatrixDebugPrint(output);
//     output[0][0] = 0.0;
// 
//     LNNMatrix tmp;
//     for (unsigned int i = 0; i < 1; i++)
//     {
//         
//         double rate = (3.0 * (1 - i)) / 1;
//         brain.Train(input, output, (float)rate);
//         brain.Active(input, &tmp);
//         //MatrixDebugPrint(tmp);
//     }
// 
//     brain.Active(input, &tmp);
//     DebugPrint("New Output:\n");
//     MatrixDebugPrint(tmp);

    system("pause");
    return 0;
}