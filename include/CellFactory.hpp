#pragma once
#include <memory>

#include "Cell.hpp"
#include "ColorBonusCell.hpp"
#include "DestroyBonusCell.hpp"

class CellFactory {
   public:
    static std::shared_ptr<Cell> createNormalCell(Color color);
    static std::shared_ptr<Cell> createEmptyCell();
    static std::shared_ptr<Cell> createRandomBonus(Color color);
};