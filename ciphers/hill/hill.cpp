#include "hill.hpp"
#include "utils.hpp"
#include "crykit.hpp"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <random>

namespace  cipher::hill {
    Matrix<GF28> keygen(size_t n){
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<uint8_t> dist(1, 255);
        while (true) {
            Matrix<GF28> key(n, n);
            for (size_t i = 0; i < n; ++i)
                for (size_t j = 0; j < n; ++j){
                    key.at(i, j) = GF28(uint8_t(dist(rng)));
                }
            try {
                key.inv();
                return key;
            } catch (const std::runtime_error&) {
            }

    }


}
