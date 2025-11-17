#include "SpatialGrid.h"
#include <cmath>
#include <algorithm> // For max and min 

SpatialGrid::SpatialGrid(double world_min_x_, double world_min_y_,
                         double world_max_x_, double world_max_y_,
                         double cell_size_)
    : min_x(world_min_x_), min_y(world_min_y_), max_x(world_max_x_), max_y(world_max_y_), cell_size(cell_size_) {
    nx = std::max(1, (int)std::ceil((max_x - min_x) / cell_size));
    ny = std::max(1, (int)std::ceil((max_y - min_y) / cell_size));
    grid.assign(nx * ny, std::vector<int>());
}

void SpatialGrid::pos_to_cell(double x, double y, int &ix, int &iy) const {
    ix = (int)std::floor((x - min_x) / cell_size);
    iy = (int)std::floor((y - min_y) / cell_size);
    if (ix < 0) ix = 0; if (ix >= nx) ix = nx - 1;
    if (iy < 0) iy = 0; if (iy >= ny) iy = ny - 1;
}

void SpatialGrid::insert(int id, double x, double y) {
    int ix, iy; pos_to_cell(x, y, ix, iy);
    grid[cell_index(ix, iy)].push_back(id);
}

void SpatialGrid::remove(int id, double x, double y) {
    int ix, iy; pos_to_cell(x, y, ix, iy);
    auto &bucket = grid[cell_index(ix, iy)];
    for (size_t i = 0; i < bucket.size(); ++i) {
        if (bucket[i] == id) { bucket[i] = bucket.back(); bucket.pop_back(); return; }
    }
}

void SpatialGrid::update(int id, double oldx, double oldy, double newx, double newy) {
    remove(id, oldx, oldy);
    insert(id, newx, newy);
}

std::vector<int> SpatialGrid::query_radius(double x, double y, double r) const {
    int ix0, iy0; pos_to_cell(x, y, ix0, iy0);
    int rad = (int)std::ceil(r / cell_size);
    std::vector<int> out;
    for (int dy = -rad; dy <= rad; ++dy) {
        int iy = iy0 + dy; if (iy < 0 || iy >= ny) continue;
        for (int dx = -rad; dx <= rad; ++dx) {
            int ix = ix0 + dx; if (ix < 0 || ix >= nx) continue;
            const auto &bucket = grid[cell_index(ix, iy)];
            out.insert(out.end(), bucket.begin(), bucket.end());
        }
    }
    return out;
}