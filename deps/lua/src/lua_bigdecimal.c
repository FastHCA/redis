#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "../lib/bigdecimal/bigdecimal.h"


#ifndef BIGDECIMAL_NAME
#define BIGDECIMAL_NAME   "bigdecimal"
#endif

#ifndef BIGDECIMAL_VERSION
#define BIGDECIMAL_VERSION   "0.1.0"
#endif


static int DEF_SIZE = 100;

typedef VP_HANDLE BigDecimal;

typedef int (*BigDecimalDivProcessProc) (lua_State *L, VP_HANDLE quotient, VP_HANDLE remainder);


static BigDecimal *
luaEX_checkBigDecimal(lua_State *L, int index)
{
    // check nth argument is a valid BigDecimal & return a ptr to it.
    void *ud = luaL_checkudata(L, index, BIGDECIMAL_NAME);
    luaL_argcheck(L, ud != NULL, index, "'bigdecimal' expected");

    return (BigDecimal *)ud;
}

static BigDecimal *
luaEX_newBigDecimal(lua_State *L, VP_HANDLE v)
{
    size_t nbytes = sizeof(BigDecimal);
    BigDecimal *d = (BigDecimal*)lua_newuserdata(L, nbytes);    // [ud]
    {
        luaL_getmetatable(L, BIGDECIMAL_NAME);                  // [ud  M]
        lua_setmetatable(L, -2);                                // [ud]
    }
    *d = v;

    return d;
}


static int
lua_bigdecimal_new(lua_State *L)
{
    VP_HANDLE v;

    int argc = lua_gettop(L);    /* number of arguments */
    if (argc > 0) {
        const char* input     = lua_tostring(L, 1);
        int         precision = luaL_optinteger (L, 2, DEF_SIZE);

        v = VpAlloc(input, precision);
    } else {
        v = VpMemAlloc(DEF_SIZE);
    }

    if (VpIsInvalid(v)) {
        VpFree(&v);
        return 0;
    }
    luaEX_newBigDecimal(L, v);

    return 1;
}


static int
lua_bigdecimal_add(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE augend = *d;

    VP_HANDLE operand;
    int free_operand = 0;

    int type = lua_type(L, 2);
    switch (type)
    {
    case LUA_TUSERDATA:
        {
            BigDecimal *pd = (BigDecimal*)lua_touserdata(L, 2);
            luaL_argcheck(L, pd != NULL, 2, "specified operand must be string, number, or 'bigdecimal' expected");
            operand = *pd;
        }
        break;

    case LUA_TNUMBER:
    case LUA_TSTRING:
        {
            const char* input = lua_tostring(L, 2);
            operand = VpAlloc(input, DEF_SIZE);

            if (VpIsInvalid(operand)) {
                VpFree(&operand);
                luaL_argerror(L, 2, "specified operand is invalid");
                return 0;
            }
        }
        free_operand = 1;
        break;

    default:
        luaL_argerror(L, 2, "specified operand must be string, number, or 'bigdecimal' expected");
        return 0;
    }

    VP_HANDLE v = VpMemAlloc(VpMaxLength(augend));
    v = VpAdd(v, augend, operand);
    luaEX_newBigDecimal(L, v);

    if (free_operand == 1) {
        VpFree(&operand);
    }

    return 1;
}


