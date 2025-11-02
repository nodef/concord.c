#include <string.h>

#include "discord.h"
#include "discord-internal.h"
#include "discord_codecs.h"

static bool
_is_zero(const void *ptr, size_t size)
{
    static const unsigned char zeros[sizeof(long double)] = { 0 };
    return !ptr || memcmp(ptr, zeros, size) == 0;
}

static bool
_is_optional(const struct discord_data_wrap *member)
{
    return member && (member->tmpl->attrs & CONCORD_OPTIONAL);
}

static CCORDcode
_jsonbcode_to_ccord(jsonbcode code)
{
    if (code >= JSONB_OK) {
        return CCORD_OK;
    }
    switch (code) {
    case JSONB_ERROR_NOMEM:
        return CCORD_OUT_OF_MEMORY;
    case JSONB_ERROR_OVERFLOW:
    case JSONB_ERROR_STACK:
    case JSONB_ERROR_INPUT:
    default:
        return CCORD_BAD_ENCODE;
    }
}

CCORDcode
discord_data_wrap_to_jsonb(struct jsonb *jb,
                           const struct discord_data_wrap *member,
                           char *p_buf[],
                           size_t *p_bufsize)
{
    const void *value = reflectc_get(member);
    if (_is_optional(member) && _is_zero(value, member->tmpl->size)) {
        return JSONB_OK;
    }
    if (reflectc_is_null(member)) {
        jsonbcode code = jsonb_null_auto(jb, p_buf, p_bufsize);
        return _jsonbcode_to_ccord(code);
    }

    switch (member->tmpl->type) {
    case REFLECTC_TYPES(char): {
        const char *resolved = (const char *)value;
        if (!resolved || !*resolved) {
            break;
        }
        jsonbcode code = jsonb_string_auto(jb, p_buf, p_bufsize, resolved,
                                           strlen(resolved));
        return _jsonbcode_to_ccord(code);
    }
    case REFLECTC_TYPES(enum):
    case REFLECTC_TYPES(short):
    case REFLECTC_TYPES(int):
    case REFLECTC_TYPES(long): {
        long long resolved = 0ULL;
        memcpy(&resolved, value, member->tmpl->size);
        jsonbcode code =
            jsonb_number_auto(jb, p_buf, p_bufsize, (double)resolved);
        return _jsonbcode_to_ccord(code);
    }
    case REFLECTC_TYPES(float): {
        const float resolved = *(const float *)value;
        jsonbcode code =
            jsonb_number_auto(jb, p_buf, p_bufsize, (float)resolved);
        return _jsonbcode_to_ccord(code);
    }
    case REFLECTC_TYPES(double): {
        const double resolved = *(const double *)value;
        jsonbcode code = jsonb_number_auto(jb, p_buf, p_bufsize, resolved);
        return _jsonbcode_to_ccord(code);
    }
    case REFLECTC_TYPES(bool): {
        const bool resolved = *(const bool *)value;
        if (resolved == false && _is_optional(member)) {
            break;
        }
        jsonbcode code = jsonb_bool_auto(jb, p_buf, p_bufsize, resolved);
        return _jsonbcode_to_ccord(code);
    }
    case REFLECTC_TYPES(struct): {
        jsonbcode code;
        if (reflectc_length(member) > 1) {
            code = jsonb_array_auto(jb, p_buf, p_bufsize);
            if (code < JSONB_OK) {
                return _jsonbcode_to_ccord(code);
            }
            for (size_t i = 0; i < member->length; ++i) {
                code = discord_data_wrap_to_jsonb(
                    jb, &member->members.array[i], p_buf, p_bufsize);
                if (code < JSONB_OK) {
                    return _jsonbcode_to_ccord(code);
                }
            }
            code = jsonb_array_pop_auto(jb, p_buf, p_bufsize);
            return _jsonbcode_to_ccord(code);
        }
        else {
            code = jsonb_object_auto(jb, p_buf, p_bufsize);
            if (code < JSONB_OK) {
                return _jsonbcode_to_ccord(code);
            }
            for (size_t i = 0; i < member->members.length; ++i) {
                const struct discord_data_wrap *f = &member->members.array[i];

                if ((code = jsonb_key_auto(jb, p_buf, p_bufsize,
                                           f->tmpl->name.buf,
                                           f->tmpl->name.length))
                    < JSONB_OK)
                {
                    return _jsonbcode_to_ccord(code);
                }
                if ((code =
                         discord_data_wrap_to_jsonb(jb, f, p_buf, p_bufsize))
                    < JSONB_OK)
                {
                    return _jsonbcode_to_ccord(code);
                }
            }
            code = jsonb_object_pop_auto(jb, p_buf, p_bufsize);
            return _jsonbcode_to_ccord(code);
        }
    } break;
    case REFLECTC_TYPES(union):
    default: {
        jsonbcode code = jsonb_null_auto(jb, p_buf, p_bufsize);
        return _jsonbcode_to_ccord(code);
    }
    }
    return JSONB_OK;
}

CCORDcode
discord_data_wrap_to_json(const struct discord_data_wrap *root,
                          char *p_buf[],
                          size_t *p_bufsize)
{
    struct jsonb jb;
    jsonb_init(&jb);
    return discord_data_wrap_to_jsonb(&jb, root, p_buf, p_bufsize);
}

