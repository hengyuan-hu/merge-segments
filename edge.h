#ifndef EDGE_H
#define EDGE_H

#include <vector>
#include <queue>
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include "utils.h"

typedef std::pair<int, int> EdgeKey;

class Edge
{
public:
    // Edge (int cluster1, int cluster2);
    Edge (const EdgeKey& edge_key_);
    // Edge (const EdgeKey& edge_key_, const EdgeKey& edge_key1, const EdgeKey& edge_key2);
    void add_pixel (const Pixel<float>& p);
    void add_pixel (const std::vector<Pixel<float>>& ps);
    float calc_distance (const IndexCluster& index_cluster, const AngleGraph& angle_graph);

    EdgeKey edge_key;
    float distance;
    std::vector<Pixel<float>> pixels;
};

class CompareEdges
{
public:
    bool operator() (const Edge* e1, const Edge* e2) { return e1->distance > e2->distance; }
};

typedef std::priority_queue<Edge*, std::vector<Edge*>, CompareEdges> EdgeQueue;
typedef std::unordered_map<EdgeKey, Edge*, boost::hash<EdgeKey>> KeyEdgeMap;

KeyEdgeMap construct_edges (const IndexCluster& index_cluster);

EdgeQueue construct_edge_queue (const KeyEdgeMap& key_edges,
                                const IndexCluster& index_clusters,
                                const AngleGraph& angle_graph);

Edge* merge_edge (const EdgeKey& new_key, const EdgeKey* old_keys, int num_keys,
                  KeyEdgeMap& key_edges, const IndexCluster& index_cluster,
                  const AngleGraph& angle_graph);

// #include "cluster.h"
// void print_edge_queue (EdgeQueue edge_queue, const ClusterPool& cp);

EdgeKey create_edge_key (int cluster1, int cluster2);


#endif // EDGE_H
