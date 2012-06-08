/*
 * Copyright (c) 2000-2010, Yandex
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 * You should have received a copy of the GNU Lesser Public License
 * along with Pire.  If not, see <http://www.gnu.org/licenses>.
 */


#ifndef PIRE_STUB_UNIDATA_H_H
#define PIRE_STUB_UNIDATA_H_H



enum WC_TYPE {
	// Category           // DefaultChar
	Lu_UPPER     =  1, // 'Ъ'
	Ll_LOWER     =  2, // 'ъ'
	Lt_TITLE     =  3, // 'Ъ'
	Lm_EXTENDER  =  4, // '-'
	Lm_LETTER    =  5, // 'ъ'
	Lo_OTHER     =  6, // '?'
	Lo_IDEOGRAPH =  7, // '?'
	Lo_KATAKANA  =  8, // '?'
	Lo_HIRAGANA  =  9, // '?'
	Lo_LEADING   = 10, // '?'
	Lo_VOWEL     = 11, // '?'
	Lo_TRAILING  = 12, // '?'

	Mn_NONSPACING= 13, // '`'
	Me_ENCLOSING = 14, // '`'
	Mc_SPACING   = 15, // '`'

	Nd_DIGIT     = 16, // '9'           // convert to digit
	Nl_LETTER    = 17, // 'X'           // X,V,C,L,I ...
	Nl_IDEOGRAPH = 18, // '?'
	No_OTHER     = 19, // '9'

	Zs_SPACE     = 20, // ' ' [\40\240] SPACE ... NO-BREAK SPACE (00A0)
	Zs_ZWSPACE   = 21, // ' '           // nothing ?
	Zl_LINE      = 22, // '\n'
	Zp_PARAGRAPH = 23, // '\n'

	Cc_ASCII     = 24, // '\x1A'        // can not happen
	Cc_SPACE     = 25, // '\x1A'        // can not happen
	Cc_SEPARATOR = 26, // '\x1A'        // can not happen

	Cf_FORMAT    = 27, // '\x1A'        // nothing ?
	Cf_JOIN      = 28, // '\x1A'        // nothing ?
	Cf_BIDI      = 29, // '\x1A'        // nothing ?
	Cf_ZWNBSP    = 30, // '\x1A'        // nothing ?

	Cn_UNASSIGNED=  0, // '?'
	Co_PRIVATE   =  0, // '?'
	Cs_LOW       = 31, // '?'
	Cs_HIGH      = 32, // '?'

	Pd_DASH      = 33, // '-'
	Pd_HYPHEN    = 34, // '-' [-]       HYPHEN-MINUS
	Ps_START     = 35, // '(' [([{]     LEFT PARENTHESIS ... LEFT CURLY BRACKET
	Ps_QUOTE     = 36, // '"'
	Pe_END       = 37, // ')' [)]}]     RIGHT PARENTHESIS ... RIGHT CURLY BRACKET
	Pe_QUOTE     = 38, // '"'
	Pi_QUOTE     = 39, // '"'
	Pf_QUOTE     = 40, // '"'
	Pc_CONNECTOR = 41, // '_' [_]       LOW LINE
	Po_OTHER     = 42, // '*' [#%&*/@\] NUMBER SIGN ... REVERSE SOLIDUS
	Po_QUOTE     = 43, // '"' ["]       QUOTATION MARK
	Po_TERMINAL  = 44, // '.' [!,.:;?]  EXCLAMATION MARK ... QUESTION MARK
	Po_EXTENDER  = 45, // '-' [№]       MIDDLE DOT (00B7)
	Po_HYPHEN    = 46, // '-'

	Sm_MATH      = 47, // '=' [+<=>|~]  PLUS SIGN ... TILDE
	Sm_MINUS     = 48, // '-'
	Sc_CURRENCY  = 49, // '$' [$]       DOLLAR SIGN
	Sk_MODIFIER  = 50, // '`' [^`]      CIRCUMFLEX ACCENT ... GRAVE ACCENT
	So_OTHER     = 51, // '°' [°]       DEGREE SIGN (00B0)

	Ps_SINGLE_QUOTE = 52, // '\'' [']   OPENING SINGLE QUOTE
	Pe_SINGLE_QUOTE = 53, // '\'' [']   CLOSING SINGLE QUOTE
	Pi_SINGLE_QUOTE = 54, // '\'' [']   INITIAL SINGLE QUOTE
	Pf_SINGLE_QUOTE = 55, // '\'' [']   FINAL SINGLE QUOTE
	Po_SINGLE_QUOTE = 56, // '\'' [']   APOSTROPHE and PRIME

