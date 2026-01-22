/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/** Implementation of k-means clustering with many variants. */

#ifndef FAISS_CLUSTERING_H
#define FAISS_CLUSTERING_H
#include <faiss/Index.h>
#include <faiss/impl/ClusteringInitialization.h>

#include <vector>

namespace faiss {

/** Class for the clustering parameters. Can be passed to the
 * constructor of the Clustering object.
 */
struct ClusteringParameters {
    /// number of clustering iterations
    int niter = 25;
    /// redo clustering this many times and keep the clusters with the best
    /// objective
    int nredo = 1;

    bool verbose = false;
    /// whether to normalize centroids after each iteration (useful for inner
    /// product clustering)
    bool spherical = false;
    /// round centroids coordinates to integer after each iteration?
    bool int_centroids = false;
    /// re-train index after each iteration?
    bool update_index = false;

    /// Use the subset of centroids provided as input and do not change them
    /// during iterations
    bool frozen_centroids = false;
    /// If fewer than this number of training vectors per centroid are provided,
    /// writes a warning. Note that fewer than 1 point per centroid raises an
    /// exception.
    int min_points_per_centroid = 39;
    /// to limit size of dataset, otherwise the training set is subsampled
    int max_points_per_centroid = 256;
    /// seed for the random number generator.
    /// negative values lead to seeding an internal rng with
    /// std::high_resolution_clock.
    int seed = 1234;

    /// when the training set is encoded, batch size of the codec decoder
    size_t decode_block_size = 32768;

    /// whether to check for NaNs in an input data
    bool check_input_data_for_NaNs = true;

    /// Whether to use splitmix64-based random number generator for subsampling,
    /// which is faster, but may pick duplicate points.
    bool use_faster_subsampling = false;

    /// Initialization method for centroids.
    /// RANDOM: uniform random sampling (default, current behavior)
    /// KMEANS_PLUS_PLUS: k-means++ (O(nkd), better quality)
    /// AFK_MC2: Assumption-Free K-MC² (O(nd) + O(mk²d), fast approximation)
    ClusteringInitMethod init_method = ClusteringInitMethod::RANDOM;

    /// Chain length for AFK-MC² initialization.
    /// Only used when init_method = AFK_MC2.
    /// Longer chains give better approximation but are slower.
    uint16_t afkmc2_chain_length = 50;
};

struct ClusteringIterationStats {
    float obj;   ///< objective values (sum of distances reported by index)
    double time; ///< seconds for iteration
    double time_search;      ///< seconds for just search
    double imbalance_factor; ///< imbalance factor of iteration
    int nsplit;              ///< number of cluster splits
};

/** K-means clustering based on assignment - centroid update iterations
 *
 * The clustering is based on an Index object that assigns training
 * points to the centroids. Therefore, at each iteration the centroids
 * are added to the index.
 *
 * On output, the centroids table is set to the latest version
 * of the centroids and they are also added to the index. If the
 * centroids table it is not empty on input, it is also used for
 * initialization.
 *
 */
struct Clustering : ClusteringParameters {
    size_t d; ///< dimension of the vectors
    size_t k; ///< nb of centroids

    /** centroids (k * d)
     * if centroids are set on input to train, they will be used as
     * initialization
     */
    std::vector<float> centroids;

    /// stats at every iteration of clustering
    std::vector<ClusteringIterationStats> iteration_stats;

    Clustering(int d, int k);
    Clustering(int d, int k, const ClusteringParameters& cp);

    /** run k-means training
     *
     * @param x          training vectors, size n * d
     * @param index      index used for assignment
     * @param x_weights  weight associated to each vector: NULL or size n
     */
    virtual void train(
            idx_t n,
            const float* x,
            faiss::Index& index,
            const float* x_weights = nullptr);

    /** run with encoded vectors
     *
     * in addition to train()'s parameters takes a codec as parameter
     * to decode the input vectors.
     *
     * @param codec      codec used to decode the vectors (nullptr =
     *                   vectors are in fact floats)
     */
    void train_encoded(
            idx_t nx,
            const uint8_t* x_in,
            const Index* codec,
            Index& index,
            const float* weights = nullptr);

    /// Post-process the centroids after each centroid update.
    /// includes optional L2 normalization and nearest integer rounding
    void post_process_centroids();

    virtual ~Clustering() {}
};

} // namespace faiss

#endif
