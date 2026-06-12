#include "GroupFinder.hpp"

#include <queue>

std::vector<sf::Vector2i> GroupFinder::findGroupCells(
    const std::vector<std::vector<std::shared_ptr<Cell>>>& board, int width,
    int height) {
    std::vector<sf::Vector2i> groupCells;
    std::vector<std::vector<bool>> visited(height,
                                           std::vector<bool>(width, false));

    // Направления обхода - (x_d, y_d)
    const int x_d[4] = {1, -1, 0, 0};
    const int y_d[4] = {0, 0, 1, -1};

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Ищем только непосещённые нормальные клетки
            if (!visited[y][x] && board[y][x]->type == CellType::Normal) {
                Color currentColor = board[y][x]->color;
                std::vector<sf::Vector2i> component;
                std::queue<sf::Vector2i> q;  // Очередб для BFS
                // Начинаем с левой верхней клетки
                q.push({x, y});
                visited[y][x] = true;

                while (!q.empty()) {
                    // Извлекаем клетку из очереди
                    sf::Vector2i cur = q.front();
                    q.pop();
                    component.push_back(cur);

                    for (int dir = 0; dir < 4; dir++) {
                        int neighbor_x = cur.x + x_d[dir];
                        int neighbor_y = cur.y + y_d[dir];
                        // Проверяем соседние клетки (neighbor_x, neighbor_y):
                        if (neighbor_x >= 0 && neighbor_x < width &&
                            neighbor_y >= 0 && neighbor_y < height) {
                            // Проверяем границы
                            if (!visited[neighbor_y][neighbor_x]) {
                                // Проверяем непосещённые клетки
                                if (board[neighbor_y][neighbor_x]->type ==
                                        CellType::Normal &&
                                    board[neighbor_y][neighbor_x]->color ==
                                        currentColor) {
                                    // Проверяем, что тип и цвет совпадают
                                    visited[neighbor_y][neighbor_x] = true;
                                    q.push({neighbor_x, neighbor_y});
                                    // Добаввили вершину в очередь
                                }
                            }
                        }
                    }
                }

                // Если компонента достаточно большая, добавляем её в результат
                if (component.size() >= 3) {
                    groupCells.insert(groupCells.end(), component.begin(),
                                      component.end());
                }
            }
        }
    }

    return groupCells;
}