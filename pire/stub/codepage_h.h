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


#ifndef PIRE_STUB_CODEPAGE_H_H
#define PIRE_STUB_CODEPAGE_H_H

struct CodePage;
struct Recoder;
struct Encoder;

/*****************************************************************\
 *                    struct CodePage                              *
\*****************************************************************/
struct CodePage {
	docCodes    CPEnum;       // int MIBEnum;
	const char *Names[30];    // name[0] -- preferred mime-name
	wchar32     unicode[256];
	const char *DefaultChar;  //[CCL_NUM]

	bool is_lower(int ch) const {return ch>=0 && Pire::is_lower(unicode[(unsigned char)ch]);}
	bool is_upper(int ch) const {return ch>=0 && Pire::is_upper(unicode[(unsigned char)ch]);}
	bool is_alpha(int ch) const {return ch>=0 && Pire::is_alpha(unicode[(unsigned char)ch]);}
	bool is_digit(int ch) const {return ch>=0 && Pire::is_digit(unicode[(unsigned char)ch]);}
	bool is_xdigit(int ch)const {return ch>=0 && Pire::is_xdigit(unicode[(unsigned char)ch]);}
	bool is_alnum(int ch) const {return ch>=0 && Pire::is_alnum(unicode[(unsigned char)ch]);}
	bool is_space(int ch) const {return ch>=0 && Pire::is_space(unicode[(unsigned char)ch]);}
	bool is_punct(int ch) const {return ch>=0 && Pire::is_punct(unicode[(unsigned char)ch]);}
	bool is_cntrl(int ch) const {return ch>=0 && Pire::is_cntrl(unicode[(unsigned char)ch]);}
	bool is_graph(int ch) const {return ch>=0 && Pire::is_graph(unicode[(unsigned char)ch]);}
	bool is_print(int ch) const {return ch>=0 && Pire::is_print(unicode[(unsigned char)ch]);}
	// non-standard
	bool is_composed(int ch) const {return ch>=0 && Pire::is_composed(unicode[(unsigned char)ch]);}

	char *strlwr(char *in_out, size_t len = (unsigned)(-1)) const;
	char *strupr(char *in_out, size_t len = (unsigned)(-1)) const;
	char *strlwr(const char *in, char *out, size_t len = (unsigned)(-1)) const;
	char *strupr(const char *in, char *out, size_t len = (unsigned)(-1)) const;
	int   stricmp(const char* s1, const char *s2) const;
	int   strnicmp(const char* s1, const char *s2, size_t len) const;

	unsigned char to_upper(unsigned char ch) const;
	unsigned char to_lower(unsigned char ch) const;
	unsigned char to_title(unsigned char ch) const;
	int           to_digit(unsigned char ch) const;

	static void Initialize();
};

const CodePage *CodePageByName(const char *name);

namespace NCodepagePrivate {
	class TCodepagesMap {
	private:
		const CodePage* Data[CODES_MAX];

	public:
		TCodepagesMap();

		const CodePage* Get(docCodes e) {
			YASSERT(CODES_UNKNOWN < e && e < CODES_MAX);
			return Data[e];
		}
	};
}

const CodePage *CodePageByDocCode(docCodes e)
{
	return Singleton<NCodepagePrivate::TCodepagesMap>()->Get(e);
}

docCodes DocCodeByName(const char *name)
{
	const CodePage *CP = CodePageByName(name);
	if (CP == 0)
		return CODES_UNKNOWN;
	return CP->CPEnum;
}
docCodes DocCodeByCodePage(const CodePage *CP)
{
	return CP->CPEnum;
}
const char *NameByDocCode(docCodes e)
{
	return CodePageByDocCode(e)->Names[0];
}
const char *NameByCodePage(const CodePage *CP)
{
	return CP->Names[0];
}

docCodes EncodingHintByName(const char* name);

/*****************************************************************\
 *                    struct Encoder                               *
\*****************************************************************/
enum RECODE_RESULT{
	RECODE_OK,
	RECODE_EOINPUT,
	RECODE_EOOUTPUT,
	RECODE_BROKENSYMBOL,
	RECODE_ERROR,
	RECODE_DEFAULTSYMBOL,
};

struct Encoder {
	char *Table[256];
	const  char *DefaultChar;

	char Code(wchar32 ch) const
	{
		if (ch > 0xFFFF)
			return 0;
		return (unsigned char)Table[(ch>>8)&255][ch&255];
	}
	char Tr(wchar32 ch) const
	{
		char code = Code(ch);
		if (code == 0 && ch != 0)
			code =  DefaultChar[wc_type(ch)];
		YASSERT(code != 0 || ch == 0);
		return code;
	}

