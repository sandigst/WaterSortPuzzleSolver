#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

enum class Color : char
{
    UNKNOWN = -1,
    EMPTY = 0,
    BRO, // brown
    DGR, // dark green
    PUR, // purple
    PIN, // pink
    GRA, // gray
    SKY, // sky / bright blue
    RED, // red
    MIN, // mint
    BGR, // bright green
    BLU, // blue / dark blue
    YEL, // yellow
    ORA, // orange
};

std::wstring toString(Color const &color)
{
    switch (color)
    {
    case Color::BRO:
        return L"brown";
    case Color::DGR:
        return L"dark green";
    case Color::PUR:
        return L"purple";
    case Color::PIN:
        return L"pink";
    case Color::GRA:
        return L"gray";
    case Color::SKY:
        return L"sky / bright blue";
    case Color::RED:
        return L"red";
    case Color::MIN:
        return L"mint";
    case Color::BGR:
        return L"bright green";
    case Color::BLU:
        return L"blue / dark blue";
    case Color::YEL:
        return L"yellow";
    case Color::ORA:
        return L"orange";
    case Color::EMPTY:
        return L"Empty";
    }
    return L"Unknown";
}

struct Bottle
{
    Color cells[4] = {Color::EMPTY, Color::EMPTY, Color::EMPTY, Color::EMPTY};
};

struct Move
{
    struct CellKey
    {
        int bottle = 0;
        int cell = 0;

        bool operator==(CellKey const &other) const { return bottle == other.bottle && cell == other.cell; };
    };
    CellKey source; // first source color cell
    CellKey target; // last empty target cell
    Color color = Color::EMPTY;
    int numCells = 0;

    bool operator==(Move const &other) const { return source == other.source && target == other.target && color == other.color && numCells == other.numCells; };
};

struct Board
{
    std::vector<Bottle> bottles;
    std::vector<Move> history;
};

struct State
{
    int totalBottles = 0;
    int fullColoredBottles = 0;
    int emptyBottles = 0;
};

bool isFinished(Bottle const &bottle)
{
    Color const &color = bottle.cells[0];
    for (int i = 1; i < 4; i++)
    {
        if (bottle.cells[i] != color)
        {
            return false;
        }
    }
    return true;
}

bool isFinished(Board const &board)
{
    for (Bottle const &bottle : board.bottles)
    {
        if (!isFinished(bottle))
        {
            return false;
        }
    }
    return true;
}

bool reachedUnknown(Bottle const &bottle)
{
    Color color = Color::EMPTY;
    for (int i = 0; i < 4; i++)
    {
        if (bottle.cells[i] != Color::EMPTY)
        {
            color = bottle.cells[i];
            break;
        }
    }
    return color == Color::UNKNOWN;
}

bool reachedUnknown(Board const &board)
{
    for (Bottle const &bottle : board.bottles)
    {
        if (reachedUnknown(bottle))
        {
            return true;
        }
    }
    return false;
}

bool isFullColored(Bottle const &bottle)
{
    Color const &color = bottle.cells[0];
    if (color == Color::EMPTY)
    {
        return false;
    }
    for (int i = 1; i < 4; i++)
    {
        if (bottle.cells[i] != color)
        {
            return false;
        }
    }
    return true;
}

bool isEmpty(Bottle const &bottle)
{
    for (int i = 0; i < 4; i++)
    {
        if (bottle.cells[i] != Color::EMPTY)
        {
            return false;
        }
    }
    return true;
}

std::optional<Move::CellKey> findPossibleSourceCell(std::vector<Bottle> const &bottles, int bottleOffset)
{
    for (int bottle = bottleOffset; bottle < bottles.size(); bottle++)
    {
        if (!isFinished(bottles[bottle]))
        {
            for (int cell = 0; cell < 4; cell++)
            {
                if (bottles[bottle].cells[cell] != Color::EMPTY)
                {
                    return Move::CellKey{bottle, cell};
                }
            }
        }
    }
    return std::nullopt;
}

