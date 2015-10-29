#include "edge.h"
// #include "cluster.h"
#include <cmath>
#include <cassert>
#include <iostream>

using namespace std;

const float eps = 0.0001f;

EdgeKey create_edge_key(int cluster1, int cluster2)
{
    if (cluster1 > cluster2) {
        int temp = cluster1;
        cluster1 = cluster2;
        cluster2 = temp;
    }
    assert(cluster1 < cluster2);
    return EdgeKey(cluster1, cluster2);
}

// Edge::Edge (int cluster1, int cluster2)
//     : edge_key(create_edge_key(cluster1, cluster2))
//     , distance(0.0f)
// {}

Edge::Edge (const EdgeKey& edge_key_)
    : edge_key(edge_key_)
    , distance(0.0f)
{}

Edge* merge_edge (const EdgeKey& new_key, const EdgeKey* old_keys, int num_keys,
                  KeyEdgeMap& key_edges, const IndexCluster& index_cluster,
                  const AngleGraph& angle_graph)
{
    Edge* new_edge = new Edge(new_key);

    assert(num_keys > 0);
    for (int i = 0; i < num_keys; ++i) {
        auto old_key_edge = key_edges.find(old_keys[i]);
        assert(old_key_edge != key_edges.end());
        new_edge->pixels.insert(new_edge->pixels.end(),
                                old_key_edge->second->pixels.begin(),
                                old_key_edge->second->pixels.end());
        key_edges.erase(old_key_edge);
    }
    new_edge->calc_distance(index_cluster, angle_graph);
    return new_edge;
}

void Edge::add_pixel (const Pixel<float>& p)
{
    pixels.push_back(p);
}

void Edge::add_pixel (const vector<Pixel<float>>& ps)
{
    pixels.insert(pixels.end(), ps.begin(), ps.end());
}

template <class T>
float spatial_distance (const Pixel<T>& p1, const Pixel<T>& p2)
{
    return sqrt(pow(p1.r-p2.r, 2) + pow(p1.c-p2.c, 2));
}

void get_margins (vector<Pixel<int>>& margin1, vector<Pixel<int>>& margin2,
                  int cluster1, int cluster2,
                  const Pixel<float>& p, float max_dis,
                  const IndexCluster& index_cluster)
{
    assert(index_cluster.size() > 0);
    const int rows = index_cluster.size();
    const int cols = index_cluster[0].size();

    float min_r, min_c, max_r, max_c;
    if (p.r - int(p.r) == 0.5f) {
        assert(abs(p.c - int(p.c)) < eps);

        min_r = max(0.5f, p.r - max_dis + 1.0f);
        max_r = min(rows-0.5f, p.r + max_dis - 1.0f);
        min_c = max(0.5f, p.c - max_dis + 0.5f);
        max_c = min(cols-0.5f, p.c + max_dis - 0.5f);
    } else {
        assert(abs(p.c - int(p.c) - 0.5f) < eps);

        min_r = max(0.5f, p.r - max_dis + 0.5f);
        max_r = min(rows-0.5f, p.r + max_dis - 0.5f);
        min_c = max(0.5f, p.c - max_dis + 1.0f);
        max_c = min(cols-0.5f, p.c + max_dis - 1.0f);
    }

    for (float r = min_r; r <= max_r; r += 1.0f) {
        for (float c = min_c; c <= max_c; c += 1.0f) {
            int index_r = int(r);
            int index_c = int(c);
            if (spatial_distance(Pixel<float>(r,c), p) > max_dis) {
                continue;
            }
            if (index_cluster[index_r][index_c] == cluster1) {
                margin1.push_back(Pixel<int>(index_r, index_c));
            } else if (index_cluster[index_r][index_c] == cluster2) {
                margin2.push_back(Pixel<int>(index_r, index_c));
            } else {}
        }
    }
}

float distance_of_margins (const vector<Pixel<int>>& m1, const vector<Pixel<int>>& m2, const AngleGraph& angle_graph)
{
    float s1 = 0.0f;
    float s2 = 0.0f;

    for (int i = 0; i < m1.size(); ++i) {
        const Pixel<int>& p = m1[i];
        s1 += angle_graph[p.r][p.c];
    }

    for (int i = 0; i < m2.size(); ++i) {
        const Pixel<int>& p = m2[i];
        s2 += angle_graph[p.r][p.c];
    }

    return abs(s1-s2);
}

float Edge::calc_distance (const IndexCluster& index_cluster, const AngleGraph& angle_graph)
{
    const float max_dis = 3.0f;
    distance = 0;
    for (int i = 0; i < pixels.size(); ++i) {
        vector<Pixel<int>> margin1;
        vector<Pixel<int>> margin2;
        get_margins(margin1, margin2, edge_key.first, edge_key.second, pixels[i], max_dis, index_cluster);

        distance += distance_of_margins(margin1, margin2, angle_graph);
    }
    return distance;
}

KeyEdgeMap construct_edges (const IndexCluster& index_cluster)
{
    assert(index_cluster.size() > 0);
    const int rows = index_cluster.size();
    const int cols = index_cluster[0].size();
    KeyEdgeMap key_edges;

    for (int r = 0; r < rows; ++r) {
        for (int c = 1; c < cols; ++c) {
            const int cluster1 = index_cluster[r][c-1];
            const int cluster2 = index_cluster[r][c];
            if (cluster1 == cluster2) {
                continue;
            }
            EdgeKey edge_key = create_edge_key(cluster1, cluster2);
            if (key_edges.find(edge_key) == key_edges.end()) {
                key_edges[edge_key] = new Edge(edge_key);
            }
            key_edges[edge_key]->add_pixel(Pixel<float>(float(r)+0.5f, float(c)));
        }
    }

    for (int c = 0; c < cols; ++c) {
        for (int r = 1; r < rows; ++r) {
            const int cluster1 = index_cluster[r-1][c];
            const int cluster2 = index_cluster[r][c];
            if (cluster1 == cluster2) {
                continue;
            }
            EdgeKey edge_key = create_edge_key(cluster1, cluster2);
            if (key_edges.find(edge_key) == key_edges.end()) {
                key_edges[edge_key] = new Edge(edge_key);
            }
            key_edges[edge_key]->add_pixel(Pixel<float>(float(r), float(c)+0.5));
        }
    }

    return key_edges;
}

EdgeQueue construct_edge_queue (const KeyEdgeMap& key_edges,
                                const IndexCluster& index_clusters,
                                const AngleGraph& angle_graph)
{
    EdgeQueue edge_queue;
    for (auto ke = key_edges.begin(); ke != key_edges.end(); ++ke) {
        ke->second->calc_distance(index_clusters, angle_graph);
        edge_queue.push(ke->second);
    }

    return edge_queue;
}