static int
lua_bigdecimal_sub(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE minuend = *d;

    VP_HANDLE operand;
    int free_operand = 0;

    int type = lua_type(L, 2);
    switch (type)
    {
    case LUA_TUSERDATA:
        {
            BigDecimal *pd = (BigDecimal*)lua_touserdata(L, 2);
            luaL_argcheck(L, pd != NULL, 2, "specified operand must be string, number, or 'bigdecimal' expected");
            operand = *pd;
        }
        break;

    case LUA_TNUMBER:
    case LUA_TSTRING:
        {
            const char* input = lua_tostring(L, 2);
            operand = VpAlloc(input, DEF_SIZE);

            if (VpIsInvalid(operand)) {
                VpFree(&operand);
                luaL_argerror(L, 2, "specified operand is invalid");
                return 0;
            }
        }
        free_operand = 1;
        break;

    default:
        luaL_argerror(L, 2, "specified operand must be string, number, or 'bigdecimal' expected");
        return 0;
    }

    if (operand) {
        VP_HANDLE v = VpMemAlloc(VpMaxLength(minuend));
        v = VpSub(v, minuend, operand);
        luaEX_newBigDecimal(L, v);

        if (free_operand == 1) {
            VpFree(&operand);
        }

        return 1;
    }

}


static int
lua_bigdecimal_mul(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE multiplier = *d;

    VP_HANDLE operand;
    int free_operand = 0;

    int type = lua_type(L, 2);
    switch (type)
    {
    case LUA_TUSERDATA:
        {
            BigDecimal *pd = (BigDecimal*)lua_touserdata(L, 2);
            luaL_argcheck(L, pd != NULL, 2, "specified operand must be string, number, or 'bigdecimal' expected");
            operand = *pd;
        }
        break;

    case LUA_TNUMBER:
    case LUA_TSTRING:
        {
            const char* input = lua_tostring(L, 2);
            operand = VpAlloc(input, DEF_SIZE);

            if (VpIsInvalid(operand)) {
                VpFree(&operand);
                luaL_argerror(L, 2, "specified operand is invalid");
                return 0;
            }
        }
        free_operand = 1;
        break;

    default:
        luaL_argerror(L, 2, "specified operand must be string, number, or 'bigdecimal' expected");
        return 0;
    }

    VP_HANDLE v;
    {
        /*  VpMaxLength(c)>VpCurLength(a)+VpCurLength(b)  */
        VP_UINT size = VpMaxLength(multiplier);
        if(size <= VpCurLength(multiplier) + VpCurLength(operand)) {
            size = VpCurLength(multiplier) + VpCurLength(operand) + 1;
        }
        v = VpMemAlloc(size);
    }
    v = VpMul(v, multiplier, operand);
    luaEX_newBigDecimal(L, v);

    if (free_operand == 1) {
        VpFree(&operand);
    }

    return 1;
}


static int
lua_bigdecimal_div_internal(lua_State *L, BigDecimalDivProcessProc process)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE dividend = *d;

    VP_HANDLE operand;
    int free_operand = 0;

    int type = lua_type(L, 2);
    switch (type)
    {
    case LUA_TUSERDATA:
        {
            BigDecimal *pd = (BigDecimal*)lua_touserdata(L, 2);
            luaL_argcheck(L, pd != NULL, 2, "specified operand must be string, number, or 'bigdecimal' expected");
            operand = *pd;
        }
        break;

    case LUA_TNUMBER:
    case LUA_TSTRING:
        {
            const char* input = lua_tostring(L, 2);
            operand = VpAlloc(input, DEF_SIZE);

            if (VpIsInvalid(operand)) {
                VpFree(&operand);
                luaL_argerror(L, 2, "specified operand is invalid");
                return 0;
            }
        }
        free_operand = 1;
        break;

    default:
        luaL_argerror(L, 2, "specified operand must be string, number, or 'bigdecimal' expected");
        return 0;
    }

    VP_HANDLE v = VpMemAlloc(VpMaxLength(dividend));
    VP_HANDLE r;
    {
        /*  VpMaxLength(r) > max(VpCurLength(a),VpMacLength(c)+VpCurLength(b))  */
        VP_UINT size = VpCurLength(operand) + VpMaxLength(v) + 1;
        if (size < VpCurLength(dividend))
            size = VpCurLength(dividend) + 1;
        r = VpMemAlloc(size);
    }
    v = VpDiv(v, r, dividend, operand);
    // luaEX_newBigDecimal(L, v);
    // luaEX_newBigDecimal(L, r);

    if (free_operand == 1) {
        VpFree(&operand);
    }

    return process(L, v, r);
}


