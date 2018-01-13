#ifndef DTWCLUST_DISTMAT_FILLERS_HPP_
#define DTWCLUST_DISTMAT_FILLERS_HPP_

#include <memory> // *_ptr

#include <RcppArmadillo.h>

#include "../distance-calculators/distance-calculators.h"
#include "../distmat/distmat.h"

namespace dtwclust {

// =================================================================================================
/* DistmatFillers (base + factory) */
// =================================================================================================

// -------------------------------------------------------------------------------------------------
/* abstract distmat filler */
// -------------------------------------------------------------------------------------------------
class DistmatFiller
{
public:
    virtual ~DistmatFiller() {};
    virtual void fill() const = 0;

protected:
    DistmatFiller(std::shared_ptr<Distmat>& distmat,
                  const std::shared_ptr<DistanceCalculator>& dist_calculator,
                  const SEXP& NUM_THREADS)
        : dist_calculator_(dist_calculator)
        , distmat_(distmat)
        , num_threads_(Rcpp::as<int>(NUM_THREADS))
    { }

    std::shared_ptr<DistanceCalculator> dist_calculator_;
    std::shared_ptr<Distmat> distmat_;
    int num_threads_;
};

// -------------------------------------------------------------------------------------------------
/* concrete factory */
// -------------------------------------------------------------------------------------------------
class DistmatFillerFactory
{
public:
    std::shared_ptr<DistmatFiller> create(
            const SEXP& FILL_TYPE,
            const SEXP& NUM_THREADS,
            std::shared_ptr<Distmat>& distmat,
            const std::shared_ptr<DistanceCalculator>& dist_calculator);
};

} // namespace dtwclust

#endif // DTWCLUST_DISTMAT_FILLERS_HPP_
