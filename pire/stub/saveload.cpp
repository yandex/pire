#include "saveload.h"

namespace Pire {

enum mp_type MsgpuckTypeof(yistream* s)
{
	int c;

	c = s->peek();
	if (c == EOF)
		throw Error("Parse error");
	return mp_typeof(c);
}

uint32_t MsgpuckReadArray(yistream* s)
{
	char buf[5]; /* see mp_sizeof_array() */
	const char *ptr;
	ptrdiff_t size;

	if (!s->read(buf, 1))
		throw Error("Read error");
	if (mp_typeof(buf[0]) != MP_ARRAY)
		throw Error("Parse error");
	if ((size = mp_check_array(buf, buf + 1)) > 0) {
		if (!s->read(buf + 1, size))
			throw Error("Read error");
	}
	ptr = buf;
	return mp_decode_array(&ptr);
}

uint64_t MsgpuckReadUint(yistream* s)
{
	char buf[9]; /* see mp_sizeof_uint() */
	const char *ptr;
	ptrdiff_t size;

	if (!s->read(buf, 1))
		throw Error("Read error");
	if (mp_typeof(buf[0]) != MP_UINT)
		throw Error("Parse error");
	if ((size = mp_check_uint(buf, buf + 1)) > 0) {
		if (!s->read(buf + 1, size))
			throw Error("Read error");
	}
	ptr = buf;
	return mp_decode_uint(&ptr);
}

bool MsgpuckReadBool(yistream* s)
{
	char buf[1]; /* see mp_sizeof_bool() */
	const char *ptr;

	if (!s->read(buf, 1))
		throw Error("Read error");
	if (mp_typeof(buf[0]) != MP_BOOL)
		throw Error("Parse error");
	ptr = buf;
	return mp_decode_bool(&ptr);
}

void MsgpuckReadNil(yistream* s)
{
	char buf[1]; /* see mp_sizeof_nil() */

	if (!s->read(buf, 1))
		throw Error("Read error");
	if (mp_typeof(buf[0]) != MP_NIL)
		throw Error("Parse error");
}

}