static int process_standard_div(lua_State *L, VP_HANDLE quotient, VP_HANDLE remainder) {
    luaEX_newBigDecimal(L, quotient);
    luaEX_newBigDecimal(L, remainder);
    return 2;
}

static int
lua_bigdecimal_standard_div(lua_State *L)
{
    return lua_bigdecimal_div_internal(L, process_standard_div);
}


static int process_metatable_div(lua_State *L, VP_HANDLE quotient, VP_HANDLE remainder) {
    if (remainder) {
        VpFree(&remainder);
    }

    luaEX_newBigDecimal(L, quotient);
    return 1;
}

static int
lua_bigdecimal_metatable_div(lua_State *L)
{
    return lua_bigdecimal_div_internal(L, process_metatable_div);
}


static int process_metatable_mod(lua_State *L, VP_HANDLE quotient, VP_HANDLE remainder) {
    if (quotient) {
        VpFree(&quotient);
    }

    luaEX_newBigDecimal(L, remainder);
    return 1;
}

static int
lua_bigdecimal_metatable_mod(lua_State *L)
{
    return lua_bigdecimal_div_internal(L, process_metatable_mod);
}


static int
lua_bigdecimal_cmp(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    VP_HANDLE comparand;
    int free_comparand = 0;

    int type = lua_type(L, 2);
    switch (type)
    {
    case LUA_TUSERDATA:
        {
            BigDecimal *pd = (BigDecimal*)lua_touserdata(L, 2);
            luaL_argcheck(L, pd != NULL, 2, "specified comparand must be string, number, or 'bigdecimal' expected");
            comparand = *pd;
        }
        break;

    case LUA_TNUMBER:
    case LUA_TSTRING:
        {
            const char* input = lua_tostring(L, 2);
            comparand = VpAlloc(input, DEF_SIZE);

            if (VpIsInvalid(comparand)) {
                VpFree(&comparand);
                luaL_argerror(L, 2, "specified comparand is invalid");
                return 0;
            }
        }
        free_comparand = 1;
        break;

    default:
        luaL_argerror(L, 2, "specified comparand must be string, number, or 'bigdecimal' expected");
        return 0;
    }

    int r = VpCmp(v, comparand);
    lua_pushinteger(L, r);

    if (free_comparand == 1) {
        VpFree(&comparand);
    }

    return 1;
}


static int
lua_bigdecimal_clone(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    v = VpClone(v);
    luaEX_newBigDecimal(L, v);

    return 1;
}


static int
lua_bigdecimal_frac(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    VP_HANDLE f = VpMemAlloc(VpMaxLength(v));

    f = VpFrac(f, v);
    luaEX_newBigDecimal(L, f);

    return 1;
}


static int
lua_bigdecimal_int(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    VP_HANDLE i = VpMemAlloc(VpMaxLength(v));

    BigDecimal *integer = (BigDecimal*)lua_newuserdata(L, sizeof(BigDecimal));
    {
        luaL_getmetatable(L, BIGDECIMAL_NAME);
        lua_setmetatable(L, -2);
    }
    *integer = VpInt(i, v);

    return 1;
}


static int
lua_bigdecimal_abs(lua_State *L)
{
    VP_HANDLE operand;

    int type = lua_type(L, 1);
    switch (type)
    {
    case LUA_TUSERDATA:
        {
            BigDecimal *pd = (BigDecimal*)lua_touserdata(L, 1);
            luaL_argcheck(L, pd != NULL, 1, "specified operand must be string, number, or 'bigdecimal' expected");
            operand = VpClone(*pd);
        }
        break;

    case LUA_TNUMBER:
    case LUA_TSTRING:
        {
            const char* input = lua_tostring(L, 1);
            operand = VpAlloc(input, DEF_SIZE);

            if (VpIsInvalid(operand)) {
                VpFree(&operand);
                luaL_argerror(L, 1, "specified operand is invalid");
                return 0;
            }
        }
        break;

    default:
        luaL_argerror(L, 1, "specified operand must be string, number, or 'bigdecimal' expected");
        return 0;
    }

    VpAbs(operand);
    luaEX_newBigDecimal(L, operand);

    return 1;
}


