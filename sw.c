
/**
 * @file sw.c
 *
 * @brief full Smith-Waterman
 */
#include "sw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define _a(p, q, plen)		( (q) * ((plen) + 1) + (p) )
#define MAX2(p, q)			( ((p) < (q)) ? (q) : (p) )
#define MAX3(p, q, r)		( MAX2(p, MAX2(q, r)) )
#define MAX4(p, q, r, s)	( MAX2(MAX2(p, q), MAX2(r, s)) )

typedef struct _maxpos {
	int16_t score;
	uint64_t apos;
	uint64_t bpos;
} maxpos_t;

/**
 * @fn sw_linear
 */
sw_result_t sw_linear(
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

	/* init */
	maxpos_t max = { 0, 0, 0 };
	mat[a(0, 0)] = 0;
	for(uint64_t i = 1; i < alen+1; i++) { mat[a(i, 0)] = 0; }
	for(uint64_t j = 1; j < blen+1; j++) { mat[a(0, j)] = 0; }

	for(uint64_t j = 1; j < blen+1; j++) {
		for(uint64_t i = 1; i < alen+1; i++) {
			int16_t score = mat[a(i, j)] = MAX4(0,
				mat[a(i - 1, j - 1)] + s(i, j),
				mat[a(i, j - 1)] + ge,
				mat[a(i - 1, j)] + ge);
			if(score >= max.score) { max = (maxpos_t){ score, i, j }; }
		}
	}
	if(max.score == 0) { max = (maxpos_t){ 0, 0, 0 }; }

	sw_result_t result = {
		.score = max.score,
		.apos = max.apos,
		.bpos = max.bpos,
		.path_length = max.apos + max.bpos + 1,
		.path = malloc(max.apos + max.bpos + 1)
	};
	uint32_t path_index = max.apos + max.bpos + 1;
	while(result.apos != 0 && result.bpos != 0) {
		if(mat[a(result.apos, result.bpos)] == mat[a(result.apos, result.bpos - 1)] + ge) {
			result.bpos--;
			result.path[--path_index] = 'I';
		} else if(mat[a(result.apos, result.bpos)] == mat[a(result.apos - 1, result.bpos)] + ge) {
			result.apos--;
			result.path[--path_index] = 'D';
		} else {
			if(mat[a(result.apos, result.bpos)] == 0) {
				break;
			}
			if(mat[a(result.apos, result.bpos)] == mat[a(result.apos - 1, result.bpos - 1)] + x) {
				result.path[--path_index] = 'X';
			} else {
				result.path[--path_index] = 'M';
			}
			result.apos--;
			result.bpos--;
		}
	}

	result.alen = max.apos - result.apos;
	result.blen = max.bpos - result.bpos;
	result.path_length -= path_index;
	for(uint64_t i = 0; i < result.path_length; i++) {
		result.path[i] = result.path[path_index++];
	}
	result.path[result.path_length] = '\0';

	free(mat);

	#undef a
	#undef s
	return(result);
}

/**
 * @fn sw_affine
 */
sw_result_t sw_affine(
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

	/* init */
	maxpos_t max = { 0, 0, 0 };
	mat[a(0, 0)] = mat[f(0, 0)] = mat[e(0, 0)] = 0;
	for(uint64_t i = 1; i < alen+1; i++) {
		mat[a(i, 0)] = mat[f(i, 0)] = mat[e(i, 0)] = 0;
	}
	for(uint64_t j = 1; j < blen+1; j++) {
		mat[a(0, j)] = mat[f(0, j)] = mat[e(0, j)] = 0;
	}

	for(uint64_t j = 1; j < blen+1; j++) {
		for(uint64_t i = 1; i < alen+1; i++) {
			int16_t score_f = mat[f(i, j)] = MAX2(
				mat[a(i - 1, j)] + gi + ge,
				mat[f(i - 1, j)] + ge);
			int16_t score_e = mat[e(i, j)] = MAX2(
				mat[a(i, j - 1)] + gi + ge,
				mat[e(i, j - 1)] + ge);
			int16_t score = mat[a(i, j)] = MAX4(0,
				mat[a(i - 1, j - 1)] + s(i, j),
				score_f, score_e);
			if(score >= max.score) { max = (maxpos_t){ score, i, j }; }
		}
	}
	if(max.score == 0) { max = (maxpos_t){ 0, 0, 0 }; }

	sw_result_t result = {
		.score = max.score,
		.apos = max.apos,
		.bpos = max.bpos,
		.path_length = max.apos + max.bpos + 1,
		.path = malloc(max.apos + max.bpos + 1)
	};
	uint32_t path_index = max.apos + max.bpos + 1;
	while(result.apos != 0 && result.bpos != 0) {
		if(mat[a(result.apos, result.bpos)] == mat[e(result.apos, result.bpos)]) {
			while(mat[e(result.apos, result.bpos)] == mat[e(result.apos, result.bpos - 1)] + ge) {
				result.bpos--;
				result.path[--path_index] = 'I';
			}
			result.bpos--;
			result.path[--path_index] = 'I';
		} else if(mat[a(result.apos, result.bpos)] == mat[f(result.apos, result.bpos)]) {
			while(mat[f(result.apos, result.bpos)] == mat[f(result.apos - 1, result.bpos)] + ge) {
				result.apos--;
				result.path[--path_index] = 'D';
			}
			result.apos--;
			result.path[--path_index] = 'D';
		} else {
			if(mat[a(result.apos, result.bpos)] == 0) {
				break;
			}
			if(mat[a(result.apos, result.bpos)] == mat[a(result.apos - 1, result.bpos - 1)] + x) {
				result.path[--path_index] = 'X';
			} else {
				result.path[--path_index] = 'M';
			}
			result.apos--;
			result.bpos--;
		}
	}

	result.alen = max.apos - result.apos;
	result.blen = max.bpos - result.bpos;
	result.path_length -= path_index;
	for(uint64_t i = 0; i < result.path_length; i++) {
		result.path[i] = result.path[path_index++];
	}
	result.path[result.path_length] = '\0';

	free(mat);

	#undef a
	#undef f
	#undef e
	#undef s
	return(result);
}