	unsigned char operator [](wchar32 ch) const
	{
		return Tr(ch);
	}
	void Tr(const wchar32 *in, char *out, size_t len) const;
	void Tr(const wchar32 *in, char *out) const;
	char * DefaultPlane;
};

struct CustomEncoder : public Encoder {
	void Create (const CodePage *target, int mode=1);
	void Free ();
private:
	void addToTable(wchar32 ucode, unsigned char code, const CodePage* target);
};

struct MultipleEncMapping {
	typedef ui32 maptype;
	static maptype DefaultPlane[256];
	maptype *Table[256];
	MultipleEncMapping();
	maptype GetEncodings(wchar32 ch) const {
		return Table[(ch>>8)&255][ch&255];
	}
	void ImportEncoder(const Encoder &E, int enc);
	~MultipleEncMapping();

	DECLARE_NOCOPY(MultipleEncMapping);
};

/*****************************************************************\
 *                    struct Recoder                               *
\*****************************************************************/
struct Recoder {
	unsigned char Table[257];

	void Create(const CodePage &source, const CodePage &target);
	void Create(const CodePage &source, const Encoder* wideTarget);

	void Create(const CodePage &page, wchar32 (*mapper)(wchar32));
	void Create(const CodePage &page, const Encoder* widePage, wchar32 (*mapper)(wchar32));

	unsigned char Tr(unsigned char c) const
	{
		return Table[c];
	}
	unsigned char operator [](unsigned char c) const
	{
		return Table[c];
	}
	void  Tr(const char *in, char *out, size_t len) const;
	void  Tr(const char *in, char *out) const;
	void  Tr(char *in_out, size_t len) const;
	void  Tr(char *in_out) const;
};

extern const Encoder *EncodeTo[CODES_MAX];

extern const struct Encoder &WideCharToYandex;

extern const Recoder rcdr_to_yandex[];
extern const Recoder rcdr_from_yandex[];

extern const Recoder rcdr_to_lower[];
extern const Recoder rcdr_to_upper[];
extern const Recoder rcdr_to_title[];

unsigned char CodePage::to_upper(unsigned char ch) const
{
	return rcdr_to_upper[CPEnum].Table[ch];
}
unsigned char CodePage::to_lower(unsigned char ch) const
{
	return rcdr_to_lower[CPEnum].Table[ch];
}
unsigned char CodePage::to_title(unsigned char ch) const
{
	return rcdr_to_title[CPEnum].Table[ch];
}
int CodePage::to_digit(unsigned char ch) const
{
	return Pire::to_digit(unicode[ch]);
}

extern const struct CodePage csYandex;

const unsigned char yaUNK_Up = 0xA6;
const unsigned char yaUNK_Lo = 0xB6;
const unsigned char yaIDEOGR = 0x9F;
const unsigned char yaSHY    = 0x8F;
const unsigned char yaACUTE  = 0x80;
const unsigned char yaGradus = 0xB0;

unsigned char utf8_leadbyte_mask(size_t len) {
	// YASSERT (len <= 4);
	return "\0\0\037\017\007"[len];
}

size_t utf8_rune_len(const unsigned char p)
{
	return "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\0\0\0\0\0\2\2\2\2\3\3\4\0"[p>>3];
}

size_t utf8_rune_len_by_ucs(wchar32 rune)
{
	if (rune < 0x80)
		return 1U;
	else if (rune < 0x800)
		return 2U;
	else if (rune < 0x10000)
		return 3U;
	else if (rune < 0x200000)
		return 4U;
	else if (rune < 0x4000000)
		return 5U;
	else
		return 6U;
}

extern const wchar32 BROKEN_RUNE;

RECODE_RESULT utf8_read_rune(wchar32 &rune, size_t &rune_len, const unsigned char *s, const unsigned char *end)
{
	rune = BROKEN_RUNE;
	rune_len = 0;
	wchar32 _rune;

	size_t _len = utf8_rune_len(*s);
	if (s + _len > end) return RECODE_EOINPUT;  //[EOINPUT]
	if (_len==0) return RECODE_BROKENSYMBOL;    //[BROKENSYMBOL] in first byte
	_rune = *s++;                               //[00000000 0XXXXXXX]

	if (_len > 1) {
		_rune &= utf8_leadbyte_mask(_len);
		unsigned char ch = *s++;
		if ((ch & 0xC0) != 0x80)
			return RECODE_BROKENSYMBOL;         //[BROKENSYMBOL] in second byte
		_rune <<= 6;
		_rune |= ch & 0x3F;                     //[00000XXX XXYYYYYY]
		if (_len > 2) {
			ch = *s++;
			if ((ch & 0xC0) != 0x80)
				return RECODE_BROKENSYMBOL;     //[BROKENSYMBOL] in third byte
			_rune <<= 6;
			_rune |= ch & 0x3F;                 //[XXXXYYYY YYZZZZZZ]
			if (_len > 3) {
				ch = *s;
				if ((ch & 0xC0) != 0x80)
					return RECODE_BROKENSYMBOL; //[BROKENSYMBOL] in fourth byte
				_rune <<= 6;
				_rune |= ch & 0x3F;             //[XXXYY YYYYZZZZ ZZQQQQQQ]
			}
		}
	}
	rune_len = _len;
	rune = _rune;
	return RECODE_OK;
}

