﻿
#include "GomokuControl.h"

#include <vector>
using std::vector;

#ifdef _DEBUG
#define DebugPrint(format, ...) printf(format, __VA_ARGS__)
#else
#define DebugPrint(format, ...)
#endif

/// @brief 棋盘状态
enum CHESS_BOARD_STATE
{
    STATE_BLACK_WIN = 0,        // 黑子胜
    STATE_WHITE_WIN = 1,        // 白子胜
    STATE_NONE_WIN = 2,         // 和棋
    STATE_BLACK_WHITE = 3       // 未结束
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
        if (chessType == SPOT_WHITE)
            return STATE_BLACK_WIN;
        if (chessType == SPOT_BLACK)
            return STATE_WHITE_WIN;
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
    if (spotCount >= 5)
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
    if (spotCount >= 5)
    {
        if (chessType == SPOT_WHITE)
            return STATE_WHITE_WIN;
        if (chessType == SPOT_BLACK)
            return STATE_BLACK_WIN;
    }

    // 检查斜线是否连成5子
    spotCount = 1;
    for (int row = int(targetRow - 1); row >= 0; row--)
    {
        for (int col = int(targetCol - 1); col >= 0; col--)
        {
            if (chessBoard[row][col] == chessType)
                spotCount++;
            else
                break;
        }
        
    }
    for (unsigned int row = targetRow + 1; row < chessBoard.RowLen; row++)
    {
        for (unsigned int col = targetCol + 1; col < chessBoard.ColumnLen; col++)
        {
            if (chessBoard[row][col] == chessType)
                spotCount++;
            else
                break;
        }

    }
    if (spotCount >= 5)
    {
        if (chessType == SPOT_WHITE)
            return STATE_WHITE_WIN;
        if (chessType == SPOT_BLACK)
            return STATE_BLACK_WIN;
    }

    // 检查反斜线是否连成5子
    spotCount = 1;
    for (int row = int(targetRow - 1); row >= 0; row--)
    {
        for (unsigned int col = targetCol + 1; col < chessBoard.ColumnLen; col++)
        {
            if (chessBoard[row][col] == chessType)
                spotCount++;
            else
                break;
        }

    }
    for (unsigned int row = targetRow + 1; row < chessBoard.RowLen; row++)
    {
        for (int col = int(targetCol - 1); col >= 0; col--)
        {
            if (chessBoard[row][col] == chessType)
                spotCount++;
            else
                break;
        }

    }
    if (spotCount >= 5)
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

int main()
{
    LGomokuAi ai;
    LChessBoard chessBoardN;            // 正常棋盘
    LChessBoard chessBoardR;            // 反转棋盘(黑白调换)

    int poolSize = 200;
    vector<LTrainData> trainPool;       // 训练池
    trainPool.resize(poolSize);
    vector<int> idxVec;                 // 索引向量
    idxVec.resize(poolSize);            // 

    // 自我对弈, 进行训练
    int gameCount = 10000;
    for (int i = 0; i < gameCount; i++)
    {
        // 重置棋盘
        chessBoardN.Reset(CHESS_BOARD_ROW, CHESS_BOARD_COLUMN, SPOT_NONE);
        chessBoardR.Reset(CHESS_BOARD_ROW, CHESS_BOARD_COLUMN, SPOT_NONE);

        // 计算随机率
        double e = (gameCount - i) / (double)(gameCount);

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
                LTrainData data;
                data.GameEnd = true;
                data.State = chessBoardR;
                data.Action = pos;
                if (state == STATE_BLACK_WIN)
                    data.Reward = GAME_LOSE_SCORE;
                if (state == STATE_WHITE_WIN)
                    data.Reward = GAME_WIN_SCORE;
                if (state == STATE_NONE_WIN)
                    data.Reward = GAME_DRAWN_SCORE;

                break;
            }

            chessBoardN[pos.Row][pos.Col] = SPOT_BLACK;
            chessBoardR[pos.Row][pos.Col] = SPOT_WHITE;
            

            // Ai以白子身份下棋
            ai.Action(chessBoardN, e, &pos);
            state = CheckChessBoard(chessBoardN, pos, SPOT_WHITE);
            // 游戏结束
            if (state != STATE_BLACK_WHITE)
            {
                LTrainData data;
                data.GameEnd = true;
                data.State = chessBoardN;
                data.Action = pos;
                if (state == STATE_BLACK_WIN)
                    data.Reward = GAME_LOSE_SCORE;
                if (state == STATE_WHITE_WIN)
                    data.Reward = GAME_WIN_SCORE;
                if (state == STATE_NONE_WIN)
                    data.Reward = GAME_DRAWN_SCORE;

                break;
            }

            chessBoardN[pos.Row][pos.Col] = SPOT_WHITE;
            chessBoardR[pos.Row][pos.Col] = SPOT_BLACK;
            

        }



    }
    return 0;
}