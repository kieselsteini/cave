////////////////////////////////////////////////////////////////////////////////
//
// Cave - a roguelike library/engine for Lua
//
//
// MIT License
//
// Copyright(c) 2017 Sebastian Steinhauer
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
////////////////////////////////////////////////////////////////////////////////

#include "SDL.h"
#include "lua53.h"

#define CAVE_COLS           80
#define CAVE_ROWS           25

#define CAVE_ATLAS_COLS     12
#define CAVE_ATLAS_ROWS     8

#define CAVE_FPS            30
#define CAVE_FPS_TICKS      (1000 / CAVE_FPS)



////////////////////////////////////////////////////////////////////////////////
//
//  Global Variables
//
////////////////////////////////////////////////////////////////////////////////

int running = SDL_TRUE;
Uint32 randomseed = 47 * 1024; // 47 is a random number

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

SDL_Texture *glyph_atlas = NULL;
SDL_Rect glyphs[256];
int glyph_width, glyph_height;
int font_number = 0;

int screen_theme = 0;
int screen_fullscreen = SDL_FALSE;
char characters[CAVE_COLS][CAVE_ROWS];
char colors[CAVE_COLS][CAVE_ROWS];

int cursor_visible = 0;
int cursor_x = -1;
int cursor_y = -1;



////////////////////////////////////////////////////////////////////////////////
//
//  Color Schemes
//
////////////////////////////////////////////////////////////////////////////////

#define CAVE_NUM_THEMES 7

