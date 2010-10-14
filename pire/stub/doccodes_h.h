/*
 * Copyright (C) 2000-2010, Yandex
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


#ifndef PIRE_STUB_DOCCODES_H_H
#define PIRE_STUB_DOCCODES_H_H




const char* mimetypeByExt(const char *fname, const char* check_ext=0);

enum docCodes{           // CP_ENUM
	CODES_UNSUPPORTED = -2, // valid but unsupported encoding
	CODES_UNKNOWN = -1,  // invalid or unspecified encoding
	CODES_WIN,           // [ 0] CP_WINDOWS_1251     Windows
	CODES_KOI8,          // [ 1] CP_KOI8_R           Koi8-r
	CODES_ALT,           // [ 2] CP_IBM_866          MS DOS, alternative
	CODES_MAC,           // [ 3] CP_MAC_CYRILLIC     Macintosh
	CODES_MAIN,          // [ 4] CP_ISO_LATIN_CYRILLIC Main
	CODES_ASCII,         // [ 5] CP_WINDOWS_1252     Latin 1
	CODES_UNIPOL,        // [ 6] CP_UNIPOL           ISO+WIN
	CODES_WIN_EAST,      // [ 7] CP_WINDOWS_1250     WIN PL
	CODES_ISO_EAST,      // [ 8] CP_ISO_8859_2       ISO PL
	// our superset of subset of windows-1251
	CODES_YANDEX,        // [ 9] CP_YANDEX
	// popular non-standard codepages
	CODES_WINWIN,        // [10] CP_WIN_WIN          win-autoconverted-from-koi-to-win (win_as_koi_to_win)
	CODES_KOIKOI,        // [11] CP_KOI_KOI          koi-autoconverted-from-win-to-koi (koi_as_win_to_koi)
	// missing standard codepages
	CODES_IBM855,        // [12] CP_IBM_855
	CODES_UTF8,          // [13] CP_FakeUTF8
	CODES_UNKNOWNPLANE,  // [14] Unrecognized characters are mapped into the PUA: U+F000..U+F0FF

	CODES_KAZWIN,        // [15] CP_WINDOWS_1251_K   Kazakh version of Windows-1251
	CODES_TATWIN,        // [16] CP_WINDOWS_1251_T   Tatarian version of Windows-1251
	CODES_ARMSCII,       // [17] Armenian ASCII
	CODES_GEO_ITA,       // [18] Academy of Sciences Georgian
	CODES_GEO_PS,        // [19] Georgian Parliament
	CODES_ISO_8859_3,    // [20] Latin-3: Turkish, Maltese and Esperanto
	CODES_ISO_8859_4,    // [21] Latin-4: Estonian, Latvian, Lithuanian, Greenlandic, Sami
	CODES_ISO_8859_6,    // [22] Latin/Arabic: Arabic
	CODES_ISO_8859_7,    // [23] Latin/Greek: Greek
	CODES_ISO_8859_8,    // [24] Latin/Hebrew: Hebrew
	CODES_ISO_8859_9,    // [25] Latin-5 or Turkish: Turkish
	CODES_ISO_8859_13,   // [26] Latin-7 or Baltic Rim: Baltic languages
	CODES_ISO_8859_15,   // [27] Latin-9: Western European languages
	CODES_ISO_8859_16,   // [28] Latin-10: South-Eastern European languages
	CODES_WINDOWS_1253,  // [29] for Greek
	CODES_WINDOWS_1254,  // [30] for Turkish
	CODES_WINDOWS_1255,  // [31] for Hebrew
	CODES_WINDOWS_1256,  // [32] for Arabic
	CODES_WINDOWS_1257,  // [33] for Estonian, Latvian and Lithuanian

	// these codes are all the other 8bit codes known by libiconv
	// they follow in alphanumeric order
	CODES_CP1046,
	CODES_CP1124,
	CODES_CP1125,
	CODES_CP1129,
	CODES_CP1131,
	CODES_CP1133,
	CODES_CP1161,
	CODES_CP1162,
	CODES_CP1163,
	CODES_CP1258,
	CODES_CP437,
	CODES_CP737,
	CODES_CP775,
	CODES_CP850,
	CODES_CP852,
	CODES_CP853,
	CODES_CP856,
	CODES_CP857,
	CODES_CP858,
	CODES_CP860,
	CODES_CP861,
	CODES_CP862,
	CODES_CP863,
	CODES_CP864,
	CODES_CP865,
	CODES_CP869,
	CODES_CP874,
	CODES_CP922,
	CODES_HP_ROMAN8,
	CODES_ISO646_CN,
	CODES_ISO646_JP,
	CODES_ISO8859_10,
	CODES_ISO8859_11,
	CODES_ISO8859_14,
	CODES_JISX0201,
	CODES_KOI8_T,
	CODES_MAC_ARABIC,
	CODES_MAC_CENTRALEUROPE,
	CODES_MAC_CROATIAN,
	CODES_MAC_GREEK,
	CODES_MAC_HEBREW,
	CODES_MAC_ICELAND,
	CODES_MAC_ROMANIA,
	CODES_MAC_ROMAN,
	CODES_MAC_THAI,
	CODES_MAC_TURKISH,
	CODES_MAC_UKRAINE,
	CODES_MULELAO,
	CODES_NEXTSTEP,
	CODES_PT154,
	CODES_RISCOS_LATIN1,
	CODES_RK1048,
	CODES_TCVN,
	CODES_TDS565,
	CODES_TIS620,
	CODES_VISCII,

	CODES_MAX            //      CP_NUM
};

docCodes codingByStr(const char *codingStr);

enum docLanguage {
	LANG_UNK = 0,
	LANG_RUS = 1,
	LANG_ENG = 2,
	LANG_POL = 3,
	LANG_HUN = 4,
	LANG_UKR = 5,
	LANG_GER = 6,
	LANG_FRN = 7,
	LANG_TAT = 8,
	LANG_BLR = 9,
	LANG_KAZ = 10,
	LANG_NAMES = 11,
	LANG_SPA = 12,
	LANG_ITA = 13,
	LANG_ARM = 14,
	LANG_DAN = 15,
	LANG_POR = 16,
	LANG_BRA = 17,
	LANG_SLO = 18,
	LANG_SLV = 19,
	LANG_DUT = 20,
	LANG_BUL = 21,
	LANG_CAT = 22,
	LANG_HRV = 23,
	LANG_CZE = 24,
	LANG_GRE = 25,
	LANG_HEB = 26,
	LANG_NOB = 27,
	LANG_NNO = 28,
	LANG_SWE = 29,
	LANG_KOR = 30,
	LANG_LAT = 31,
	LANG_TRANS = 32,
	LANG_E2R = 33,
	LANG_R2E = 34,
	LANG_EMPTY = 35,
	LANG_UNK_LAT = 36,
	LANG_UNK_CYR = 37,
	LANG_UNK_ALPHA = 38,
	LANG_FIN = 39,
	LANG_EST = 40,
	LANG_LAV = 41,
	LANG_LIT = 42,
	LANG_BAK = 43,
	LANG_TUR = 44,
	LANG_RUM = 45,
	LANG_MON = 46,
	LANG_UZB = 47,
	LANG_KIR = 48,
	LANG_TGK = 49,
	LANG_TUK = 50,
	LANG_SRP = 51,
	LANG_AZE = 52,
	// missed language: use it for new language before adding them to the end of the list
	LANG_GEO = 54,
	LANG_ARA = 55,
	LANG_PER = 56,
	LANG_UNTRANS = 57,
	LANG_MAX
};

docLanguage LanguageByName(const char *name);
extern const char *NameByLanguage[LANG_MAX];
extern const char *IsoNameByLanguage[LANG_MAX];

enum MimeTypes {
	MIME_UNKNOWN    = 0,
	MIME_TEXT       = 1,
	MIME_HTML       = 2,  MIME_XHTMLXML = MIME_HTML,
	MIME_PDF        = 3,
	MIME_RTF        = 4,
	MIME_DOC        = 5,  MIME_MSWORD = MIME_DOC,
	MIME_MPEG       = 6,
	MIME_XML        = 7,  MIME_RSS = MIME_XML,
	MIME_WML        = 8,
	MIME_SWF        = 9,  MIME_FLASH = MIME_SWF,
	MIME_XLS        = 10, MIME_EXCEL = MIME_XLS,
	MIME_PPT        = 11,
	MIME_IMAGE_JPG  = 12,
	MIME_IMAGE_PJPG = 13,
	MIME_IMAGE_PNG  = 14,
	MIME_IMAGE_GIF  = 15,
	MIME_DOCX       = 16,
	MIME_ODT        = 17,
	MIME_ODP        = 18,
	MIME_ODS        = 19,
	//MIME_XHTMLXML   = 20,
	MIME_IMAGE_BMP  = 21,
	MIME_WAV        = 22,
	MIME_ARCHIVE    = 23,
	MIME_EXE        = 24,
	MIME_ODG        = 25,
	MIME_GZIP       = 26,
	MIME_XLSX       = 27,
	MIME_PPTX       = 28,
	MIME_MAX
};

extern const char *MimeNames[MIME_MAX];

MimeTypes mimeByStr(const char *mimeStr) {
	if (strcmp(mimeStr,"text/html") == 0)
		return MIME_HTML;
	if (strcmp(mimeStr,"text/plain") == 0)
		return MIME_TEXT;
	if (strcmp(mimeStr,"audio/mpeg") == 0)
		return MIME_MPEG;
	if (strcmp(mimeStr,"text/xml") == 0)
		return MIME_XML;
	if (strcmp(mimeStr,"text/vnd.wap.wml") == 0)
		return MIME_WML;
	if (strcmp(mimeStr,"application/pdf") == 0)
		return MIME_PDF;
	if (strcmp(mimeStr,"text/rtf") == 0)
		return MIME_RTF;
	if (strcmp(mimeStr,"application/msword") == 0)
		return MIME_DOC;
	if (strcmp(mimeStr,"application/x-shockwave-flash") == 0)
		return MIME_SWF;
	if (strcmp(mimeStr,"application/vnd.ms-excel") == 0)
		return MIME_XLS;
	if (strcmp(mimeStr,"application/vnd.ms-powerpoint") == 0)
		return MIME_PPT;
	if (strcmp(mimeStr,"application/xhtml+xml") == 0)
		return MIME_XHTMLXML;
	if (strcmp(mimeStr,"image/jpeg") == 0)
		return MIME_IMAGE_JPG;
	if (strcmp(mimeStr,"image/jpg") == 0)
		return MIME_IMAGE_JPG;
	if (strcmp(mimeStr,"image/pjpeg") == 0)
		return MIME_IMAGE_PJPG;
	if (strcmp(mimeStr,"image/png") == 0)
		return MIME_IMAGE_PNG;
	if (strcmp(mimeStr,"image/gif") == 0)
		return MIME_IMAGE_GIF;
	if (strcmp(mimeStr,"application/vnd.openxmlformats-officedocument.wordprocessingml.document") == 0)
		return MIME_DOCX;
	if (strcmp(mimeStr,"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet") == 0)
		return MIME_XLSX;
	if (strcmp(mimeStr,"application/vnd.openxmlformats-officedocument.presentationml.presentation") == 0)
		return MIME_PPTX;
	if (strcmp(mimeStr,"application/vnd.oasis.opendocument.text") == 0)
		return MIME_ODT;
	if (strcmp(mimeStr,"application/vnd.oasis.opendocument.presentation") == 0)
		return MIME_ODP;
	if (strcmp(mimeStr,"application/vnd.oasis.opendocument.spreadsheet") == 0)
		return MIME_ODS;
	if (strcmp(mimeStr,"application/vnd.oasis.opendocument.graphics") == 0)
		return MIME_ODG;
	if (strcmp(mimeStr, "image/x-ms-bmp") == 0)
		return MIME_IMAGE_BMP;
	if (strcmp(mimeStr, "audio/x-wav") == 0)
		return MIME_WAV;
	if (strcmp(mimeStr, "application/x-archive") == 0)
		return MIME_ARCHIVE;
	if (strcmp(mimeStr, "application/x-dosexec") == 0)
		return MIME_EXE;
	if (strcmp(mimeStr, "application/x-gzip") == 0)
		return MIME_GZIP;
	return MIME_UNKNOWN;
}

const char* strByMime(MimeTypes mime) {
	const char *res = 0;
	if (mime == MIME_HTML)
		res = "text/html";
	else if (mime == MIME_TEXT)
		res = "text/plain";
	else if (mime == MIME_MPEG)
		res = "audio/mpeg";
	else if (mime == MIME_XML)
		res = "text/xml";
	else if (mime == MIME_WML)
		res = "text/vnd.wap.wml";
	else if (mime == MIME_PDF)
		res = "application/pdf";
	else if (mime == MIME_RTF)
		res = "text/rtf";
	else if (mime == MIME_DOC)
		res = "application/msword";
	else if (mime == MIME_SWF)
		res = "application/x-shockwave-flash";
	else if (mime == MIME_XLS)
		res = "application/vnd.ms-excel";
	else if (mime == MIME_PPT)
		res = "application/vnd.ms-powerpoint";
	else if (mime == MIME_XHTMLXML)
		res = "application/xhtml+xml";
	else if (mime == MIME_IMAGE_JPG)
		res = "image/jpeg";
	else if (mime == MIME_IMAGE_PJPG)
		res = "image/pjpeg";
	else if (mime == MIME_IMAGE_PNG)
		res = "image/png";
	else if (mime == MIME_IMAGE_GIF)
		res = "image/gif";
	else if (mime == MIME_DOCX)
		res = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	else if (mime == MIME_XLSX)
		res = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	else if (mime == MIME_PPTX)
		res = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	else if (mime == MIME_ODT)
		res = "application/vnd.oasis.opendocument.text";
	else if (mime == MIME_ODP)
		res = "application/vnd.oasis.opendocument.presentation";
	else if (mime == MIME_ODS)
		res = "application/vnd.oasis.opendocument.spreadsheet";
	else if (mime == MIME_ODG)
		res = "application/vnd.oasis.opendocument.graphics";
	else if (mime == MIME_IMAGE_BMP)
		res = "image/x-ms-bmp";
	else if (mime == MIME_WAV)
		res = "audio/x-wav";
	else if (mime == MIME_ARCHIVE)
		res = "application/x-archive";
	else if (mime == MIME_EXE)
		res = "application/x-dosexec";
	else if (mime == MIME_GZIP)
		res = "application/x-gzip";
	return res;
}

#endif
