
#if defined(unix) || defined(__unix__) || defined(__unix)
#include <dirent.h>
#else
#error No dirent.h on non-Unix platforms, aborting.
#endif
#include <vector>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "util/log.hpp"
#include "video/sdl/sdl_window.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"

std::unique_ptr<SDLWindow> g_window;
std::vector<std::string> g_pics;
size_t g_current_pic = 0;
float g_zoom = 1.f;
bool g_pressed = false;
Vector g_camera;
Vector g_mouse_pos;

bool load_pics(std::string folder)
{
  DIR *dir;
  dirent *ent;

  dir = opendir(folder.c_str());

  if (!dir)
    return false;

  while ((ent = readdir(dir)) != NULL)
    if (std::string(ent->d_name) != "." && std::string(ent->d_name) != "..")
      g_pics.push_back(folder + "/" + std::string(ent->d_name));

  closedir(dir);

  return true;
}

void adjust_zoom()
{
  try
  {
    auto s = g_window->get_size();
    auto ts = g_window->load_texture(g_pics[g_current_pic]).get_size();
    g_zoom = std::min(s.w / ts.w, s.h / ts.h);
  }
  catch(...)
  {
    g_zoom = 1.f;
  }
}

bool loop()
{
  SDL_Event e;
  while(SDL_PollEvent(&e))
  {
    switch(e.type)
    {
      case SDL_QUIT:
        return false;

      case SDL_MOUSEBUTTONDOWN:
        g_pressed = true;
        break;

      case SDL_MOUSEBUTTONUP:
        g_pressed = false;
        break;

      case SDL_MOUSEMOTION:
        g_mouse_pos = Vector(e.motion.x, e.motion.y);
        if (g_pressed)
        {
          g_camera += Vector(e.motion.xrel, e.motion.yrel);
        }
        break;

      case SDL_KEYDOWN:
        switch(e.key.keysym.sym)
        {
          case SDLK_LEFT:
            if (!g_current_pic--)
            {
              g_current_pic = g_pics.size() - 1;
            }
            adjust_zoom();
            g_camera = Vector();
            break;

          case SDLK_RIGHT:
            if (++g_current_pic >= g_pics.size())
            {
              g_current_pic = 0;
            }
            adjust_zoom();
            g_camera = Vector();
            break;

          default:
            break;
        }
        break;

      case SDL_WINDOWEVENT:
        switch (e.window.type)
        {
          case SDL_WINDOWEVENT_RESIZED:
            adjust_zoom();
            break;

          default:
            break;
        }
        break;

      case SDL_MOUSEWHEEL:
        try
        {
          auto ts = g_window->load_texture(g_pics[g_current_pic]).get_size();
          float old_zoom = g_zoom;
          g_zoom *= SDL_pow(2, e.wheel.y / 10.f);
          Vector point_on_image = (g_mouse_pos - g_camera - g_window->get_size() / 2.f) / old_zoom;
          g_camera = -(point_on_image * g_zoom + g_window->get_size() / 2.f - g_mouse_pos);
        }
        catch (const std::exception& e)
        {
        }
        break;

      default:
        break;
    }
  }

  DrawingContext dc(g_window->get_renderer());
  dc.draw_filled_rect(g_window->get_size(), Color(0.f, 0.f, 0.f), Renderer::Blend::BLEND, -9999);

  try
  {
    auto& t = g_window->load_texture(g_pics[g_current_pic]);
    dc.draw_texture(t, t.get_size(), Rect(Vector(g_window->get_size() / 2.f) + g_camera - Vector(t.get_size() * g_zoom / 2), t.get_size() * g_zoom), 0.f, Color(1.f, 1.f, 1.f), Renderer::Blend::BLEND, 1);
  }
  catch(std::exception& /* e */)
  {
    dc.draw_text("Cannot render " + g_pics[g_current_pic], g_window->get_size() / 2.f, Renderer::TextAlign::CENTER, "../data/fonts/SuperTux-Medium.ttf", 16, Color(1.f, 1.f, 1.f), Renderer::Blend::BLEND, 10);
  }

  dc.render();

  return true;
}

int main()
{
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

  SDL_Init(SDL_INIT_VIDEO);
  IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP);
  TTF_Init();

  g_window = std::make_unique<SDLWindow>();
  g_window->set_resizable(true);
  g_window->set_title("Pictures");

  if (load_pics("/home/user/Pictures"))
  {
    adjust_zoom();

    while (loop())
      ;
  }

  Font::flush_fonts();

  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}
