#include "cluster.h"
#include <algorithm>
#include <iostream>
#include <cassert>

using namespace std;

Cluster::Cluster (int index_)
    : index(index_)
{}

void Cluster::add_pixel (const Pixel<int>& p)
{
    pixels.push_back(p);
}

void Cluster::add_pixel (const vector<Pixel<int>>& ps)
{
    pixels.insert(pixels.end(), ps.begin(), ps.end());
}

ClusterPool::ClusterPool (int num_clusters)
    : next_index(num_clusters), num_valid_index(num_clusters), valid_index(num_clusters, true)
{}

int  ClusterPool::get_next_index ()
{
    int ret = next_index;
    next_index++;
    num_valid_index++;
    valid_index.push_back(true);
    assert(next_index == valid_index.size());
    return ret;
}

void ClusterPool::set_invalid (int index)
{
    assert(index < next_index);
    // should only be set false once
    assert(valid_index[index] == true);
    num_valid_index--;
    valid_index[index] = false;
}

bool ClusterPool::is_valid (int index) const
{
    assert(index < next_index);
    return valid_index[index];
}

unordered_map<int, Cluster*> construct_clusters (const IndexCluster& index_cluster)
{
    assert(index_cluster.size() > 0);
    const int rows = index_cluster.size();
    const int cols = index_cluster[0].size();
    unordered_map<int, Cluster*> id_clusters;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int cluster_index = index_cluster[r][c];
            if (id_clusters.find(cluster_index) == id_clusters.end()) {
                id_clusters[cluster_index] = new Cluster(cluster_index);
            }
            id_clusters[cluster_index]->add_pixel(Pixel<int>(r,c));
            if (c == 0 || index_cluster[r][c] == index_cluster[r][c-1]) {
                continue;
            }

            int nbr_index = index_cluster[r][c-1];
            assert(id_clusters.find(nbr_index) != id_clusters.end());
            assert(id_clusters.find(cluster_index) != id_clusters.end());
            id_clusters[nbr_index]->nbr_indices.insert(cluster_index);
            id_clusters[cluster_index]->nbr_indices.insert(nbr_index);
        }
    }

    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows; ++r) {
            int cluster_index = index_cluster[r][c];
            assert(id_clusters.find(cluster_index) != id_clusters.end());
            if (r == 0 || index_cluster[r][c] == index_cluster[r-1][c]) {
                continue;
            }

            int nbr_index = index_cluster[r-1][c];
            id_clusters[nbr_index]->nbr_indices.insert(cluster_index);
            id_clusters[cluster_index]->nbr_indices.insert(nbr_index);
        }
    }

    return id_clusters;
}

void change_nbr_index (int self_index, int old_nbr_index, int new_nbr_index, unordered_map<int, Cluster*>& id_clusters)
{
    auto self_id_cluster = id_clusters.find(self_index);
    if (self_id_cluster == id_clusters.end()) {
        cout << self_index << ", " << old_nbr_index << endl;
    }
    assert(self_id_cluster != id_clusters.end());
    Cluster* self_cluster = self_id_cluster->second;
    // id_clusters.erase(self_id_cluster);
    assert(self_cluster->nbr_indices.erase(old_nbr_index) == 1);
    self_cluster->nbr_indices.insert(new_nbr_index);
}

