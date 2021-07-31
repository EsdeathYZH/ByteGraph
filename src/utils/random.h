#ifndef BYTEGRAPH_UTILS_RANDOM_H_
#define BYTEGRAPH_UTILS_RANDOM_H_

#include <random>

namespace {
  static thread_local std::default_random_engine e(time(0));
  static thread_local std::uniform_real_distribution<double> u(0., 1.);
} // namespace

static double ThreadLocalRandom() {
  return u(e);
}

/*!
 * \brief Thread-local Random Number Generator class
 */
class RandomEngine {
 public:
    /*! \brief Constructor with default seed */
    explicit RandomEngine(uint32_t tid) {
        std::random_device rd;
        SetSeed(rd() + tid);
    }

    /*! \brief Constructor with given seed */
    explicit RandomEngine(uint32_t tid, uint32_t seed) {
        SetSeed(seed + tid);
    }

    /*!
    * \brief Set the seed of this random number generator
    */
    void SetSeed(uint32_t seed) {
        rng_.seed(seed);
    }

    /*!
    * \brief Generate a uniform random integer in [0, upper)
    */
    template<typename T>
    T RandInt(T upper) {
        return RandInt<T>(0, upper);
    }

    /*!
    * \brief Generate a uniform random integer in [lower, upper)
    */
    template<typename T>
    T RandInt(T lower, T upper) {
        std::uniform_int_distribution<T> dist(lower, upper - 1);
        return dist(rng_);
    }

    /*!
    * \brief Generate a uniform random float in [0, 1)
    */
    template<typename T>
    T Uniform() {
        return Uniform<T>(0., 1.);
    }

    /*!
    * \brief Generate a uniform random float in [lower, upper)
    */
    template<typename T>
    T Uniform(T lower, T upper) {
        std::uniform_real_distribution<T> dist(lower, upper);
        return dist(rng_);
    }

private:
    std::mt19937 rng_;
};

#endif