static int
lua_bigdecimal_exponent(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    int r = VpExponent(v);
    lua_pushinteger(L, r);

    return 1;
}


static int
lua_bigdecimal_effective_digits(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    int r = VpEffectiveDigits(v);
    lua_pushinteger(L, r);

    return 1;
}


static int
lua_bigdecimal_negate(lua_State *L)
{
    VP_HANDLE operand;

    int type = lua_type(L, 1);
    switch (type)
    {
    case LUA_TUSERDATA:
        {
            BigDecimal *pd = (BigDecimal*)lua_touserdata(L, 1);
            luaL_argcheck(L, pd != NULL, 1, "specified operand must be string, number, or 'bigdecimal' expected");
            operand = VpClone(*pd);
        }
        break;

    case LUA_TNUMBER:
    case LUA_TSTRING:
        {
            const char* input = lua_tostring(L, 1);
            operand = VpAlloc(input, DEF_SIZE);

            if (VpIsInvalid(operand)) {
                VpFree(&operand);
                luaL_argerror(L, 1, "specified operand is invalid");
                return 0;
            }
        }
        break;

    default:
        luaL_argerror(L, 1, "specified operand must be string, number, or 'bigdecimal' expected");
        return 0;
    }

    VpNegate(operand);
    luaEX_newBigDecimal(L, operand);

    return 1;
}


static int
lua_bigdecimal_sign(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    int argc = lua_gettop(L);    // number of arguments
    if (argc > 1) {
        int sign = lua_tointeger(L, 2);
        v = VpSetSign(v, sign);
        luaEX_newBigDecimal(L, v);

        return 1;
    }

    int sign = VpGetSign(v);
    lua_pushinteger(L, sign);

    return 1;
}


static int
lua_bigdecimal_sqrt(lua_State *L)
{
    VP_HANDLE base;

    int type = lua_type(L, 1);
    switch (type)
    {
    case LUA_TUSERDATA:
        {
            BigDecimal *pd = (BigDecimal*)lua_touserdata(L, 1);
            luaL_argcheck(L, pd != NULL, 1, "specified operand must be string, number, or 'bigdecimal' expected");
            base = *pd;
        }
        break;

    case LUA_TNUMBER:
    case LUA_TSTRING:
        {
            const char* input = lua_tostring(L, 1);
            base = VpAlloc(input, DEF_SIZE);

            if (VpIsInvalid(base)) {
                VpFree(&base);
                luaL_argerror(L, 1, "specified operand is invalid");
                return 0;
            }
        }
        break;

    default:
        luaL_argerror(L, 1, "specified operand must be string, number, or 'bigdecimal' expected");
        return 0;
    }

    VP_HANDLE v = VpMemAlloc(VpMaxLength(base));
    v = VpSqrt(v, base);
    luaEX_newBigDecimal(L, v);

    return 1;
}


static int
lua_bigdecimal_power(lua_State *L)
{
    VP_HANDLE base;

    int type = lua_type(L, 1);
    switch (type)
    {
    case LUA_TUSERDATA:
        {
            BigDecimal *pd = (BigDecimal*)lua_touserdata(L, 1);
            luaL_argcheck(L, pd != NULL, 1, "specified operand must be string, number, or 'bigdecimal' expected");
            base = *pd;
        }
        break;

    case LUA_TNUMBER:
    case LUA_TSTRING:
        {
            const char* input = lua_tostring(L, 1);
            base = VpAlloc(input, DEF_SIZE);

            if (VpIsInvalid(base)) {
                VpFree(&base);
                luaL_argerror(L, 1, "specified operand is invalid");
                return 0;
            }
        }
        break;

    default:
        luaL_argerror(L, 1, "specified operand must be string, number, or 'bigdecimal' expected");
        return 0;
    }

    int exponent = lua_tointeger(L, 2);

    VP_HANDLE v = VpMemAlloc(VpMaxLength(base));
    v = VpPower(v, base, exponent);
    luaEX_newBigDecimal(L, v);

    return 1;
}