	CCL_NUM      = 57,
	CCL_MASK     = 0x3F,

	TO_LOWER     = 1<< 6,
	TO_UPPER     = 1<< 7,
	TO_TITLE     = 1<< 8,

	IS_XDIGIT    = 1<< 9,
	IS_DIGIT     = 1<<10,
	IS_NONBREAK  = 1<<11,

	IS_PRIVATE   = 1<<12,
	IS_ORDERED   = 1<<13,

	IS_COMPAT    = 1<<14,
	IS_CANON     = 1<<15,

	BIDI_OFFSET   =  16,
	SVAL_OFFSET   =  22,
};

const size_t DEFCHAR_BUF = 58; // CCL_NUM + 1

extern const ui32 unicode_types[];
extern const wchar32 decomp_mapping[];
extern const ui32 *unicode_pages[];

extern const unsigned DECOMP_OFFSET;
extern const unsigned DECOMP_MASK;
extern const unsigned LENGTH_OFFSET;
extern const unsigned LENGTH_MASK;
extern const unsigned TYPES_OFFSET;
extern const unsigned TYPES_MASK;

#define _(i) (ULL(1)<<(i))

ui32 _runeinfo(wchar32 ch)
{
	if (ch > 0xFFFF)
		return _runeinfo(0xE001);//as characters from Private Use Zone
	return unicode_pages[(ch>>5)&0x7FF][ch&0x1F];
}
ui32 wc_info(wchar32 ch)
{
	return unicode_types[(_runeinfo(ch)>>TYPES_OFFSET) & TYPES_MASK];
}
WC_TYPE wc_type(wchar32 ch)
{
	return (WC_TYPE)(wc_info(ch) & CCL_MASK);
}
unsigned get_decomp_mapping(wchar32 ch, const wchar32 *&decomp_p, unsigned &decomp_len)
{
	ui32 info = _runeinfo(ch);
	decomp_len = (info>>LENGTH_OFFSET)&LENGTH_MASK;
	decomp_p = &decomp_mapping[(info>>DECOMP_OFFSET) & DECOMP_MASK];
	return decomp_len;
}
bool wc_istype(wchar32 ch, ui64 type_bits)
{
	return (_(wc_type(ch)) & type_bits) != 0;
}

// all usefull properties

