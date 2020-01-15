//
// Created by jmaerte on 09.01.20.
//

#pragma once

template<typename T, typename S>
int binary_search(std::vector<T> arr, S t, int start, int end, int(*compare_to)(const T&, const S&)) {
    if (end == 0 || compare_to(arr[end - 1], t) < 0) return end;
    if (compare_to(arr[start], t) < 0) return 0;
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