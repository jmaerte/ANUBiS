//
// Created by jmaerte on 07.10.20.
//

#ifndef ANUBIS_SUPERBUILD_UTIL_HPP
#define ANUBIS_SUPERBUILD_UTIL_HPP

/**
* time in microseconds gets cast to the best unit for the magnitude.
*/
static std::pair<double, std::string> cast_time(double time) {
    if (time < 1'000) return {time, "µs"};
    else if (time < 1'000'000) return {time / 1'000, "ms"};

    time /= 1'000'000;
    if (time < 60) return {time, "s"};
    else if (time < 3'600) return {time / 60, "min"};
    return {time / 3'600, "h"};
}

#endif //ANUBIS_SUPERBUILD_UTIL_HPP