void merge_cluster (ClusterPool& cluster_pool,
                    EdgeQueue& edge_queue,
                    KeyEdgeMap& key_edges,
                    unordered_map<int, Cluster*>& id_clusters,
                    IndexCluster& index_cluster,
                    const AngleGraph& angle_graph)
{
    assert(edge_queue.size());
    Edge* shortest_edge = edge_queue.top();
    edge_queue.pop();

    int cluster_index1 = shortest_edge->edge_key.first;
    int cluster_index2 = shortest_edge->edge_key.second;

    // cout << "merging edge: " << cluster_index1 << ", " << cluster_index2 << endl;

    if (!cluster_pool.is_valid(cluster_index1) || !cluster_pool.is_valid(cluster_index2)) {
        assert(key_edges.erase(shortest_edge->edge_key) == 0);
        delete shortest_edge;
        return;
    }

    cluster_pool.set_invalid(cluster_index1);
    cluster_pool.set_invalid(cluster_index2);

    auto id_cluster1 = id_clusters.find(cluster_index1);
    auto id_cluster2 = id_clusters.find(cluster_index2);
    // if (id_cluster1 == id_clusters.end() || id_cluster2 == id_clusters.end()) {
    //     cout << cluster_index1 << ", " << cluster_index2 << endl;
    //     cout << (id_cluster2 == id_clusters.end()) << endl;
    //     assert(false);
    // }

    assert(id_cluster1 != id_clusters.end() && id_cluster2 != id_clusters.end());
    Cluster* cluster1 = id_cluster1->second;
    Cluster* cluster2 = id_cluster2->second;
    id_clusters.erase(id_cluster1);
    id_clusters.erase(id_cluster2);

    assert(key_edges.erase(shortest_edge->edge_key) == 1);

    // cout << "hit" << endl;

    const int new_index = cluster_pool.get_next_index();
    Cluster* new_cluster = new Cluster(new_index);
    new_cluster->add_pixel(cluster1->pixels);
    new_cluster->add_pixel(cluster2->pixels);
    id_clusters[new_index] = new_cluster;

    // update the index_cluster
    for (int i = 0; i < new_cluster->pixels.size(); ++i) {
        const Pixel<int>& p = new_cluster->pixels[i];
        assert(index_cluster[p.r][p.c] != new_index);
        // cout << "changing: " << p.r << ", " << p.c << ", "
        //      << index_cluster[p.r][p.c] << "->" << new_index << endl;
        index_cluster[p.r][p.c] = new_index;
    }

    // handle neighbors
    auto nbr1 = cluster1->nbr_indices.begin();
    auto nbr2 = cluster2->nbr_indices.begin();
    // cout << "hit1" << endl;
    while (nbr1 != cluster1->nbr_indices.end() || nbr2 != cluster2->nbr_indices.end()) {
        Edge* new_edge = 0;
        if (nbr1 == cluster1->nbr_indices.end() || nbr2 == cluster2->nbr_indices.end() || *nbr1 != *nbr2) {
            // cout << "hit 3.1 start" << endl;
            int nbr = -1;
            int old_part_index = -1;
            assert(nbr1 != cluster1->nbr_indices.end() || nbr2 != cluster2->nbr_indices.end());
            if (nbr2 == cluster2->nbr_indices.end()
                || (nbr1 != cluster1->nbr_indices.end()
                    && nbr2 != cluster2->nbr_indices.end()
                    && *nbr1 < *nbr2)) {
                assert(nbr1 != cluster1->nbr_indices.end());
                nbr = *nbr1;
                old_part_index = cluster_index1;
                ++nbr1;
            } else if (nbr1 == cluster1->nbr_indices.end()
                       || (nbr1 != cluster1->nbr_indices.end()
                           && nbr2 != cluster2->nbr_indices.end()
                           && *nbr1 > *nbr2)) {
                assert(nbr2 != cluster2->nbr_indices.end());
                nbr = *nbr2;
                old_part_index = cluster_index2;
                ++nbr2;
            } else {
                assert(false);
            }

            // cout << "hit 3.1" << endl;

            EdgeKey old_edge_key = create_edge_key(old_part_index, nbr);
            if (old_edge_key == shortest_edge->edge_key) {
                continue;
            }
            new_cluster->nbr_indices.insert(nbr);
            // change its nbr accordingly
            change_nbr_index(nbr, old_part_index, new_index, id_clusters);
            new_edge = merge_edge(create_edge_key(new_index, nbr), &old_edge_key, 1, key_edges, index_cluster, angle_graph);
        } else {
            // cout << "hit 3.2 start" << endl;
            // *nbr1 == *nbr2, i.e. shared neighbor
            new_cluster->nbr_indices.insert(*nbr1);
            change_nbr_index(*nbr1, cluster_index1, new_index, id_clusters);
            change_nbr_index(*nbr2, cluster_index2, new_index, id_clusters);

            // cout << "hit 3.2" << endl;

            EdgeKey old_edge_keys[2] = { create_edge_key(*nbr1, cluster_index1), create_edge_key(*nbr2, cluster_index2) };
            new_edge = merge_edge(create_edge_key(new_index, *nbr1), old_edge_keys, 2, key_edges, index_cluster, angle_graph);
            ++nbr1, ++nbr2;
        }
        edge_queue.push(new_edge);
        key_edges[new_edge->edge_key] = new_edge;
        // cout << "new edge: " << new_edge->edge_key.first << ", " << new_edge->edge_key.second << endl;
    }

    delete cluster1;
    delete cluster2;
    delete shortest_edge;
}

void print_clusters (const unordered_map<int, Cluster*>& id_clusters)
{
    // print a hashmap
    cout << "======= print  cluster =======" << endl;
    for (unordered_map<int, Cluster*>::const_iterator id_cluster = id_clusters.begin(); id_cluster != id_clusters.end(); ++id_cluster) {
        cout << "cluster: " << id_cluster->first << endl;
        cout << "\tneighbors: ";
        for (set<int>::const_iterator nbr = id_cluster->second->nbr_indices.begin(); nbr != id_cluster->second->nbr_indices.end(); ++nbr) {
            cout << *nbr << ", ";
        }
        cout << endl;
    }
    cout << "========= end  print =========" << endl;
}
