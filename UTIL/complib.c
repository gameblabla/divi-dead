#define SUCCESS                0
#define ERROR_BAD_INPUT       -4
#define ERROR_UNKNOWN_VERSION -5

#define cleanup(err) { error = err; goto _cleanup; }
#define put2(c) { if (ousp >= oust) cleanup(SUCCESS); *(ousp++) = c; }
#define put(c) { put2(c); text_buf[r++] = c; r &= (N - 1); }
#define get(c) { if (insp >= inst) break; c = *(insp++); }

#define N   0x1000
#define NIL N
#define MF  0x12
#define F 0x12
#define T 2

#define DECODE_CONTEXT unsigned char text_buf[N + MF - 1];
#define ENCODE_CONTEXT DECODE_CONTEXT unsigned long int textsize = 0, codesize = 0; int match_position, match_length, lson[N + 1], rson[N + 257], dad[N + 1];

#define FillTextBuffer() { int n; for (n = 0; n < (N + MF - 1); n++) text_buf[n] = 0; }
#define InitTree() { int i; for (i = N + 1; i <= N + 256; i++) rson[i] = NIL; for (i = 0; i < N; i++) dad[i] = NIL; }

#define InsertNode(r, id) { int _r = (r); \
	int _i, _p, _cmp; \
	unsigned char  *_key; \
	_cmp = 1; _key = &text_buf[_r]; _p = N + 1 + _key[0]; \
	rson[_r] = lson[_r] = NIL; match_length = 0; \
	while (1) { \
		if (_cmp >= 0) { \
			if (rson[_p] != NIL) _p = rson[_p]; \
			else { rson[_p] = _r; dad[_r] = _p; goto InsertNodeContinue##id; } \
		} else { \
			if (lson[_p] != NIL) _p = lson[_p]; \
			else { lson[_p] = _r; dad[_r] = _p; goto InsertNodeContinue##id; } \
		} \
		for (_i = 1; _i < F; _i++) if ((_cmp = _key[_i] - text_buf[_p + _i]) != 0) break; \
		if (_i > match_length) { \
			match_position = _p; \
			if ((match_length = _i) >= F) break; \
		} \
	} \
	dad[_r] = dad[_p]; lson[_r] = lson[_p]; rson[_r] = rson[_p]; \
	dad[lson[_p]] = _r; dad[rson[_p]] = _r; \
	if (rson[dad[_p]] == _p) rson[dad[_p]] = _r; else lson[dad[_p]] = _r; \
	dad[_p] = NIL; \
	InsertNodeContinue##id:; \
}

#define DeleteNode(p, id) { int _p = (p); \
	int _q; \
	if (dad[_p] == NIL) goto DeleteNodeContinue##id; \
	if (rson[_p] == NIL) _q = lson[_p]; \
	else if (lson[_p] == NIL) _q = rson[_p]; \
	else { \
		_q = lson[_p]; \
		if (rson[_q] != NIL) { \
			do { _q = rson[_q]; } while (rson[_q] != NIL); \
			rson[dad[_q]] = lson[_q]; dad[lson[_q]] = dad[_q]; \
			lson[_q] = lson[_p]; dad[lson[_p]] = _q; \
		} \
		rson[_q] = rson[_p]; dad[rson[_p]] = _q; \
	} \
	dad[_q] = dad[_p]; \
	if (rson[dad[_p]] == _p) rson[dad[_p]] = _q; else lson[dad[_p]] = _q; \
	dad[_p] = NIL; \
	DeleteNodeContinue##id:; \
}

int dv_compress(void *in, int inl, void *out, int *outl) {
	ENCODE_CONTEXT;
	unsigned char *insp, *inst, *ousp, *oust, *inspb, *insplb;
	int i, c, len, r, s, last_match_length, dup_match_length = 0, code_buf_ptr, dup_last_match_length = 0;
	unsigned char code_buf[1 + 8 * 5], mask;
	int error = SUCCESS;

	inst = (insplb = inspb = insp = (unsigned char *)in) + inl; oust = (ousp = (unsigned char *)out) + *outl;

	FillTextBuffer();
	InitTree();

	code_buf[0] = 0x00;
	code_buf_ptr = mask = 1;
	s = 0; r = N - F;

	for (len = 0; len < F; len++) { get(c); text_buf[r + len] = c; }
	if ((textsize = len) == 0) return SUCCESS;

	for (i = 1; i <= F; i++) InsertNode(r - i, 1); InsertNode(r, 2);

	do {
		if (match_length > len) match_length = len;

		if (match_length >= (inst - insp)) match_length--;
		
		if (match_length > T) {
			code_buf[code_buf_ptr++] = (unsigned char)match_position;
			code_buf[code_buf_ptr++] = (unsigned char)(((match_position >> 4) & 0xf0) | ((match_length - (T + 1)) & 0x0f));
		} else {
			code_buf[0] |= mask;
			match_length = 1;
			code_buf[code_buf_ptr++] = text_buf[r];
		}

		if ((mask <<= 1) == 0) {
			for (i = 0; i < code_buf_ptr; i++) put2(code_buf[i]);
			codesize += code_buf_ptr;
			code_buf[0] = 0x00; code_buf_ptr = mask = 1;
		}

		last_match_length = match_length;
		for (i = 0; i < last_match_length; i++) {
			get(c); DeleteNode(s, 1); text_buf[s] = c;
			if (s < F - 1) text_buf[s + N] = c;
			s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
			inspb++;
			InsertNode(r, 3);
		}

		textsize += i;

		while (i++ < last_match_length) {
			DeleteNode(s, 2); s = (s + 1) & (N - 1); r = (r + 1) & (N - 1);
			inspb++;
			if (--len) InsertNode(r, 4);
		}
	} while (len > 0);

	if (code_buf_ptr > 1) {
		for (i = 0; i < code_buf_ptr; i++) put2(code_buf[i]);
		codesize += code_buf_ptr;
	}

_cleanup:

	if (insp != inst) return ERROR_BAD_INPUT;

	*outl = ousp - (unsigned char *)out;

	return SUCCESS;
}