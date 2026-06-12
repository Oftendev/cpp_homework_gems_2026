#pragma once
#include <memory>
#include <vector>

#include "Cell.hpp"

class GroupFinder {
   public:
    GroupFinder() = default;

    // Поиск всех клеток из групп из трёх и более одинаковых цветов
    std::vector<sf::Vector2i> findGroupCells(
        const std::vector<std::vector<std::shared_ptr<Cell>>>& board, int width,
        int height);
};