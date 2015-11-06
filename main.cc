#include "edge.h"
#include "cluster.h"
#include "utils.h"

#include <iostream>

using namespace std;

void print_edge_queue (EdgeQueue edge_queue, const ClusterPool& cp)
{
    cout << "====== print edge queue ======" << endl;
    cout << "size: " << edge_queue.size() << endl;
    while (edge_queue.size()) {
        int c1 = edge_queue.top()->edge_key.first;
        int c2 = edge_queue.top()->edge_key.second;
        // if (cp.is_valid(c1) && cp.is_valid(c2)) {
            cout << c1 << ", " << c2 << ", dis: " << edge_queue.top()->distance << endl;
        // }
        edge_queue.pop();
    }
    cout << "========= end  print =========" << endl;
}


int main ()
{
    // const string seg_name = "meanshift(20,65_segm).png";
    const string seg_name = "meanshift(15,15 100_segm)_segm.png";
    const string angle_file_name = "newit10_angles.txt";

    // for test
    // const string seg_file_name = "test_cluster.txt";
    // const string angle_file_name = "test_angle.txt";
    AngleGraph angle_graph = load_angle_graph(angle_file_name);

    int num_clusters;
    IndexCluster index_clusters = convert_color_cluster_to_index_cluster(seg_name, &num_clusters);
    cout << "number of clusters: " << num_clusters << endl;
    save_index_cluster(seg_name+"_index_8_dir.txt", index_clusters);

    // IndexCluster index_clusters = load_index_cluster(seg_name+"_index.txt");

    save_cluster_graph(seg_name+"_original_8_dir.jpg", index_clusters);

    unordered_map<int, Cluster*> id_clusters = construct_clusters (index_clusters);
    ClusterPool cluster_pool(id_clusters.size());
    // print_clusters(id_clusters);

    KeyEdgeMap key_edges = construct_edges(index_clusters);
    EdgeQueue edge_queue = construct_edge_queue(key_edges, index_clusters, angle_graph);
    cout << "num of edges: " << edge_queue.size() << endl;
    // print_edge_queue(edge_queue, cluster_pool);

    int i = 0;
    while (edge_queue.size()) {
        if(merge_cluster(cluster_pool, edge_queue, key_edges, id_clusters, index_clusters, angle_graph)) {
            i += 1;
            if (i % 20 == 0) {
                string file_name = "result_pic/"+to_string(i)+"_merge.jpg";
                save_cluster_graph(file_name, index_clusters);
                cout << "iter: " << i << endl;
                cout << "num cluster left: " << cluster_pool.num_valid_index << endl;
            }
        }
        // print_edge_queue(edge_queue, cluster_pool);
        // print_matrix<int>(index_clusters);
    }

    return 0;
}