static int
lua_bigdecimal_isvalid(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    lua_pushboolean(L, VpIsValid(v));
    return 1;
}


static int
lua_bigdecimal_isnumberic(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    lua_pushboolean(L, VpIsNumeric(v));
    return 1;
}


static int
lua_bigdecimal_isone(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    lua_pushboolean(L, VpIsOne(v));
    return 1;
}


static int
lua_bigdecimal_isposzero(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    lua_pushboolean(L, VpIsPosZero(v));
    return 1;
}


static int
lua_bigdecimal_isnegzero(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    lua_pushboolean(L, VpIsNegZero(v));
    return 1;
}


static int
lua_bigdecimal_iszero(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    lua_pushboolean(L, VpIsZero(v));
    return 1;
}


static int
lua_bigdecimal_isnan(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    lua_pushboolean(L, VpIsNaN(v));
    return 1;
}


static int
lua_bigdecimal_isposinf(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    lua_pushboolean(L, VpIsPosInf(v));
    return 1;
}


static int
lua_bigdecimal_isneginf(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    lua_pushboolean(L, VpIsNegInf(v));
    return 1;
}


static int
lua_bigdecimal_isinf(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    lua_pushboolean(L, VpIsInf(v));
    return 1;
}


static int
lua_bigdecimal_scaleround(lua_State *L)
{
    VP_HANDLE v;

    int type = lua_type(L, 1);
    switch (type)
    {
    case LUA_TUSERDATA:
        {
            BigDecimal *pd = (BigDecimal*)lua_touserdata(L, 1);
            luaL_argcheck(L, pd != NULL, 1, "specified operand must be string, number, or 'bigdecimal' expected");
            v = *pd;
        }
        break;

    case LUA_TNUMBER:
    case LUA_TSTRING:
        {
            const char* input = lua_tostring(L, 1);
            v = VpAlloc(input, DEF_SIZE);

            if (VpIsInvalid(v)) {
                VpFree(&v);
                luaL_argerror(L, 1, "specified operand is invalid");
                return 0;
            }
        }
        break;

    default:
        luaL_argerror(L, 1, "specified operand must be string, number, or 'bigdecimal' expected");
        return 0;
    }

    int scale = lua_tointeger(L, 2);

    int mode = VpGetRoundMode();
    mode = luaL_optinteger(L, 3, mode);
    if (mode < 1 || mode > 7) {
        return luaL_argerror(L, 2, "invalid round mode");
    }

    v = VpScaleRound2(v, scale, mode);
    luaEX_newBigDecimal(L, v);

    return 1;
}


static int
lua_bigdecimal_lengthround(lua_State *L)
{
    VP_HANDLE v;

    int type = lua_type(L, 1);
    switch (type)
    {
    case LUA_TUSERDATA:
        {
            BigDecimal *pd = (BigDecimal*)lua_touserdata(L, 1);
            luaL_argcheck(L, pd != NULL, 1, "specified operand must be string, number, or 'bigdecimal' expected");
            v = *pd;
        }
        break;

    case LUA_TNUMBER:
    case LUA_TSTRING:
        {
            const char* input = lua_tostring(L, 1);
            v = VpAlloc(input, DEF_SIZE);

            if (VpIsInvalid(v)) {
                VpFree(&v);
                luaL_argerror(L, 1, "specified operand is invalid");
                return 0;
            }
        }
        break;

    default:
        luaL_argerror(L, 1, "specified operand must be string, number, or 'bigdecimal' expected");
        return 0;
    }

    int scale = lua_tointeger(L, 2);

    int mode = VpGetRoundMode();
    mode = luaL_optinteger(L, 3, mode);
    if (mode < 1 || mode > 7) {
        return luaL_argerror(L, 2, "invalid round mode");
    }

    v = VpLengthRound2(v, scale, mode);
    luaEX_newBigDecimal(L, v);

    return 1;
}