bool is_unicode_space(wchar32 ch) // is_space without \n,\r,\v,\f,\40,\t
{
	return wc_istype(ch, _(Zs_SPACE)|_(Zs_ZWSPACE)|_(Zl_LINE)|_(Zp_PARAGRAPH));
}
bool is_whitespace(wchar32 ch)
{
	return wc_istype(ch, _(Cc_SPACE)|_(Zs_SPACE)|_(Zs_ZWSPACE)|_(Zl_LINE)|_(Zp_PARAGRAPH));
}
bool is_ascii_cntrl(wchar32 ch)
{
	return wc_istype(ch, _(Cc_ASCII)|_(Cc_SPACE)|_(Cc_SEPARATOR));
}
bool is_bidi_cntrl(wchar32 ch)
{
	return wc_istype(ch, _(Cf_BIDI));
}
bool is_join_cntrl(wchar32 ch)
{
	return wc_istype(ch, _(Cf_JOIN));
}
bool is_format_cntrl(wchar32 ch)
{
	return wc_istype(ch, _(Cf_FORMAT));
}
bool is_ignorable_cntrl(wchar32 ch)
{
	return wc_istype(ch, _(Cf_FORMAT)|_(Cf_JOIN)|_(Cf_BIDI)|_(Cf_ZWNBSP));
}
bool is_cntrl(wchar32 ch)
{
	return wc_istype(ch,
		_(Cf_FORMAT)|_(Cf_JOIN)|_(Cf_BIDI)|_(Cf_ZWNBSP)|
		_(Cc_ASCII)|_(Cc_SPACE)|_(Cc_SEPARATOR)
	);
}
bool is_zerowidth(wchar32 ch)
{
	return wc_istype(ch, _(Cf_FORMAT)|_(Cf_JOIN)|_(Cf_BIDI)|_(Cf_ZWNBSP)|_(Zs_ZWSPACE));
}
bool is_line_sep(wchar32 ch)
{
	return wc_istype(ch, _(Zl_LINE));
}
bool is_para_sep(wchar32 ch)
{
	return wc_istype(ch, _(Zp_PARAGRAPH));
}
bool is_dash(wchar32 ch)
{
	return wc_istype(ch, _(Pd_DASH)|_(Pd_HYPHEN)|_(Sm_MINUS));
}
bool is_hyphen(wchar32 ch)
{
	return wc_istype(ch, _(Pd_HYPHEN)|_(Po_HYPHEN));
}
bool is_quotation(wchar32 ch)
{
	return wc_istype(ch, _(Po_QUOTE)|_(Ps_QUOTE)|_(Pe_QUOTE)|_(Pi_QUOTE)|_(Pf_QUOTE)|
		_(Po_SINGLE_QUOTE)|_(Ps_SINGLE_QUOTE)|_(Pe_SINGLE_QUOTE)|_(Pi_SINGLE_QUOTE)|_(Pf_SINGLE_QUOTE));

}
bool is_terminal(wchar32 ch)
{
	return wc_istype(ch, _(Po_TERMINAL));
}
bool is_paired_punct(wchar32 ch)
{
	return wc_istype(ch, _(Ps_START)|_(Pe_END) |
		_(Ps_QUOTE)|_(Pe_QUOTE)|_(Pi_QUOTE)|_(Pf_QUOTE)|
		_(Ps_SINGLE_QUOTE)|_(Pe_SINGLE_QUOTE)|_(Pi_SINGLE_QUOTE)|_(Pf_SINGLE_QUOTE));
}
bool is_left_punct(wchar32 ch)
{
	return wc_istype(ch, _(Ps_START)|_(Ps_QUOTE)|_(Ps_SINGLE_QUOTE));
}
bool is_right_punct(wchar32 ch)
{
	return wc_istype(ch, _(Pe_END)|_(Pe_QUOTE)|_(Pe_SINGLE_QUOTE));
}
bool is_combining(wchar32 ch)
{
	return wc_istype(ch, _(Mc_SPACING)|_(Mn_NONSPACING)|_(Me_ENCLOSING));
}
bool is_nonspacing(wchar32 ch)
{
	return wc_istype(ch, _(Mn_NONSPACING)|_(Me_ENCLOSING));
}
bool is_alphabetic(wchar32 ch)
{
	return wc_istype(ch, _(Lu_UPPER)|_(Ll_LOWER)|_(Lt_TITLE)|
		_(Lm_EXTENDER)|_(Lm_LETTER)|_(Lo_OTHER)|
		_(Nl_LETTER)
	);
}
bool is_ideographic(wchar32 ch)
{
	return wc_istype(ch, _(Lo_IDEOGRAPH)|_(Nl_IDEOGRAPH));
}
bool is_katakana(wchar32 ch)
{
	return wc_istype(ch, _(Lo_KATAKANA));
}
bool is_hiragana(wchar32 ch)
{
	return wc_istype(ch, _(Lo_HIRAGANA));
}
bool is_hangul_leading(wchar32 ch)
{
	return wc_istype(ch, _(Lo_LEADING));
}
bool is_hangul_vowel(wchar32 ch)
{
	return wc_istype(ch, _(Lo_VOWEL));
}
bool is_hangul_trailing(wchar32 ch)
{
	return wc_istype(ch, _(Lo_TRAILING));
}
bool is_hexdigit(wchar32 ch)
{
	return (wc_info(ch) & IS_XDIGIT) != 0;
}
bool is_decdigit(wchar32 ch)
{
	return wc_istype(ch, _(Nd_DIGIT));
}
bool is_numeric(wchar32 ch)
{
	return wc_istype(ch, _(Nd_DIGIT)|_(Nl_LETTER)|_(Nl_IDEOGRAPH)|_(No_OTHER));
}
bool is_currency(wchar32 ch)
{
	return wc_istype(ch, _(Sc_CURRENCY));
}
bool is_math(wchar32 ch)
{
	return wc_istype(ch, _(Sm_MATH));
}
bool is_symbol(wchar32 ch)
{
	return wc_istype(ch, _(Sm_MATH)|_(Sm_MINUS)|_(Sc_CURRENCY)|_(Sk_MODIFIER)|_(So_OTHER));
}
bool is_idstart(wchar32 ch) // unicode
{
	return wc_istype(ch,
		_(Lu_UPPER)|_(Ll_LOWER)|_(Lt_TITLE)|_(Lm_EXTENDER)|_(Lm_LETTER)|
		_(Lo_OTHER)|_(Lo_IDEOGRAPH)|_(Lo_KATAKANA)|_(Lo_HIRAGANA)|
		_(Lo_LEADING)|_(Lo_VOWEL)|_(Lo_TRAILING)|
		_(Nl_LETTER)
	);
}
bool is_idignorable(wchar32 ch)
{
	return is_ignorable_cntrl(ch);
}
bool is_idpart(wchar32 ch) // unicode
{
	return is_idignorable(ch) || is_idstart(ch) || wc_istype(ch,
		_(Mn_NONSPACING)|_(Mc_SPACING)|_(Nd_DIGIT)|_(Pc_CONNECTOR)
	);
}
bool is_nmstart(wchar32 ch) // xml
{
	return ch == ':' || ch == '_' || ((wc_info(ch) & IS_COMPAT) == 0 &&
		wc_istype(ch,
			_(Lu_UPPER)|_(Ll_LOWER)|_(Lt_TITLE)|_(Lm_LETTER)|
			_(Lo_OTHER)|_(Lo_IDEOGRAPH)|_(Lo_KATAKANA)|_(Lo_HIRAGANA)|
			_(Lo_LEADING)|_(Lo_VOWEL)|_(Lo_TRAILING)|
			_(Nl_LETTER)
		));
}
int is_nmchar(wchar32 ch) // xml
{
	return is_nmstart(ch) || ch == '.' || ch == '-' ||
		((wc_info(ch) & IS_COMPAT) == 0 &&
			wc_istype(ch,
				_(Lm_EXTENDER)|_(Po_EXTENDER)|
				_(Mc_SPACING)|_(Mn_NONSPACING)|_(Nd_DIGIT)|
				_(Nl_IDEOGRAPH)
			));
}
bool is_low_surrogate(wchar32 ch)
{
	return wc_istype(ch, _(Cs_LOW));
}
bool is_high_surrogate(wchar32 ch)
{
	return wc_istype(ch, _(Cs_HIGH));
}
bool is_nonbreak(wchar32 ch)
{
	return (wc_info(ch) & IS_NONBREAK) != 0;
}
bool is_private(wchar32 ch)
{
	return (wc_info(ch) & IS_PRIVATE) && !wc_istype(ch, _(Cs_HIGH));
}
bool is_unassigned(wchar32 ch)
{
	int i = wc_info(ch);
	return ((i & 0x3F) == 0) && !(i & IS_PRIVATE);
}
bool is_private_high_surrogate(wchar32 ch)
{
	return wc_istype(ch, _(Cs_HIGH)) && (wc_info(ch) & IS_PRIVATE);
}
bool is_composed(wchar32 ch)
{
	return wc_info(ch) & (IS_COMPAT|IS_CANON) ? true : false;
}
bool is_canon_composed(wchar32 ch)
{
	return wc_info(ch) & IS_CANON ? true : false;
}