RECODE_RESULT utf8_put_rune(wchar32 rune, size_t &rune_len, unsigned char *s, const unsigned char *end){
	rune_len = 0;
	size_t tail = end - s;
	if (rune < 0x80){
		if (tail <= 0) return RECODE_EOOUTPUT;
		*s = (unsigned char)rune;
		rune_len = 1;
		return RECODE_OK;
	}
	if (rune < 0x800){
		if (tail <= 1) return RECODE_EOOUTPUT;
		*s++ = (unsigned char)(0xC0 | (rune >> 6));
		*s   = (unsigned char)(0x80 | (rune & 0x3F));
		rune_len = 2;
		return RECODE_OK;
	}
	if (rune < 0x10000) {
		if (tail <= 2) return RECODE_EOOUTPUT;
		*s++ = (unsigned char)(0xE0 | (rune >> 12));
		*s++ = (unsigned char)(0x80 | ((rune >> 6) & 0x3F));
		*s   = (unsigned char)(0x80 | (rune & 0x3F));
		rune_len = 3;
		return RECODE_OK;
	}
	/*if (rune < 0x200000)*/ {
		if (tail <= 3) return RECODE_EOOUTPUT;
		*s++ = (unsigned char)(0xF0 | ((rune >> 18) & 0x07));
		*s++ = (unsigned char)(0x80 | ((rune >> 12) & 0x3F));
		*s++ = (unsigned char)(0x80 | ((rune >> 6) & 0x3F));
		*s   = (unsigned char)(0x80 | (rune & 0x3F));
		rune_len = 4;
		return RECODE_OK;
	}
};

RECODE_RESULT utf8_read_rune_from_unknown_plane(wchar32 &rune, size_t &rune_len, const wchar32 *s, const wchar32 *end) {
	if ((*s & 0xFF00) != 0xF000) {
		rune_len = 1;
		rune = *s;
		return RECODE_OK;
	}

	rune_len = 0;

	size_t _len = utf8_rune_len((unsigned char)(*s));
	if (s + _len > end) return RECODE_EOINPUT;  //[EOINPUT]
	if (_len == 0) return RECODE_BROKENSYMBOL;  //[BROKENSYMBOL] in first byte

	wchar32 _rune = (ui8)(*s++);                //[00000000 0XXXXXXX]
	if (_len > 1) {
		_rune &= utf8_leadbyte_mask(_len);
		wchar32 ch = *s++;
		if ((ch & 0xFFC0) != 0xF080)
			return RECODE_BROKENSYMBOL;         //[BROKENSYMBOL] in second byte
		_rune <<= 6;
		_rune |= ch & 0x3F;                     //[00000XXX XXYYYYYY]
		if (_len > 2) {
			ch = *s++;
			if ((ch & 0xFFC0) != 0xF080)
				return RECODE_BROKENSYMBOL;     //[BROKENSYMBOL] in third byte
			_rune <<= 6;
			_rune |= ch & 0x3F;                 //[XXXXYYYY YYZZZZZZ]
			if (_len > 3) {
				ch = *s;
				if ((ch & 0xFFC0) != 0xF080)
					return RECODE_BROKENSYMBOL; //[BROKENSYMBOL] in fourth byte
				_rune <<= 6;
				_rune |= ch & 0x3F;             //[XXXYY YYYYZZZZ ZZQQQQQQ]
			}
		}
	}
	rune_len = _len;
	rune = _rune;
	return RECODE_OK;
}

/// this function changes (lowers) [end] position in case of utf-8
/// null character is NOT assumed or written at [*end]
void DecodeUnknownPlane(wchar32 *start, wchar32 *&end, const docCodes enc4unk);

/// this function may return less than [len] bytes in case of utf-8
/// [dst] buffer must have at least [len] bytes
/// [dst] is NOT terminated with null character
size_t DecodeUnknownAndRecodeToYandex(const wchar32 *src, char *dst, size_t len, const docCodes enc4unk);

void ToLower(char* s, size_t n, const CodePage& cp = csYandex) {
	char* const e = s + n;
	for (; s != e; ++s)
		*s = cp.to_lower(*s);
}

void ToUpper(char* s, size_t n, const CodePage& cp = csYandex) {
	char* const e = s + n;
	for (; s != e; ++s)
		*s = cp.to_upper(*s);
}

#endif
