/**
 * @file mean_shift_impl.hpp
 * @author Shangtong Zhang
 *
 * Mean Shift clustering
 */

#include "mean_shift.hpp"
#include <mlpack/core/metrics/lmetric.hpp>

namespace mlpack {
namespace meanshift {
  
/**
  * Construct the Mean Shift object.
  */
template<typename MatType,
         typename MetricType>
MeanShift<
    MatType,
    MetricType>::
MeanShift(const size_t maxIterations,
          const double stopThresh,
          const double radius,
          const MetricType metric) :
    maxIterations(maxIterations),
    stopThresh(stopThresh),
    radius(radius),
    metric(metric)
{
  // Nothing to do.
}

/**
  * Perform Mean Shift clustering on the data, returning a list of cluster
  * assignments and centroids.
  */
template<typename MatType,
         typename MetricType>
inline void MeanShift<
    MatType,
    MetricType>::
Cluster(const MatType& data,
        arma::Col<size_t>& assignments,
        arma::mat& centroids) {
  
  // all centroids before remove duplicate ones.
  arma::mat allCentroids(data.n_rows, data.n_cols);
  assignments.set_size(data.n_cols);
  
  // for each point in dataset, perform mean shift algorithm.
  for (size_t i = 0; i < data.n_cols; ++i) {
    
    size_t completedIterations = 0;
    
    //initial centroid is the point itself.
    allCentroids.col(i) = data.col(i);
    
    while (true) {
      
      // mean shift vector.
      arma::Col<double> mhVector = arma::zeros(data.n_rows, 1);
      
      // number of neighbouring points.
      int vecCount = 0;
      for (size_t j = 0; j < data.n_cols; ++j) {
        
        // find neighbouring points.
        double dist = metric.Evaluate(data.col(j), allCentroids.col(i));
        if (dist < radius) {
          vecCount ++;
          
          // update mean shift vector.
          mhVector += data.col(j) - allCentroids.col(i);
        }
      }
      mhVector /= vecCount;
      
      completedIterations ++;
      // update centroid.
      allCentroids.col(i) += mhVector;
      
      if (arma::norm(mhVector, 2) < stopThresh ||
          completedIterations > maxIterations) {
        break;
      }
      
    }
    
  }
  
  // remove duplicate centroids.
  for (size_t i = 0; i < allCentroids.n_cols; ++i) {
    
    bool isDuplicated = false;
    
    /** 
     * if a centroid is a neighbouring point of existing points,
     * remove it and update corresponding assignments.
     */
    for (size_t j = 0; j < centroids.n_cols; ++j) {
      arma::Col<double> delta = allCentroids.col(i) - centroids.col(j);
      if (norm(delta, 2) < radius) {
        isDuplicated = true;
        assignments(i) = j;
        break;
      }
    }
    
    if (!isDuplicated) {
      
      // this centroid is a new centroid.
      if (centroids.n_cols == 0) {
        centroids.insert_cols(0, allCentroids.col(i));
        assignments(i) = 0;
      } else {
        centroids.insert_cols(centroids.n_cols, allCentroids.col(i));
        assignments(i) = centroids.n_cols - 1;
      }
    }
  }
  
}
  
}; // namespace meanshift
}; // namespace mlpack