// transformations

wchar32 to_lower(wchar32 ch)
{
	i32 i = wc_info(ch);
	return (wchar32)(ch + ((i & TO_LOWER) ? (i >> SVAL_OFFSET) : 0));
}
wchar32 to_upper(wchar32 ch)
{
	i32 i = wc_info(ch);
	return (wchar32)(ch - ((i & TO_UPPER) ? (i >> SVAL_OFFSET) : 0));
}
wchar32 to_title(wchar32 ch)
{
	i32 i = wc_info(ch);
	wchar32 ret = ch;
	if (i & TO_TITLE) {
		if (wc_istype(ch, _(Lu_UPPER)))
			ret++;
		else if (wc_istype(ch, _(Ll_LOWER)))
			ret--;
	} else if (i & TO_UPPER) {
		ret = (wchar32)(ret - (i >> SVAL_OFFSET));
	}
	return ret;
}
int to_digit(wchar32 ch)
{
	i32 i = wc_info(ch);
	return (i & IS_DIGIT) ? (i >> SVAL_OFFSET) : -1;
}

// BIDI properties (C2_...)

int is_bidi_left(wchar32 ch)    {return ((wc_info(ch) >> BIDI_OFFSET) & 15) == 1;}
int is_bidi_right(wchar32 ch)   {return ((wc_info(ch) >> BIDI_OFFSET) & 15) == 2;}
int is_bidi_euronum(wchar32 ch) {return ((wc_info(ch) >> BIDI_OFFSET) & 15) == 3;}
int is_bidi_eurosep(wchar32 ch) {return ((wc_info(ch) >> BIDI_OFFSET) & 15) == 4;}
int is_bidi_euroterm(wchar32 ch){return ((wc_info(ch) >> BIDI_OFFSET) & 15) == 5;}
int is_bidi_arabnum(wchar32 ch) {return ((wc_info(ch) >> BIDI_OFFSET) & 15) == 6;}
int is_bidi_commsep(wchar32 ch) {return ((wc_info(ch) >> BIDI_OFFSET) & 15) == 7;}
int is_bidi_blocksep(wchar32 ch){return ((wc_info(ch) >> BIDI_OFFSET) & 15) == 8;}
int is_bidi_segmsep(wchar32 ch) {return ((wc_info(ch) >> BIDI_OFFSET) & 15) == 9;}
int is_bidi_space(wchar32 ch)   {return ((wc_info(ch) >> BIDI_OFFSET) & 15) == 10;}
int is_bidi_neutral(wchar32 ch) {return ((wc_info(ch) >> BIDI_OFFSET) & 15) == 11;}
int is_bidi_notappl(wchar32 ch) {return ((wc_info(ch) >> BIDI_OFFSET) & 15) == 0;}