static const SDL_Color palette[CAVE_NUM_THEMES][16] = {
  // CGA default
  {
    { 0x00, 0x00, 0x00, 0xff },
    { 0x00, 0x00, 0xaa, 0xff },
    { 0x00, 0xaa, 0x00, 0xff },
    { 0x00, 0xaa, 0xaa, 0xff },
    { 0xaa, 0x00, 0x00, 0xff },
    { 0xaa, 0x00, 0xaa, 0xff },
    { 0xaa, 0x55, 0x00, 0xff },
    { 0xaa, 0xaa, 0xaa, 0xff },
    { 0x55, 0x55, 0x55, 0xff },
    { 0x55, 0x55, 0xff, 0xff },
    { 0x55, 0xff, 0x55, 0xff },
    { 0x55, 0xff, 0xff, 0xff },
    { 0xff, 0x55, 0x55, 0xff },
    { 0xff, 0x55, 0xff, 0xff },
    { 0xff, 0xff, 0x55, 0xff },
    { 0xff, 0xff, 0xff, 0xff },
  },
  // monokai.dark
  {
    { 0x27, 0x28, 0x22, 0xff },
    { 0xf9, 0x26, 0x72, 0xff },
    { 0xa6, 0xe2, 0x2e, 0xff },
    { 0xf4, 0xbf, 0x75, 0xff },
    { 0x66, 0xd9, 0xef, 0xff },
    { 0xae, 0x81, 0xff, 0xff },
    { 0xa1, 0xef, 0xe4, 0xff },
    { 0xf8, 0xf8, 0xf2, 0xff },
    { 0x75, 0x71, 0x5e, 0xff },
    { 0xf9, 0x26, 0x72, 0xff },
    { 0xa6, 0xe2, 0x2e, 0xff },
    { 0xf4, 0xbf, 0x75, 0xff },
    { 0x66, 0xd9, 0xef, 0xff },
    { 0xae, 0x81, 0xff, 0xff },
    { 0xa1, 0xef, 0xe4, 0xff },
    { 0xf9, 0xf8, 0xf5, 0xff }
  },
  // solarized.dark
  {
    { 0x00, 0x2b, 0x36, 0xff },
    { 0xdc, 0x32, 0x2f, 0xff },
    { 0x85, 0x99, 0x00, 0xff },
    { 0xb5, 0x89, 0x00, 0xff },
    { 0x26, 0x8b, 0xd2, 0xff },
    { 0x6c, 0x71, 0xc4, 0xff },
    { 0x2a, 0xa1, 0x98, 0xff },
    { 0x93, 0xa1, 0xa1, 0xff },
    { 0x65, 0x7b, 0x83, 0xff },
    { 0xdc, 0x32, 0x2f, 0xff },
    { 0x85, 0x99, 0x00, 0xff },
    { 0xb5, 0x89, 0x00, 0xff },
    { 0x26, 0x8b, 0xd2, 0xff },
    { 0x6c, 0x71, 0xc4, 0xff },
    { 0x2a, 0xa1, 0x98, 0xff },
    { 0xfd, 0xf6, 0xe3, 0xff }
  },
  // mocha.dark
  {
    { 0x3b, 0x32, 0x28, 0xff },
    { 0xcb, 0x60, 0x77, 0xff },
    { 0xbe, 0xb5, 0x5b, 0xff },
    { 0xf4, 0xbc, 0x87, 0xff },
    { 0x8a, 0xb3, 0xb5, 0xff },
    { 0xa8, 0x9b, 0xb9, 0xff },
    { 0x7b, 0xbd, 0xa4, 0xff },
    { 0xd0, 0xc8, 0xc6, 0xff },
    { 0x7e, 0x70, 0x5a, 0xff },
    { 0xcb, 0x60, 0x77, 0xff },
    { 0xbe, 0xb5, 0x5b, 0xff },
    { 0xf4, 0xbc, 0x87, 0xff },
    { 0x8a, 0xb3, 0xb5, 0xff },
    { 0xa8, 0x9b, 0xb9, 0xff },
    { 0x7b, 0xbd, 0xa4, 0xff },
    { 0xf5, 0xee, 0xeb, 0xff }
  },
  // atelierforest.dark
  {
    { 0x1b, 0x19, 0x18, 0xff },
    { 0xf2, 0x2c, 0x40, 0xff },
    { 0x5a, 0xb7, 0x38, 0xff },
    { 0xd5, 0x91, 0x1a, 0xff },
    { 0x40, 0x7e, 0xe7, 0xff },
    { 0x66, 0x66, 0xea, 0xff },
    { 0x00, 0xad, 0x9c, 0xff },
    { 0xa8, 0xa1, 0x9f, 0xff },
    { 0x76, 0x6e, 0x6b, 0xff },
    { 0xf2, 0x2c, 0x40, 0xff },
    { 0x5a, 0xb7, 0x38, 0xff },
    { 0xd5, 0x91, 0x1a, 0xff },
    { 0x40, 0x7e, 0xe7, 0xff },
    { 0x66, 0x66, 0xea, 0xff },
    { 0x00, 0xad, 0x9c, 0xff },
    { 0xf1, 0xef, 0xee, 0xff }
  },
  // grayscale.dark
  {
    { 0x10, 0x10, 0x10, 0xff },
    { 0x7c, 0x7c, 0x7c, 0xff },
    { 0x8e, 0x8e, 0x8e, 0xff },
    { 0xa0, 0xa0, 0xa0, 0xff },
    { 0x68, 0x68, 0x68, 0xff },
    { 0x74, 0x74, 0x74, 0xff },
    { 0x86, 0x86, 0x86, 0xff },
    { 0xb9, 0xb9, 0xb9, 0xff },
    { 0x52, 0x52, 0x52, 0xff },
    { 0x7c, 0x7c, 0x7c, 0xff },
    { 0x8e, 0x8e, 0x8e, 0xff },
    { 0xa0, 0xa0, 0xa0, 0xff },
    { 0x68, 0x68, 0x68, 0xff },
    { 0x74, 0x74, 0x74, 0xff },
    { 0x86, 0x86, 0x86, 0xff },
    { 0xf7, 0xf7, 0xf7, 0xff }
  },
  // greenscreen.dark
  {
    { 0x00, 0x11, 0x00, 0xff },
    { 0x00, 0x77, 0x00, 0xff },
    { 0x00, 0xbb, 0x00, 0xff },
    { 0x00, 0x77, 0x00, 0xff },
    { 0x00, 0x99, 0x00, 0xff },
    { 0x00, 0xbb, 0x00, 0xff },
    { 0x00, 0x55, 0x00, 0xff },
    { 0x00, 0xbb, 0x00, 0xff },
    { 0x00, 0x77, 0x00, 0xff },
    { 0x00, 0x77, 0x00, 0xff },
    { 0x00, 0xbb, 0x00, 0xff },
    { 0x00, 0x77, 0x00, 0xff },
    { 0x00, 0x99, 0x00, 0xff },
    { 0x00, 0xbb, 0x00, 0xff },
    { 0x00, 0x55, 0x00, 0xff },
    { 0x00, 0xff, 0x00, 0xff }
  }
};