std::optional<Move::CellKey> findPossibleTargetCell(std::vector<Bottle> const &bottles, Move::CellKey const &sourceCell, int bottleOffset)
{
    for (int bottle = bottleOffset; bottle < bottles.size(); bottle++)
    {
        if (bottle == sourceCell.bottle)
        {
            continue;
        }

        std::optional<Move::CellKey> result;
        for (int cell = 0; cell < 4; cell++)
        {
            Color const &currentColor = bottles[bottle].cells[cell];
            if (currentColor == Color::EMPTY)
            {
                result = Move::CellKey{bottle, cell};
                if (cell == 3) // empty bottle
                {
                    return result;
                }
            }
            else if (result && currentColor == bottles[sourceCell.bottle].cells[sourceCell.cell])
            {
                // next cell matches color
                return result;
            }
            else
            {
                break;
            }
        }
    }
    return std::nullopt;
}

Move getReverse(Move const &move)
{
    return {{move.target.bottle, move.target.cell - (move.numCells - 1)}, {move.source.bottle, move.source.cell + (move.numCells - 1)}, move.color, move.numCells};
}

bool isMoveValid(std::vector<Move> const &history, Move const &move)
{
    Move reverse = getReverse(move);
    for (Move const &historicMove : history)
    {
        if (historicMove == move)
        {
            return false;
        }
        if (historicMove == reverse)
        {
            return false;
        }
    }
    return true;
}

bool isMoveNeeded(Move const &move)
{
    if (move.target.cell == 3 && move.source.cell + move.numCells == 4)
    {
        return false;
    }

    return true;
}

Move getMove(std::vector<Bottle> const &bottles, Move::CellKey const &source, Move::CellKey const &target)
{
    Color color = bottles[source.bottle].cells[source.cell];
    int colorCells = 0;
    for (int i = source.cell; i < 4; i++)
    {
        if (bottles[source.bottle].cells[i] != color)
        {
            break;
        }
        colorCells++;
    }
    int emptyCells = target.cell + 1;
    return {source, target, color, std::min(colorCells, emptyCells)};
}

void performMove(Board &board, Move const &move)
{
    for (int i = 0; i < move.numCells; i++)
    {
        board.bottles[move.target.bottle].cells[move.target.cell - i] = move.color;
        board.bottles[move.source.bottle].cells[move.source.cell + i] = Color::EMPTY;
    }
    board.history.push_back(move);
}

std::optional<Move> revertLastMove(Board &board)
{
    if (board.history.empty())
    {
        return std::nullopt;
    }

    Move lastMove = board.history.back();
    board.history.pop_back();

    for (int i = 0; i < lastMove.numCells; i++)
    {
        board.bottles[lastMove.target.bottle].cells[lastMove.target.cell - i] = Color::EMPTY;
        board.bottles[lastMove.source.bottle].cells[lastMove.source.cell + i] = lastMove.color;
    }
    return lastMove;
}

std::optional<Move> findNextMove(Board &board, int sourceBottleOffset, int targetBottleOffset)
{
    std::optional<Move> result;
    while (!result && sourceBottleOffset < board.bottles.size())
    {
        std::optional<Move::CellKey> source = findPossibleSourceCell(board.bottles, sourceBottleOffset);
        if (!source)
        {
            break;
        }

        if (std::optional<Move::CellKey> target = findPossibleTargetCell(board.bottles, *source, targetBottleOffset))
        {
            Move move = getMove(board.bottles, *source, *target);
            if (isMoveValid(board.history, move) && isMoveNeeded(move))
            {
                result = move;
                break;
            }
            target = std::nullopt;
        }
        sourceBottleOffset = source->bottle + 1;
        targetBottleOffset = 0;
        source = std::nullopt;
    }
    return result;
}