// C properties (C1_...)

bool is_space(wchar32 ch) // == is_whitespace
{
	return is_whitespace(ch);
}
bool is_lower(wchar32 ch)
{
	return wc_istype(ch, _(Ll_LOWER));
}
bool is_upper(wchar32 ch)
{
	return wc_istype(ch, _(Lu_UPPER));
}
bool is_alpha(wchar32 ch)
{
	return wc_istype(ch,
		_(Lu_UPPER)|_(Ll_LOWER)|_(Lt_TITLE)|_(Lm_LETTER)|_(Lm_EXTENDER)|
		_(Lo_OTHER)|_(Lo_IDEOGRAPH)|_(Lo_KATAKANA)|_(Lo_HIRAGANA)|
		_(Lo_LEADING)|_(Lo_VOWEL)|_(Lo_TRAILING)
	);
}
bool is_alnum(wchar32 ch)
{
	return wc_istype(ch,
		_(Lu_UPPER)|_(Ll_LOWER)|_(Lt_TITLE)|_(Lm_LETTER)|_(Lm_EXTENDER)|
		_(Lo_OTHER)|_(Lo_IDEOGRAPH)|_(Lo_KATAKANA)|_(Lo_HIRAGANA)|
		_(Lo_LEADING)|_(Lo_VOWEL)|_(Lo_TRAILING)|
		_(Nd_DIGIT)|_(Nl_LETTER)|_(Nl_IDEOGRAPH)|_(No_OTHER)
	);
}
bool is_punct(wchar32 ch)
{
	return wc_istype(ch,
		_(Pd_DASH)|
		_(Pd_HYPHEN)|_(Ps_START)|_(Ps_QUOTE)|_(Pe_END)|_(Pe_QUOTE)|_(Pc_CONNECTOR)|
		_(Po_OTHER)|_(Po_QUOTE)|_(Po_TERMINAL)|_(Po_EXTENDER)|_(Po_HYPHEN)|
		_(Pi_QUOTE)|_(Pf_QUOTE)
	);
}
bool is_xdigit(wchar32 ch) {return is_hexdigit(ch);}
bool is_digit(wchar32 ch) {return is_decdigit(ch);}
bool is_graph(wchar32 ch) {return is_alnum(ch)||is_punct(ch)||is_symbol(ch);}
bool is_blank(wchar32 ch)
{
	return wc_istype(ch, _(Zs_SPACE)|_(Zs_ZWSPACE)) || ch == '\t';
}
bool is_print(wchar32 ch) {return is_alnum(ch)||is_punct(ch)||is_symbol(ch)||is_blank(ch);}

#undef _


#define UCS2_SURROGATE_CHAR 0x046C // CAPITAL IOTIFIED BIG YUS

#endif
