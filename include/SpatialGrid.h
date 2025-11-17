#ifndef SPATIALGRID_H
#define SPATIALGRID_H

#include <vector>

class SpatialGrid {
public:
    double min_x, min_y, max_x, max_y;
    double cell_size;
    int nx, ny;
    std::vector<std::vector<int>> grid;

    SpatialGrid(double world_min_x, double world_min_y, double world_max_x, double world_max_y, double cell_size);
    
    inline int cell_index(int ix, int iy) const { return iy * nx + ix; }
    void pos_to_cell(double x, double y, int &ix, int &iy) const;
    void insert(int id, double x, double y);
    void remove(int id, double x, double y);
    void update(int id, double oldx, double oldy, double newx, double newy);
    std::vector<int> query_radius(double x, double y, double r) const;
};

#endif