#ifdef MAIN
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
void print_help(void)
{
	fprintf(stderr,
		"\n"
		"  sw -- simple Smith-Waterman\n"
		"\n"
		"  usage:\n"
		"    $ sw -m 2 -x -3 -o -5 -e -1 TACTA CT\n"
		"\n");
	return;
}

int main(int argc, char *argv[])
{
	int c;
	int8_t t[5] = {2, -3, -5, -1};
	enum _mode { SCORE, PATH, ALL } mode = ALL;

	while((c = getopt(argc, argv, "m:x:o:e:hspa")) != -1) {
		switch(c) {
			case 'm': t[0] = atoi(optarg); break;
			case 'x': t[1] = atoi(optarg); break;
			case 'o': t[2] = atoi(optarg); break;
			case 'e': t[3] = atoi(optarg); break;
			case 'h': print_help(); exit(1);
			case 's': mode = SCORE; break;
			case 'p': mode = PATH; break;
			case 'a': mode = ALL; break;
			default:
				fprintf(stderr, "[error] invalid option flag.\n");
				exit(1);
		}
	}

	if(argc - optind != 2) {
		fprintf(stderr, "[error] two sequences must be passed as argument.\n");
		print_help();
		exit(1);
	}

	sw_result_t result = sw_affine(
		argv[optind], strlen(argv[optind]),
		argv[optind+1], strlen(argv[optind+1]),
		t[0], t[1], t[2], t[3]);

	switch(mode) {
		case SCORE: printf("%d\n", result.score); break;
		case PATH: printf("%s\n", result.path); break;
		case ALL:
			printf("score\t%d\npos\t(%llu, %llu)\n"
				"len\t(%llu, %llu)\npath len\t%u\npath\t%s\n",
				result.score,
				result.apos,
				result.bpos,
				result.alen,
				result.blen,
				result.path_length,
				result.path);
			break;
		default: break;
	}

	free(result.path);
	return(0);
}
#endif

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
	#define l(s, ap, bp, al, bl, aln, p, q) { \
		sw_result_t r = _linear(p, q, t[0]); \
		assert(r.score == (s)); \
		assert(r.apos == (ap)); \
		assert(r.bpos == (bp)); \
		assert(r.alen == (al)); \
		assert(r.blen == (bl)); \
		assert(r.path_length == strlen(aln)); \
		assert(strcmp(r.path, aln) == 0); \
		if(r.path != NULL) { free(r.path); } \
	}
	l( 0,  0,  0,  0,  0, "", "", "");
	l( 0,  0,  0,  0,  0, "", "A", "");
	l( 1,  0,  0,  1,  1, "M", "A", "A");
	l( 3,  0,  0,  3,  3, "MMM", "AAA", "AAA");
	l( 0,  0,  0,  0,  0, "", "AAA", "TTT");
	l( 3,  3,  6,  3,  3, "MMM", "GGGAAAGGG", "TTTTTTAAATTTTTT");
	l( 4,  8, 11,  4,  4, "MMMM", "TTTGGGGGAAAA", "TTTCCCCCCCCAAAA");

	l( 5,  3,  7,  7,  6, "MMMDMMM", "GGGAAACAAAGGG", "TTTTTTTAAAAAATTTTTTT");
	l( 4,  3,  7,  8,  6, "MMMDDMMM", "GGGAAACCAAAGGG", "TTTTTTTAAAAAATTTTTTT");
	#undef l
}

void test_affine_1_1_1(void)
{
	#define a(s, ap, bp, al, bl, aln, p, q) { \
		sw_result_t r = _affine(p, q, t[0]); \
		assert(r.score == (s)); \
		assert(r.apos == (ap)); \
		assert(r.bpos == (bp)); \
		assert(r.alen == (al)); \
		assert(r.blen == (bl)); \
		assert(r.path_length == strlen(aln)); \
		assert(strcmp(r.path, aln) == 0); \
		if(r.path != NULL) { free(r.path); } \
	}
	a( 0,  0,  0,  0,  0, "", "", "");
	a( 0,  0,  0,  0,  0, "", "A", "");
	a( 1,  0,  0,  1,  1, "M", "A", "A");
	a( 3,  0,  0,  3,  3, "MMM", "AAA", "AAA");
	a( 0,  0,  0,  0,  0, "", "AAA", "TTT");
	a( 3,  3,  6,  3,  3, "MMM", "GGGAAAGGG", "TTTTTTAAATTTTTT");

	a( 4,  3,  7,  7,  6, "MMMDMMM", "GGGAAACAAAGGG", "TTTTTTTAAAAAATTTTTTT");
	a( 3,  3,  7,  8,  6, "MMMDDMMM", "GGGAAACCAAAGGG", "TTTTTTTAAAAAATTTTTTT");
	#undef a
}

int main(void)
{
	test_linear_1_1_1();
	test_affine_1_1_1();
	return(0);
}
#endif

/**
 * end of sw.c
 */
