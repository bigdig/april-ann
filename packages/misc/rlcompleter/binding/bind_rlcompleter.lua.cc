#ifndef LUA_USE_READLINE
#error "rlcompleter package needs READLINE support"
#endif
// GitHub link: https://github.com/rrthomas/lua-rlcompleter/
//
/************************************************************************
 * Lua readline completion for the Lua standalone interpreter
 *
 * (c) 2013 Francisco Zamora-Martinez, adapted to April-ANN
 * (c) 2011 Reuben Thomas <rrt@sc3d.org>
 * (c) 2007 Steve Donovan
 * (c) 2004 Jay Carlson
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ************************************************************************/

//BIND_HEADER_C

#include <cstdlib>
extern "C" {
#include <readline/readline.h>
}

/* Lua 5.2 compatibility */
#if LUA_VERSION_NUM > 501
static int luaL_typerror(lua_State *L, int narg, const char *tname)
{
  const char *msg = lua_pushfstring(L, "%s expected, got %s",
				    tname, luaL_typename(L, narg));
  return luaL_argerror(L, narg, msg);
}
#endif

/* Static copy of Lua state, as readline has no per-use state */
static lua_State *storedL;

/* This function is called repeatedly by rl_completion_matches inside
   do_completion, each time returning one element from the Lua table. */
static char *iterator(const char* text, int state)
{
  size_t len;
  const char *str;
  char *result;
  lua_rawgeti(storedL, -1, state + 1);
  if (lua_isnil(storedL, -1))
    return NULL;
  str = lua_tolstring(storedL, -1, &len);
  result = static_cast<char*>(malloc(len + 1));
  strcpy(result, str);
  lua_pop(storedL, 1);
  return result;
}

/* This function is called by readline() when the user wants a completion. */
static char **do_completion (const char *text, int start, int end)
{
  int oldtop = lua_gettop(storedL);
  char **matches = NULL;

  lua_pushlightuserdata(storedL, (void *)iterator);
  lua_gettable(storedL, LUA_REGISTRYINDEX);

  rl_completion_suppress_append = 1;

  if (lua_isfunction(storedL, -1)) {
    lua_pushstring(storedL, text);
    lua_pushstring(storedL, rl_line_buffer);
    lua_pushinteger(storedL, start + 1);
    lua_pushinteger(storedL, end + 1);
    if (!lua_pcall(storedL, 4, 1, 0) && lua_istable(storedL, -1))
      matches = rl_completion_matches (text, iterator);
  }
  lua_settop(storedL, oldtop);
  return matches;
}
//BIND_END

//BIND_HEADER_H
//BIND_END

//BIND_FUNCTION rlcompleter._set
{
  if (!lua_isfunction(L, 1))
    luaL_typerror(L, 1, "function");
  lua_pushlightuserdata(L, (void *)iterator);
  lua_pushvalue(L, 1);
  lua_settable(L, LUA_REGISTRYINDEX);
}
//BIND_END

//BIND_FUNCTION rlcompleter.redisplay
{
  rl_forced_update_display();
}
//BIND_END

//BIND_FUNCTION rlcompleter.readline
{
  const char* prompt = lua_tostring(L,1);
  char *line = readline(prompt);
  LUABIND_RETURN(string, line);
  /* readline return value must be free'd */
  free(line);
}
//BIND_END

//BIND_STATIC_CONSTRUCTOR rlcompleter
{
  storedL = L;
  rl_basic_word_break_characters = " \t\n\"\\'><=;:+-*/%^~#{}()[].,";
  rl_attempted_completion_function = do_completion;
}
//BIND_END
