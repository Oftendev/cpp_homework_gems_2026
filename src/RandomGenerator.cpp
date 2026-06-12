#include "RandomGenerator.hpp"

RandomGenerator::RandomGenerator() {
    rng.seed(static_cast<unsigned int>(std::time(nullptr)));
}

RandomGenerator& RandomGenerator::getInstance() {
    static RandomGenerator
        instance;  // Едтнственный экземпляр класса, который ицициализируется
                   // при первом вызове метода и сохраняет значение между
                   // вызовами
    return instance;
}

Color RandomGenerator::randomColor() {
    std::uniform_int_distribution<int> dist(0,
                                            static_cast<int>(Color::Count) - 1);
    return static_cast<Color>(dist(rng));
}

int RandomGenerator::randomInRange(int a, int b) {
    std::uniform_int_distribution<int> dist(a, b);
    return dist(rng);
}

std::mt19937& RandomGenerator::getRng() { return rng; }