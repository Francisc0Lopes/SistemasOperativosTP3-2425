#include <stdio.h>
#include <stdlib.h>
#include "modules/vector.h"

int main() {
    int v[] = {10, 20, 30, 40, 50};
    size_t v_sz = sizeof(v) / sizeof(v[0]);
    int nThreads = 3;

    printf("Vetor original: ");
    for (size_t i = 0; i < v_sz; ++i) printf("%d ", v[i]);
    printf("\n");

    norm_min_max_and_classify_parallel(v, v_sz, nThreads);

    printf("Vetor classificado: ");
    for (size_t i = 0; i < v_sz; ++i) printf("%d ", v[i]);
    printf("\n");

    return 0;
}