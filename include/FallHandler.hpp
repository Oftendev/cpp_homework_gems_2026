#pragma once
#include <memory>
#include <vector>

#include "Cell.hpp"

class FallHandler {
   public:
    FallHandler() = default;

    // "Схлопывание" пустых клеток, спуск непустых вниз и заполнение новыми
    // сверху
    // Возвращает true, если были изменения
    void fallAndAdd(std::vector<std::vector<std::shared_ptr<Cell>>>& board,
                    int width, int height);
};