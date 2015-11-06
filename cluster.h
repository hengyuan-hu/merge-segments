#ifndef CLUSTER_H
#define CLUSTER_H

#include <set>
#include <vector>
#include <unordered_map>

#include "utils.h"
#include "edge.h"

struct Cluster
{
    Cluster (int index_);
    void add_pixel (const Pixel<int>& p);
    void add_pixel (const std::vector<Pixel<int>>& ps);

    int index;
    std::set<int> nbr_indices;
    std::vector<Pixel<int>> pixels;
};

class ClusterPool
{
public:
    ClusterPool (int num_clusters);
    int get_next_index ();
    void set_invalid (int index);
    bool is_valid (int index) const;

    int next_index;
    int num_valid_index;
    std::vector<bool> valid_index;
};

std::unordered_map<int, Cluster*> construct_clusters (const IndexCluster& index_cluster);

bool merge_cluster (ClusterPool& cluster_pool, EdgeQueue& edge_queue, KeyEdgeMap& key_edges,
                    std::unordered_map<int, Cluster*>& id_clusters, IndexCluster& index_cluster,
                    const AngleGraph& angle_graph);

void print_clusters (const std::unordered_map<int, Cluster*>& id_clusters);


#endif // CLUSTER_H