////////////////////////////////////////////////////////////////////////////////
//
//  Helper functions
//
////////////////////////////////////////////////////////////////////////////////

static void cave_setup_window(lua_State *L) {
  SDL_DisplayMode mode;
  int i, width, height;

  if (screen_fullscreen) return;
  width = CAVE_COLS * glyph_width;
  height = CAVE_ROWS * glyph_height;
  if (SDL_RenderSetLogicalSize(renderer, width, height)) luaL_error(L, "SDL_RenderSetLogicalSize() failed: %s", SDL_GetError());
  if ((i = SDL_GetWindowDisplayIndex(window)) < 0) luaL_error(L, "SDL_GetWindowDisplayIndex() failed: %s", SDL_GetError());
  if (SDL_GetDesktopDisplayMode(i, &mode)) luaL_error(L, "SDL_GetDesktopDisplayMode() failed: %s", SDL_GetError());
  while (width > mode.w || height > mode.h) {
    width = (int)((float)width * 0.75f);
    height = (int)((float)height * 0.75f);
  }
  SDL_SetWindowSize(window, width, height);
  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

static int cave_load_font(lua_State *L) {
  char filename[256];
  int x, y, i;
  SDL_Surface *surface;
  SDL_RWops *rw;

  SDL_snprintf(filename, sizeof(filename), "font%d.bmp", font_number);
  if ((rw = SDL_RWFromFile(filename, "rb")) == NULL) return SDL_FALSE;
  if ((surface = SDL_LoadBMP_RW(rw, SDL_TRUE)) == NULL) luaL_error(L, "cannot load font " LUA_QS, filename);
  if (glyph_atlas) SDL_DestroyTexture(glyph_atlas);
  if ((glyph_atlas = SDL_CreateTextureFromSurface(renderer, surface)) == NULL) luaL_error(L, "SDL_CreateTextureFromSurface() failed: %s", SDL_GetError());
  glyph_width = surface->w / CAVE_ATLAS_COLS;
  glyph_height = surface->h / CAVE_ATLAS_ROWS;
  SDL_zero(glyphs);
  for (i = ' ', x = 0, y = 0; i <= 128; ++i) {
    glyphs[i].x = x; glyphs[i].y = y;
    glyphs[i].w = glyph_width; glyphs[i].h = glyph_height;
    x += glyph_width;
    if (x >= surface->w) {
      x = 0;
      y += glyph_height;
    }
  }
  SDL_FreeSurface(surface);
  cave_setup_window(L);
  return SDL_TRUE;
}


////////////////////////////////////////////////////////////////////////////////
//
//  Lua Interface
//
////////////////////////////////////////////////////////////////////////////////

static int l_quit(lua_State *L) {
  (void)L; running = SDL_FALSE; return 0;
}

static int l_randomseed(lua_State *L) {
  if (lua_gettop(L) > 0) randomseed = (Uint32)luaL_checkinteger(L, 1);
  lua_pushinteger(L, randomseed);
  return 1;
}

static int l_random(lua_State *L) {
  double r;
  lua_Integer low, up;
  randomseed ^= randomseed << 13;
  randomseed ^= randomseed >> 17;
  randomseed ^= randomseed << 5;
  r = (double)(randomseed % 100000) / 100000.0;
  switch (lua_gettop(L)) {
  case 0: lua_pushnumber(L, (lua_Number)r); return 1;
  case 1: low = 1; up = luaL_checkinteger(L, 1); break;
  case 2: low = luaL_checkinteger(L, 1); up = luaL_checkinteger(L, 2); break;
  default: return luaL_error(L, "wrong number of arguments");
  }
  luaL_argcheck(L, low <= up, 1, "interval is empty");
  luaL_argcheck(L, low >= 0 || up <= LUA_MAXINTEGER + low, 1, "interval too large");
  r *= (double)(up - low) + 1.0;
  lua_pushinteger(L, (lua_Integer)r + low);
  return 1;
}

static int l_clear(lua_State *L) {
  (void)L;  SDL_zero(characters); return 0;
}

static int l_print(lua_State *L) {
  int x = (int)luaL_checkinteger(L, 1);
  int y = (int)luaL_checkinteger(L, 2);
  int color = (int)luaL_checkinteger(L, 3);
  const char *text = luaL_checkstring(L, 4);
  for (; *text; ++text, ++x) {
    if (x >= 0 && x < CAVE_COLS && y >= 0 && y < CAVE_ROWS) {
      characters[x][y] = *text;
      colors[x][y] = (char)color;
    }
  }
  return 0;
}

static int l_cursor(lua_State *L) {
  cursor_x = (int)luaL_checkinteger(L, 1);
  cursor_y = (int)luaL_checkinteger(L, 2);
  return 0;
}

static const luaL_Reg l__funcs[] = {
  { "quit", l_quit },
  { "randomseed", l_randomseed },
  { "random", l_random },
  { "clear", l_clear },
  { "print", l_print },
  { "cursor", l_cursor },
  { NULL, NULL }
};



////////////////////////////////////////////////////////////////////////////////
//
//  Main Loop
//
////////////////////////////////////////////////////////////////////////////////

static void cave_render_screen() {
  int x, y;
  SDL_Rect rect, *glyph;
  const SDL_Color *color;

  // clear screen
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
  SDL_SetRenderDrawColor(renderer, palette[screen_theme][0].r, palette[screen_theme][0].g, palette[screen_theme][0].b, 255);
  SDL_RenderClear(renderer);
  // render glyphs
  rect.w = glyph_width; rect.h = glyph_height;
  for (y = 0; y < CAVE_ROWS; ++y) {
    rect.y = y * glyph_height;
    for (x = 0; x < CAVE_COLS; ++x) {
      glyph = &glyphs[characters[x][y]];
      if (glyph->w > 0) {
        rect.x = x * glyph_width;
        color = &palette[screen_theme][colors[x][y] & 15];
        SDL_SetTextureColorMod(glyph_atlas, color->r, color->g, color->b);
        SDL_RenderCopy(renderer, glyph_atlas, glyph, &rect);
      }
    }
  }
  // render cursor
  if (cursor_x >= 0 && cursor_x < CAVE_COLS && cursor_y >= 0 && cursor_y < CAVE_ROWS && cursor_visible < CAVE_FPS / 2) {
    color = &palette[screen_theme][colors[cursor_x][cursor_y]];
    rect.x = cursor_x * glyph_width; rect.y = cursor_y * glyph_height;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, 128);
    SDL_RenderFillRect(renderer, &rect);
  }
  SDL_RenderPresent(renderer);
}