static int
lua_bigdecimal_digit_separation_count(lua_State *L)
{
    int argc = lua_gettop(L);    /* number of arguments */
    if (argc > 0) {
        int m = luaL_checkinteger(L, 1);
        lua_pushboolean(L, VpSetDigitSeparationCount(m) == m);
        return 1;
    }

    lua_pushinteger(L, VpGetDigitSeparationCount());
    return 1;
}


static int
lua_bigdecimal_digit_separator(lua_State *L)
{
    int argc = lua_gettop(L);    /* number of arguments */
    if (argc > 0) {
        size_t l;
        const char *sp = luaL_checklstring(L, 1, &l);
        char c = (char)(sp[0]);
        lua_pushboolean(L, VpSetDigitSeparator(c) == c);
        return 1;
    }

    lua_pushinteger(L, VpGetDigitSeparator());
    return 1;
}


static int
lua_bigdecimal_digit_leader(lua_State *L)
{
    int argc = lua_gettop(L);    /* number of arguments */
    if (argc > 0) {
        size_t l;
        const char *sp = luaL_checklstring(L, 1, &l);
        char c = (char)(sp[0]);
        lua_pushboolean(L, VpSetDigitLeader(c) == c);
        return 1;
    }

    char c = VpGetDigitLeader();
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    luaL_addchar(&b, c);
    luaL_pushresult(&b);
    return 1;
}


static int
lua_bigdecimal_tostring(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    const char* arg_format = NULL;

    int argc = lua_gettop(L);
    if (argc > 1) {
        arg_format = luaL_checkstring(L, 2);
    }

    size_t sz;
    char *buf = NULL;
    if ((!arg_format) || (strcmp("E", arg_format) == 0)) {
        sz = VpStringLengthE(v);
        buf = (char*)malloc(sz);
        VpToStringE(v, buf);
    } else if (strcmp("F", arg_format) == 0) {
        sz = VpStringLengthF(v);
        buf = (char*)malloc(sz);
        VpToStringF(v, buf);
    } else {
        luaL_argerror(L, 2, "invalid formatter");
        return 0;
    }
    lua_pushstring(L, buf);

    if (buf) {
        free(buf);
    }

    return 1;
}


static int
lua_bigdecimal_max_length(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    int r = VpMaxLength(v);
    lua_pushinteger(L, r);

    return 1;
}


static int
lua_bigdecimal_cur_length(lua_State *L)
{
    BigDecimal *d = luaEX_checkBigDecimal(L, 1);
    VP_HANDLE v = *d;

    int r = VpCurLength(v);
    lua_pushinteger(L, r);

    return 1;
}


static int
lua_bigdecimal_destroy(lua_State *L)
{
    lua_pop(L, 1);

    return 0;
}


