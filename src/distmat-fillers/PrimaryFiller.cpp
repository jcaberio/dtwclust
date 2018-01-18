#include "concrete-fillers.h"

#include <cstddef>
#include <memory>

#include <RcppArmadillo.h>
#include <RcppParallel.h>

#include "../utils/utils++.h" // get_grain

namespace dtwclust {

// =================================================================================================
/* multi-threaded primary filler
 *   This filler makes no assumptions about the dimensions of *distmat_, but makes the parallel
 *   chunks based on the number of rows.
 */
// =================================================================================================

// -------------------------------------------------------------------------------------------------
/* constructor */
// -------------------------------------------------------------------------------------------------
PrimaryFiller::PrimaryFiller(std::shared_ptr<Distmat>& distmat,
                             const std::shared_ptr<DistanceCalculator>& dist_calculator,
                             const SEXP& NUM_THREADS)
    : DistmatFiller(distmat, dist_calculator, NUM_THREADS)
{ }

// -------------------------------------------------------------------------------------------------
/* parallel worker */
// -------------------------------------------------------------------------------------------------
class ParallelFillWorker : public RcppParallel::Worker {
public:
    // constructor
    ParallelFillWorker(const std::shared_ptr<DistanceCalculator>& dist_calculator,
                       const std::shared_ptr<Distmat>& distmat)
        : dist_calculator_(dist_calculator)
        , distmat_(distmat)
        , ncols_(dist_calculator_->yLimit())
    { }

    // parallel loop across specified range
    void operator()(std::size_t begin, std::size_t end) {
        // local copy of dist_calculator so it is setup separately for each thread
        mutex_.lock();
        DistanceCalculator* dist_calculator = dist_calculator_->clone();
        mutex_.unlock();
        // fill distances
        for (std::size_t i = begin; i < end; i++) {
            for (int j = 0; j < ncols_; j++) {
                (*distmat_)(i,j) = dist_calculator->calculate(i,j);
            }
        }
        // delete local calculator
        mutex_.lock();
        delete dist_calculator;
        mutex_.unlock();
    }

private:
    const std::shared_ptr<DistanceCalculator> dist_calculator_;
    std::shared_ptr<Distmat> distmat_;
    int ncols_;
    // for synchronization during memory allocation (from TinyThread++, comes with RcppParallel)
    tthread::mutex mutex_;
};

// -------------------------------------------------------------------------------------------------
/* public fill method */
// -------------------------------------------------------------------------------------------------
void PrimaryFiller::fill() const
{
    ParallelFillWorker fill_worker(dist_calculator_, distmat_);
    int size = dist_calculator_->xLimit();
    int grain = get_grain(size, num_threads_);
    RcppParallel::parallelFor(0, size, fill_worker, grain);
}

} // namespace dtwclust