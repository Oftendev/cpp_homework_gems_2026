#pragma once
#include <ctime>
#include <random>

#include "Cell.hpp"

// Класс генератора рандомных числел (синглтон)
class RandomGenerator {
   public:
    static RandomGenerator&
    getInstance();  // Метод доступа к единственному экземпляру объекта

    Color randomColor();
    int randomInRange(int a, int b);
    std::mt19937& getRng();

   private:
    std::mt19937 rng;
    RandomGenerator();  // Приватный конструктор
};