static void
_json_parse_assign_null(const struct discord_data_wrap *member)
{
    unsigned depth = reflectc_get_pointer_depth(member);
    const void *value = NULL;

    if (depth < 2) {
        return;
    }
    memcpy((void *)member->ptr_value, &value, member->tmpl->size);
}

CCORDcode
discord_data_wrap_from_jsmnf(const struct jsmnf_pair *p,
                             const char *json,
                             size_t length,
                             const struct discord_data_wrap *member)
{
    if (!p || !member) {
        return CCORD_OK;
    }

    switch (p->v->type) {
    case JSMN_STRING: {
        if (member->tmpl->type != REFLECTC_TYPES(char)
            || reflectc_get_pointer_depth(member) != 2)
        {
            return CCORD_OK;
        }
        reflectc_string(member, json + p->v->start,
                        (size_t)(p->v->end - p->v->start));
        return CCORD_OK;
    }
    case JSMN_PRIMITIVE: {
        switch (json[p->v->start]) {
        case 't':
        case 'f': {
            const bool value = json[p->v->start] == 't';
            if (member->tmpl->type != REFLECTC_TYPES(bool)) {
                return CCORD_OK;
            }
            reflectc_set(member, &value, sizeof(value));
            return CCORD_OK;
        } break;
        case 'n':
            _json_parse_assign_null(member);
            return CCORD_OK;
        default: {
            switch (member->tmpl->type) {
            case REFLECTC_TYPES(float): {
                const float value = strtof(json + p->v->start, NULL);
                reflectc_set(member, &value, sizeof(value));
            } break;
            case REFLECTC_TYPES(double): {
                const double value = strtod(json + p->v->start, NULL);
                reflectc_set(member, &value, sizeof(value));
            } break;
            case REFLECTC_TYPES(int): {
                const int value = (int)strtol(json + p->v->start, NULL, 10);
                reflectc_set(member, &value, sizeof(value));
            } break;
            case REFLECTC_TYPES(enum): {
                const int value = (int)strtol(json + p->v->start, NULL, 10);
                reflectc_set(member, &value, member->tmpl->size);
            } break;
            case REFLECTC_TYPES(long): {
                const long value = strtol(json + p->v->start, NULL, 10);
                reflectc_set(member, &value, sizeof(value));
            } break;
            case REFLECTC_TYPES(short): {
                const short value =
                    (short)strtol(json + p->v->start, NULL, 10);
                reflectc_set(member, &value, sizeof(value));
            } break;
            case REFLECTC_TYPES(char): {
                const char value = (char)strtol(json + p->v->start, NULL, 10);
                reflectc_set(member, &value, sizeof(value));
            } break;
            default:
                return CCORD_OK;
            }
            return CCORD_OK;
        } break;
        }
    }
    case JSMN_OBJECT:
        if (member->tmpl->type != REFLECTC_TYPES(struct)) {
            return CCORD_OK;
        }
        for (size_t i = 0; i < p->capacity; ++i) {
            const size_t pos = reflectc_get_pos(member, p->buckets[i].key.buf,
                                                p->buckets[i].key.length);
            if (pos != SIZE_MAX) {
                const jsmnf_pair *p_m = jsmnf_find(p, p->buckets[i].key.buf,
                                                   p->buckets[i].key.length);
                const struct discord_data_wrap *m =
                    member->members.array + pos;
                if (!p_m) {
                    _json_parse_assign_null(m);
                    continue;
                }
                CCORDcode code =
                    discord_data_wrap_from_jsmnf(p_m, json, length, m);
                if (code != CCORD_OK) {
                    return code;
                }
            }
        }
        return CCORD_OK;
    case JSMN_ARRAY:
        reflectc_array(member, p->length);
        for (size_t i = 0; i < p->length; ++i) {
            CCORDcode code = discord_data_wrap_from_jsmnf(&p->fields[i], json,
                                                          length, &member[i]);
            if (code != CCORD_OK) {
                return code;
            }
        }
        return CCORD_OK;
    default:
        return CCORD_OK;
    }
    return CCORD_OK;
}

static CCORDcode
_jsmncode_to_ccord(long code)
{
    if (code > 0) {
        return CCORD_OK;
    }
    switch ((int)code) {
    case JSMN_ERROR_NOMEM:
        return CCORD_OUT_OF_MEMORY;
    case JSMN_ERROR_INVAL:
    case JSMN_ERROR_PART:
    default:
        return CCORD_BAD_DECODE;
    }
}

CCORDcode
discord_data_wrap_from_json(const char *json,
                            size_t len,
                            struct discord_data_wrap *root)
{
    jsmnf_table *pairs = NULL;
    size_t pairs_len = 0;
    jsmnf_loader l;
    jsmnf_init(&l);
    long code = jsmnf_load_auto(&l, json, len, &pairs, &pairs_len);
    if (code <= 0) {
        CCORDcode err = _jsmncode_to_ccord(code);
        free(pairs);
        return err;
    }
    CCORDcode result = discord_data_wrap_from_jsmnf(l.root, json, len, root);
    free(pairs);
    return result;
}
