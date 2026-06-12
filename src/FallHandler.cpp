#include "FallHandler.hpp"

#include "CellFactory.hpp"
#include "RandomGenerator.hpp"

void FallHandler::fallAndAdd(
    std::vector<std::vector<std::shared_ptr<Cell>>>& board, int width,
    int height) {
    for (int x = 0; x < width; x++) {
        // Будем собирать по новому столбцу
        std::vector<std::shared_ptr<Cell>> newColumn;

        for (int y = height - 1; y >= 0; y--) {
            if (!board[y][x]->isEmpty()) {
                // Кладём в newColumn все клетки в столбце, кроме пустых
                newColumn.push_back(board[y][x]);
            }
        }
        // В newcolumn докладываем рандомные (не бонусные) ячеёки
        for (int i = static_cast<int>(newColumn.size()); i < height; i++) {
            newColumn.push_back(CellFactory::createNormalCell(
                RandomGenerator::getInstance().randomColor()));
        }
        // Теперь копируем из newColunm в столбец board (но надо развернуть)
        for (int y = 0; y < height; y++) {
            board[height - y - 1][x] = newColumn[y];
        }
    }
}