bool performNextMove(Board &board)
{
    if (isFinished(board) || reachedUnknown(board))
    {
        return false;
    }

    // look for the next move on board
    std::optional<Move> nextMove = findNextMove(board, 0, 0);

    // next move found: revert last move until a next move can be found
    if (!nextMove)
    {
        std::optional<Move> lastMove = revertLastMove(board);
        while (!nextMove && lastMove)
        {
            if (nextMove = findNextMove(board, lastMove->source.bottle, lastMove->target.bottle + 1))
            {
                break;
            }
            lastMove = revertLastMove(board);
        }
    }

    if (nextMove)
    {
        performMove(board, *nextMove);
        return true;
    }
    return false;
}

State getState(Board const &board)
{
    State result{board.bottles.size()};
    for (Bottle const &b : board.bottles)
    {
        if (isFullColored(b))
        {
            result.fullColoredBottles++;
        }
        else if (isEmpty(b))
        {
            result.emptyBottles++;
        }
    }
    return result;
}

void printBoard(State const &state)
{
    static int fullColoredBottles = -1;
    if (state.fullColoredBottles != fullColoredBottles)
    {
        fullColoredBottles = state.fullColoredBottles;
        std::wcout << L"Total: " << std::to_wstring(state.totalBottles) << L" Full colored: " << std::to_wstring(state.fullColoredBottles) << L" Empty: " << std::to_wstring(state.emptyBottles) << std::endl;
    }
}

void printHistory(Board const &board)
{
    std::wcout << L"--- History Start ---" << std::endl;
    for (Move const &move : board.history)
    {
        std::wcout << L"Source: " << std::to_wstring(move.source.bottle) << L" Target: " << std::to_wstring(move.target.bottle)
                   << L" Color: " << toString(move.color) << L" (" << std::to_wstring(move.numCells) << L")" << std::endl;
    }
    std::wcout << L"--- History End ---" << std::endl;
}

bool isValid(Board const &board)
{
    std::unordered_map<Color, int> colorCells;
    for (Bottle const &b : board.bottles)
    {
        for (Color c : b.cells)
        {
            if (colorCells.find(c) == colorCells.end())
            {
                colorCells[c] = 1;
            }
            else
            {
                colorCells[c]++;
            }
        }
    }

    if (colorCells.empty())
    {
        return false;
    }

    int missingCells = 0;
    for (auto const &[col, num] : colorCells)
    {
        if (col != Color::EMPTY && col != Color::UNKNOWN && num != 4)
        {
            missingCells += 4 - num;
        }
    }

    return missingCells == colorCells[Color::UNKNOWN];
}

std::vector<Bottle> const initialBottles = {
    {Color::ORA, Color::BGR, Color::MIN, Color::BLU},
    {Color::PIN, Color::DGR, Color::ORA, Color::DGR},
    {Color::RED, Color::BLU, Color::DGR, Color::YEL},
    {Color::YEL, Color::PIN, Color::PUR, Color::YEL},
    {Color::SKY, Color::BGR, Color::YEL, Color::ORA},
    {Color::MIN, Color::BRO, Color::RED, Color::GRA},
    {Color::BLU, Color::GRA, Color::BRO, Color::PIN},

    {Color::ORA, Color::DGR, Color::BLU, Color::PIN},
    {Color::BRO, Color::MIN, Color::BRO, Color::GRA},
    {Color::DGR, Color::RED, Color::PUR, Color::SKY},
    {Color::GRA, Color::BGR, Color::PUR, Color::SKY},
    {Color::BGR, Color::RED, Color::SKY, Color::MIN},
    {},
    {}};

int main(int argc, char *argv[])
{
    Board board{initialBottles};
    if (!isValid(board))
    {
        std::cout << L"Board invalid!" << std::endl;
        return -1;
    }

    printBoard(getState(board));

    while (performNextMove(board))
    {
        printBoard(getState(board));
    }

    if (reachedUnknown(board))
    {
        std::wcout << L"Result: Unknown Cell" << std::endl;
        printHistory(board);
        return -1;
    }

    if (isFinished(board))
    {
        std::wcout << L"Result: Finished Board" << std::endl;
        printHistory(board);
        return 1;
    }

    std::cout << L"No result!" << std::endl;
    return 0;
}