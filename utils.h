#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <iostream>
// #include "edge.h"
// #include "cluster.h"

typedef std::vector<std::vector<int>> IndexCluster;
typedef std::vector<std::vector<float>> AngleGraph;

template <class T>
struct Pixel
{
    T r, c;
    Pixel (T r_, T c_) : r(r_), c(c_) {}
};

IndexCluster convert_color_cluster_to_index_cluster (const std::string& cc_name, int* num_clusters);

void save_index_cluster (const std::string& file_name, const IndexCluster& index_cluster);

IndexCluster load_index_cluster (const std::string& file_name);

AngleGraph load_angle_graph (const std::string& file_name);

template <class T>
void print_matrix (const std::vector<std::vector<T>>& m)
{
    int rows = m.size();
    int cols = m[0].size();

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            std::cout << m[r][c] << " ";
        }
        std::cout << std::endl;
    }
}

void save_cluster_graph (const std::string& file_name, const IndexCluster& index_cluster);

#endif // UTILS_H
