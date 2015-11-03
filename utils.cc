#include "utils.h"
#include "edge.h"
#include <opencv2/opencv.hpp>
#include <cassert>
#include <fstream>
#include <vector>
using namespace std;
using cv::Vec3b;
using cv::Mat;

bool all_assigned (const IndexCluster& index_cluster, int& r, int& c)
{
    assert(index_cluster.size() > 0);
    for (int row = 0; row < index_cluster.size(); ++row) {
        for (int col = 0; col < index_cluster[0].size(); ++col) {
            if (index_cluster[row][col] == -1) {
                r = row;
                c = col;
                return false;
            }
        }
    }
    return true;
}

void assign_cluster (int r, int c, const Mat& seg_image, int index, IndexCluster& index_cluster)
{
    assert(index >= 0);
    // cout << r << ", " << c << endl;
    static const int num_dirs = 8;
    static const int dr[num_dirs] = {-1, -1, -1,  0, 0,  1, 1, 1};
    static const int dc[num_dirs] = {-1,  0,  1, -1, 1, -1, 0, 1};
    const int& rows = seg_image.rows;
    const int& cols = seg_image.cols;
    const cv::Vec3b& center_color = seg_image.at<cv::Vec3b>(r,c);

    index_cluster[r][c] = index;

    for (int i = 0; i < num_dirs; ++i) {
        int rr = r + dr[i];
        int cc = c + dc[i];
        if (rr < 0 || rr >= rows || cc < 0 || cc >= cols) {
            continue;
        }

        const cv::Vec3b& color = seg_image.at<cv::Vec3b>(rr,cc);
        if (color == center_color && index_cluster[rr][cc] == -1) {
            assign_cluster(rr, cc, seg_image, index, index_cluster);
        }
    }
}

IndexCluster convert_color_cluster_to_index_cluster (const string& cc_name, int* num_clusters)
{
    const cv::Mat seg_image = cv::imread(cc_name, CV_LOAD_IMAGE_COLOR);
    const int rows = seg_image.rows;
    const int cols = seg_image.cols;

    int nar, nac;
    int next_cluster = 0;
    IndexCluster index_cluster(rows, vector<int>(cols, -1));

    while (!all_assigned(index_cluster, nar, nac)) {
        assign_cluster(nar, nac, seg_image, next_cluster, index_cluster);
        next_cluster++;
    }
    *num_clusters = next_cluster;
    return index_cluster;
}

void save_index_cluster (const string& file_name, const IndexCluster& index_cluster)
{
    ofstream out_file(file_name);
    assert(index_cluster.size() > 0);
    const int rows = index_cluster.size();
    const int cols = index_cluster[0].size();
    out_file << rows << " " << cols << endl;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            out_file << index_cluster[r][c] << " ";
        }
        out_file << endl;
    }
    out_file.close();
}

template <class T>
vector<vector<T>> load_matrix_from_file (const string& file_name)
{
    ifstream in_file(file_name);
    int rows, cols;
    in_file >> rows >> cols;
    assert(rows > 0 && cols > 0);
    vector<vector<T>> matrix(rows, vector<T>(cols));

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            in_file >> matrix[r][c];
        }
    }
    in_file.close();
    return matrix;
}

IndexCluster load_index_cluster (const string& file_name)
{
    return load_matrix_from_file<int>(file_name);
}

AngleGraph load_angle_graph (const string& file_name)
{
    return load_matrix_from_file<float>(file_name);
}

void save_cluster_graph (const string& file_name, const IndexCluster& index_cluster)
{
    assert(index_cluster.size() > 0);
    const int rows = index_cluster.size();
    const int cols = index_cluster[0].size();
    Mat graph(rows, cols, CV_8UC3);
    for (int r = 0; r < rows; ++r) {
        graph.at<Vec3b>(r,0) = Vec3b(255, 255, 255);
        for (int c = 1; c < cols; ++c) {
            if (index_cluster[r][c] == index_cluster[r][c-1]) {
                graph.at<Vec3b>(r,c) = Vec3b(255, 255, 255);
            } else {
                graph.at<Vec3b>(r,c) = Vec3b(0, 0, 0);
            }
        }
    }

    for (int c = 0; c < cols; ++c) {
        for (int r = 1; r < rows; ++r) {
            if (index_cluster[r][c] != index_cluster[r-1][c]) {
                graph.at<Vec3b>(r,c) = Vec3b(0, 0, 0);
            }
        }
    }
    cout << "saving: " << file_name << endl;
    assert(cv::imwrite(file_name, graph));
}