static const struct luaL_Reg funcs[] = {
    { "new"             , lua_bigdecimal_new             },
    { "add"             , lua_bigdecimal_add             },
    { "sub"             , lua_bigdecimal_sub             },
    { "mul"             , lua_bigdecimal_mul             },
    { "div"             , lua_bigdecimal_standard_div    },
    { "cmp"             , lua_bigdecimal_cmp             },
    { "clone"           , lua_bigdecimal_clone           },

    // { "assign"          , lua_bigdecimal_assign          },

    { "frac"            , lua_bigdecimal_frac            },
    { "int"             , lua_bigdecimal_int             },
    { "abs"             , lua_bigdecimal_abs             },
    { "exponent"        , lua_bigdecimal_exponent        },
    { "effective_digits", lua_bigdecimal_effective_digits},
    { "negate"          , lua_bigdecimal_negate          },
    { "sign"            , lua_bigdecimal_sign            },
    { "sqrt"            , lua_bigdecimal_sqrt            },
    { "power"           , lua_bigdecimal_power           },
    { "isvalid"         , lua_bigdecimal_isvalid         },
    { "isnumberic"      , lua_bigdecimal_isnumberic      },
    { "isone"           , lua_bigdecimal_isone           },
    { "isposzero"       , lua_bigdecimal_isposzero       },
    { "isnegzero"       , lua_bigdecimal_isnegzero       },
    { "iszero"          , lua_bigdecimal_iszero          },
    { "isnan"           , lua_bigdecimal_isnan           },
    { "isposinf"        , lua_bigdecimal_isposinf        },
    { "isneginf"        , lua_bigdecimal_isneginf        },
    { "isinf"           , lua_bigdecimal_isinf           },
    { "round"           , lua_bigdecimal_scaleround      },
    { "lengthround"     , lua_bigdecimal_lengthround     },

    { "tostring"        , lua_bigdecimal_tostring        },

    { "max_length"      , lua_bigdecimal_max_length      },
    { "cur_length"      , lua_bigdecimal_cur_length      },

    { "digit_separation_count" , lua_bigdecimal_digit_separation_count},
    { "digit_separator"        , lua_bigdecimal_digit_separator       },
    { "digit_leader"           , lua_bigdecimal_digit_leader          },
    { NULL, NULL }
};


static const struct luaL_Reg metafuncs[] = {
    { "__add"      , lua_bigdecimal_add           },
    { "__sub"      , lua_bigdecimal_sub           },
    { "__mul"      , lua_bigdecimal_mul           },
    { "__div"      , lua_bigdecimal_metatable_div },
    // { "__mod"      , lua_bigdecimal_metatable_mod },   // incompatible with lua number.
                                                          //   15.345 % 7 = 0.6E-107   VpDiv()
                                                          //   15.345 % 7 = 1.345      lua number  
    // { "__pow"      , lua_bigdecimal_power         },   // incompatible with lua number.
                                                          //   The bigdecimal VpPower() exponent
                                                          //   must be an ingteger.
                                                          //   But math.pow(base, exponent) can use
                                                          //   any valid double (excepts nan, inf).
                                                          //   e.g: math.pow(10, 0.5) 
    { "__unm"      , lua_bigdecimal_negate        },
    { "__tostring" , lua_bigdecimal_tostring      },
    { "__gc"       , lua_bigdecimal_destroy       },
    { NULL, NULL }
};


/* Return bigdecimal module table */
static int luaopen_create_bigdecimal(lua_State *L)
{
    /* Manually construct our module table instead of
     * relying on _register or _newlib */
    luaL_newmetatable(L, BIGDECIMAL_NAME);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    #if LUA_VERSION_NUM < 502
        luaL_register(L, NULL, metafuncs);
    #else
        luaL_newlib(L, metafuncs);
    #endif

    /* Set module name / version fields */
    lua_pushliteral(L, BIGDECIMAL_NAME);
    lua_setfield(L, -2, "_NAME");
    lua_pushliteral(L, BIGDECIMAL_VERSION);
    lua_setfield(L, -2, "_VERSION");

    #if LUA_VERSION_NUM < 502
        luaL_register(L, NULL, funcs);
    #else
        luaL_newlib(L, funcs);
    #endif

    return 1;
}


LUALIB_API int luaopen_bigdecimal(lua_State *L) {
    luaopen_create_bigdecimal(L);

#if LUA_VERSION_NUM < 502
    /* Register name globally for 5.1 */
    lua_pushvalue(L, -1);
    lua_setglobal(L, BIGDECIMAL_NAME);
#endif

    return 1;
}
