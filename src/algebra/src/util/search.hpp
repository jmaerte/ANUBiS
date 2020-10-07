//
// Created by jmaerte on 25.03.20.
//

#ifndef ANUBIS_SUPERBUILD_SEARCH_HPP
#define ANUBIS_SUPERBUILD_SEARCH_HPP

namespace jmaerte {
    namespace algebra {
        namespace util {

            template<typename T, typename S>
            int binary_search(std::vector<T> arr, S t, int start, int end, int(*compare_to)(const T&, const S&)) {
                if (end == 0 || compare_to(arr[end - 1], t) < 0) return end;
                if (compare_to(arr[start], t) > 0) return 0;
                int min = start;
                int max = end;
                int compare = 0;
                while (min < max) {
                    int mid = (min + max)/2;
                    if ((compare = compare_to(arr[mid], t)) < 0) min = mid + 1;
                    else if (compare > 0) max = mid;
                    else return mid;
                }
                return min;
            }

            template<typename T>
            int binary_search(std::vector<T> arr, T t, int start, int end, int(*compare_to)(const T&, const T&)) {
                return binary_search<T, T>(arr, t, start, end, compare_to);
            }

            int compare_ints(const int& a, const int& b) {
                return a - b;
            }

            int binary_search_ints(std::vector<int> arr, int t, int start, int end) {
                if (end == 0 || arr[end - 1] < t) return end;
                if (arr[start] > t) return 0;
                int min = start;
                int max = end;
                while (min < max) {
                    int mid = (min + max)/2;
                    if (arr[mid] < t) min = mid + 1;
                    else if (arr[mid] > t) max = mid;
                    else return mid;
                }
                return min;
            }
        }
    }
}

#endif //ANUBIS_SUPERBUILD_SEARCH_HPP