static int cave_push_event_handler(lua_State *L, const char *name) {
  if (lua_getglobal(L, "cave") == LUA_TTABLE) {
    if (lua_getfield(L, -1, name) == LUA_TFUNCTION) {
      lua_remove(L, -2); return SDL_TRUE;
    }
    lua_pop(L, 2); return SDL_FALSE;
  }
  lua_pop(L, 1); return SDL_FALSE;
}

static void cave_send_key(lua_State *L, const char *key) {
  if (cave_push_event_handler(L, "on_key")) {
    lua_pushstring(L, key);
    lua_call(L, 1, 0);
  }
}

static void cave_loop(lua_State *L) {
  SDL_Event ev;
  Uint32 delta_ticks, last_tick, current_tick;

  if (cave_push_event_handler(L, "on_init")) lua_call(L, 0, 0);
  delta_ticks = 0; last_tick = SDL_GetTicks();
  while (running) {
    // do events
    while (SDL_PollEvent(&ev)) {
      switch (ev.type) {
      case SDL_QUIT: running = SDL_FALSE; break;
      case SDL_TEXTINPUT: cave_send_key(L, ev.text.text); break;
      case SDL_KEYDOWN:
        switch (ev.key.keysym.sym) {
        case SDLK_UP: cave_send_key(L, "up"); break;
        case SDLK_DOWN: cave_send_key(L, "down"); break;
        case SDLK_LEFT: cave_send_key(L, "left"); break;
        case SDLK_RIGHT: cave_send_key(L, "right"); break;
        case SDLK_ESCAPE: cave_send_key(L, "escape"); break;
        case SDLK_RETURN: cave_send_key(L, "enter"); break;
        case SDLK_BACKSPACE: cave_send_key(L, "backspace"); break;
        case SDLK_TAB: cave_send_key(L, "tab"); break;
        case SDLK_F1: cave_send_key(L, "F1"); break;
        case SDLK_F2: cave_send_key(L, "F2"); break;
        case SDLK_F3: cave_send_key(L, "F3"); break;
        case SDLK_F4: cave_send_key(L, "F4"); break;
        case SDLK_F5: cave_send_key(L, "F5"); break;
        case SDLK_F6: cave_send_key(L, "F6"); break;
        case SDLK_F7: cave_send_key(L, "F7"); break;
        case SDLK_F8: cave_send_key(L, "F8"); break;
        case SDLK_F9: cave_send_key(L, "F9"); break;
        case SDLK_F10:
          ++font_number;
          if (!cave_load_font(L)) font_number = 0;
          cave_load_font(L);
          break;
        case SDLK_F11:
          if (++screen_theme >= CAVE_NUM_THEMES) screen_theme = 0;
          break;
        case SDLK_F12:
          screen_fullscreen = !screen_fullscreen;
          SDL_SetWindowFullscreen(window, screen_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
          break;
        }
        break;
      }
    }
    // do timed updates
    current_tick = SDL_GetTicks();
    delta_ticks += current_tick - last_tick;
    last_tick = current_tick;
    for (; delta_ticks >= CAVE_FPS_TICKS; delta_ticks -= CAVE_FPS_TICKS) {
      if (++cursor_visible >= CAVE_FPS) cursor_visible = 0;
      if (cave_push_event_handler(L, "on_update")) lua_call(L, 0, 0);
    }
    // render screen
    cave_render_screen();
  }
}


////////////////////////////////////////////////////////////////////////////////
//
//  Init / Shutdown
//
////////////////////////////////////////////////////////////////////////////////

static int cave_init(lua_State *L) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS)) luaL_error(L, "SDL_Init() failed: %s", SDL_GetError());
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
  if ((window = SDL_CreateWindow("Cave Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_RESIZABLE)) == NULL) luaL_error(L, "SDL_CreateWindow() failed: %s", SDL_GetError());
  if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)) == NULL) luaL_error(L, "SDL_CreateRenderer() failed: %s", SDL_GetError());
  if (!cave_load_font(L)) luaL_error(L, "Cannot load any font");
  if (luaL_loadfile(L, "game.lua") != LUA_OK) lua_error(L);
  lua_call(L, 0, 0);
  cave_loop(L);
  return 0;
}

static void cave_shutdown() {
  if (renderer != NULL) SDL_DestroyRenderer(renderer);
  if (window != NULL) SDL_DestroyWindow(window);
  SDL_Quit();
}

static int cave_open(lua_State *L) {
  luaL_newlib(L, l__funcs);
  return 1;
}

int main(int argc, char **argv) {
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  luaL_requiref(L, "cave", cave_open, 1);
  lua_getglobal(L, "debug"); lua_getfield(L, -1, "traceback"); lua_remove(L, -2);
  lua_pushcfunction(L, cave_init);
  if (lua_pcall(L, 0, 0, -2) != LUA_OK) {
    const char *msg = luaL_gsub(L, lua_tostring(L, -1), "\t", "  ");
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Cave Panic!", msg, window);
#ifndef _WIN32
    fprintf(stderr, "Cave Panic!\n%s\n", msg);
#endif // _WIN32
  }
  lua_close(L);
  cave_shutdown();
  return 0;
}
