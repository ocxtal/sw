
/**
 * @file sw.c
 *
 * @brief full Smith-Waterman
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define _a(p, q, plen)		( (q) * ((plen) + 1) + (p) )
#define MAX2(p, q)			( ((p) < (q)) ? (q) : (p) )
#define MAX3(p, q, r)		( MAX2(p, MAX2(q, r)) )
#define MAX4(p, q, r, s)	( MAX2(MAX2(p, q), MAX2(r, s)) )

/**
 * @fn sw_linear
 */
int
sw_linear(
	char const *a,
	uint64_t alen,
	char const *b,
	uint64_t blen,
	int8_t m, int8_t x, int8_t gi, int8_t ge)
{
	/* util macros */
	#define a(p, q)		_a(p, q, alen)
	#define s(p, q)		( (a[(p) - 1] == b[(q) - 1]) ? m : x )

	int16_t *mat = (int16_t *)malloc(
		(alen + 1) * (blen + 1) * sizeof(int16_t));
	int16_t *ptr = mat;

	/* init */
	int16_t max = ptr[a(0, 0)] = 0;
	for(uint64_t i = 1; i < alen+1; i++) { ptr[a(i, 0)] = 0; }
	for(uint64_t j = 1; j < blen+1; j++) { ptr[a(0, j)] = 0; }

	for(uint64_t j = 1; j < blen+1; j++) {
		for(uint64_t i = 1; i < alen+1; i++) {
			int16_t score = ptr[a(i, j)] = MAX4(0,
				ptr[a(i - 1, j - 1)] + s(i, j),
				ptr[a(i, j - 1)] + ge,
				ptr[a(i - 1, j)] + ge);
			max = MAX2(max, score);
		}
	}
	free(mat);

	#undef a
	#undef s
	return(max);
}

/**
 * @fn sw_affine
 */
int
sw_affine(
	char const *a,
	uint64_t alen,
	char const *b,
	uint64_t blen,
	int8_t m, int8_t x, int8_t gi, int8_t ge)
{
	/* utils */
	#define a(p, q)		_a(p, 3*(q), alen)
	#define f(p, q)		_a(p, 3*(q)+1, alen)
	#define e(p, q)		_a(p, 3*(q)+2, alen)
	#define s(p, q)		( (a[(p) - 1] == b[(q) - 1]) ? m : x )

	int16_t *mat = (int16_t *)malloc(
		3 * (alen + 1) * (blen + 1) * sizeof(int16_t));
	int16_t *ptr = mat;

	/* init */
	int16_t max = ptr[a(0, 0)] = ptr[f(0, 0)] = ptr[e(0, 0)] = 0;
	for(uint64_t i = 1; i < alen+1; i++) {
		ptr[a(i, 0)] = ptr[f(i, 0)] = ptr[e(i, 0)] = 0;
	}
	for(uint64_t j = 1; j < blen+1; j++) {
		ptr[a(0, j)] = ptr[f(0, j)] = ptr[e(0, j)] = 0;
	}

	for(uint64_t j = 1; j < blen+1; j++) {
		for(uint64_t i = 1; i < alen+1; i++) {
			int16_t score_f = ptr[f(i, j)] = MAX2(
				ptr[a(i - 1, j)] + gi + ge,
				ptr[f(i - 1, j)] + ge);
			int16_t score_e = ptr[e(i, j)] = MAX2(
				ptr[a(i, j - 1)] + gi + ge,
				ptr[e(i, j - 1)] + ge);
			int16_t score = ptr[a(i, j)] = MAX4(0,
				ptr[a(i - 1, j - 1)] + s(i, j),
				score_f, score_e);
			max = MAX2(max, score);
		}
	}
	free(mat);

	#undef a
	#undef f
	#undef e
	#undef s
	return(max);
}

#ifdef TEST
#include <stdio.h>
#include <assert.h>
#define _f(f, a, b, s)		f(a, strlen(a), b, strlen(b), s[0], s[1], s[2], s[3])
#define _linear(a, b, s)	_f(sw_linear, a, b, s)
#define _affine(a, b, s)	_f(sw_affine, a, b, s)
static int8_t const t[][5] = {
	{1, -1, -1, -1},
	{1, -1, -2, -1},
	{1, -3, -5, -1},
	{2, -3, -5, -1}
};

void test_linear_1_1_1(void)
{
	#define l(p, q)		_linear(p, q, t[0])
	assert( 0 == l("", ""));
	assert( 0 == l("A", ""));
	assert( 1 == l("A", "A"));
	assert( 3 == l("AAA", "AAA"));
	assert( 0 == l("AAA", "TTT"));
	assert( 3 == l("GGGAAAGGG", "TTTTTTAAATTTTTT"));

	assert( 5 == l("GGGAAACAAAGGG", "TTTTTTTAAAAAATTTTTTT"));
	assert( 4 == l("GGGAAACCAAAGGG", "TTTTTTTAAAAAATTTTTTT"));
	#undef l
}

void test_affine_1_1_1(void)
{
	#define a(p, q)		_affine(p, q, t[0])
	assert( 0 == a("", ""));
	assert( 0 == a("A", ""));
	assert( 1 == a("A", "A"));
	assert( 3 == a("AAA", "AAA"));
	assert( 0 == a("AAA", "TTT"));
	assert( 3 == a("GGGAAAGGG", "TTTTTTAAATTTTTT"));

	assert( 4 == a("GGGAAACAAAGGG", "TTTTTTTAAAAAATTTTTTT"));
	assert( 3 == a("GGGAAACCAAAGGG", "TTTTTTTAAAAAATTTTTTT"));
	#undef a
}

int main(void)
{
	test_linear_1_1_1();
	test_affine_1_1_1();
	return(0);
}
#endif







