
/**
 * @file sw.h
 */
#ifndef _SW_H_INCLUDED
#define _SW_H_INCLUDED

#include <stdint.h>

typedef struct _sw_result {
	int score;
	uint32_t path_length;
	uint64_t apos, bpos;
	char const *path;
	int16_t *mat;
} sw_result_t;

sw_result_t sw_linear(
	char const *a,
	uint64_t alen,
	char const *b,
	uint64_t blen,
	int8_t m, int8_t x, int8_t gi, int8_t ge);

sw_result_t sw_affine(
	char const *a,
	uint64_t alen,
	char const *b,
	uint64_t blen,
	int8_t m, int8_t x, int8_t gi, int8_t ge);

void sw_linear_get_path(sw_result_t *result);
void sw_affine_get_path(sw_result_t *result);

void sw_result_free(sw_result_t *result);

#endif
/**
 * end of sw.h
 */
