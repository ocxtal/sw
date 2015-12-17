
# sw -- simple Smith-Waterman implementation

[![Build Status](https://travis-ci.org/ocxtal/sw.svg?branch=master)](https://travis-ci.org/ocxtal/sw)

## 目的

Exercise on git, github, and travis CI.

## インストール

Cコンパイラ (C99に対応したもの) と、python (2 or 3) が必要です。
	
	$ ./waf configure build
	$ ./waf install

## 使い方

	char const *seq_a = "ATACA";
	char const *seq_b = "TAC";
	sw_result_t result = sw_affine(
		seq_a, strlen(seq_a),	/* seq_a and its length */
		seq_b, strlen(seq_b),	/* seq_b and its length */
		2, -3, -5, -1);			/* match, mismatch, gapopen, gapextend */

	printf("score\t%d\npos\t(%llu, %llu)\n"
		"len\t(%llu, %llu)\npath len\t%u\npath\t%s\n",
		result.score,
		result.apos,
		result.bpos,
		result.alen,
		result.blen,
		result.path_length,
		result.path);

	free(result.path);			/* result.path must be freed with stdlib.free */
