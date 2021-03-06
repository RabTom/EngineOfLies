#include "eol_window.h"
#include "eol_sprite.h"
#include "eol_component.h"
#include "eol_resource.h"
#include "eol_logger.h"
#include "eol_config.h"

/*local global variables*/
eolBool              _eol_window_initialized = eolFalse;
eolResourceManager * _eol_window_manager = NULL;
eolUint              _eol_window_max = 0;
/**<its not really a stack as per the data structure, just a draw order*/
GList              * _eol_window_stack = NULL;
eolLine              _eol_window_sprite_files[9];
eolSprite          * _eol_window_sprites[9];
#define _eol_window_sprite_count  9

/*local function prototypes*/
void eol_window_delete(void *win);
eolBool eol_window_load_data_from_file(char * filename,void *data);
void eol_window_draw_generic(eolWindow *win);

/*definitions*/
void eol_window_init()
{
  int i;
  eol_logger_message(
      EOL_LOG_INFO,
      "eol_window:initializing\n");
  _eol_window_manager = eol_resource_manager_init(
    "eol_window_manager",
    _eol_window_max,
    sizeof(eolWindow),
    eolTrue,
    eol_window_delete,
    eol_window_load_data_from_file
    );
  eol_logger_message(
      EOL_LOG_INFO,
      "eol_window:initialized\n");
  atexit(eol_window_close);
  for (i = 0; i < _eol_window_sprite_count;i++)
  {
    if (strlen(_eol_window_sprite_files[i]) > 0)
    {
      _eol_window_sprites[i] = eol_sprite_load(_eol_window_sprite_files[i],-1,-1);
    }
  }
  _eol_window_initialized = eolTrue;
  eol_component_config();
}

eolBool eol_window_initialized()
{
  return _eol_window_initialized;
}


void eol_window_config()
{
  /*TODO load from config*/
  _eol_window_max = 16;
  strcpy(_eol_window_sprite_files[0],"images/UI/windowbackground1.png");
  strcpy(_eol_window_sprite_files[1],"images/UI/windowborder_corner_UL.png");
  strcpy(_eol_window_sprite_files[2],"images/UI/windowborder_corner_UR.png");
  strcpy(_eol_window_sprite_files[3],"images/UI/windowborder_corner_BL.png");
  strcpy(_eol_window_sprite_files[4],"images/UI/windowborder_corner_BR.png");
  strcpy(_eol_window_sprite_files[5],"images/UI/windowborder_horiz.png");
  strcpy(_eol_window_sprite_files[6],"images/UI/windowborder_vert.png");
  strcpy(_eol_window_sprite_files[7],"images/UI/windowborder_horiz.png");
  strcpy(_eol_window_sprite_files[8],"images/UI/windowborder_vert.png");
}

void eol_window_clean()
{
  if (!eol_window_initialized())
  {
    eol_logger_message(
        EOL_LOG_INFO,
        "eol_window:cannot clean, not initialized\n");
    return;
  }
  if (!_eol_window_manager)return;
  eol_resource_manager_clean(_eol_window_manager);
}

void eol_window_close()
{
  eol_logger_message(
      EOL_LOG_INFO,
      "eol_window:closing\n"
  );
  eol_resource_manager_free(&_eol_window_manager);
  _eol_window_max = 0;
  _eol_window_initialized = eolFalse;
  eol_logger_message(
      EOL_LOG_INFO,
      "eol_window:closed\n"
  );
}

void eol_window_draw_all()
{
  GList *l = NULL;
  GList *c = NULL;
  eolWindow *win;
  if (!eol_window_initialized())return;
  for (l = _eol_window_stack;l != NULL; l = l->next)
  {
    win = (eolWindow*)l->data;
    if (win == NULL)continue;
    if (win->drawGeneric)
    {
      eol_window_draw_generic(win);
    }
    if (win->draw != NULL)
    {
      win->draw(win);
    }
    for (c = win->components;c != NULL;c = c->next)
    {
      if (c->data == NULL)continue;
      eol_component_draw((eolComponent *)c->data,win->rect);
    }
  }
}
void eol_window_update_all()
{
  GList *l = NULL;
  GList *c = NULL;
  GList *update = NULL;
  eolWindow *win;
  if (!eol_window_initialized())return;
  l = g_list_last(_eol_window_stack);
  if (l != NULL)
  {
    win = (eolWindow*)l->data;
    if ((win != NULL)&&(win->update != NULL))
    {
      /*update all components*/
      for (c = win->components;c != NULL; c= c->next)
      {
        if (c->data != NULL)
        {
          if (eol_component_update(c->data))
          {
            update = g_list_append(update,c->data);
          }
        }
      }
    }
    /*call update for window*/
    win->update(win,update);
    if (update != NULL)
    {
      g_list_free(update);
    }
  }
}

eolWindow *eol_window_new()
{
  eolWindow *win = NULL;
  if (!eol_window_initialized())
  {
    eol_logger_message(
      EOL_LOG_INFO,
      "eol_window:used uninitialized\n");
    return NULL;
  }
  win = (eolWindow *)eol_resource_new_element(_eol_window_manager);
  if (win == NULL)
  {
    eol_logger_message(
      EOL_LOG_INFO,
      "eol_window:failed to get new resource\n");
    return NULL;
  }
  /*new windows are added at the top of the stack, ie: last to draw*/
  _eol_window_stack = g_list_append(_eol_window_stack,win);
  return win;
}

void eol_window_free(eolWindow **win)
{
  if (!eol_window_initialized())return;
  _eol_window_stack = g_list_remove(_eol_window_stack,*win);
  eol_resource_free_element(_eol_window_manager,(void **)win);
}

void eol_window_delete(void *window)
{
  GList *l = NULL;
  eolWindow *win;
  if (!eol_window_initialized())return;
  if (window)return;
  win = (eolWindow *)window;
  if (win->customData != NULL)
  {
    if (win->custom_delete != NULL)
    {
      win->custom_delete(win->customData);
    }
  }
  if (win->components != NULL)
  {
    for (l = win->components;l != NULL;l = l->next)
    {
      if (l->data != NULL)
      {
        eol_component_free((eolComponent**)&l->data);
      }
    }
    g_list_free(win->components);
    win->components = NULL;
  }
  if (win->callbacks != NULL)
  {
    free(win->callbacks);
  }
  _eol_window_stack = g_list_remove(_eol_window_stack,win);
  memset(win, 0, sizeof(eolWindow));
}

eolBool eol_window_load_data_from_file(char * filename,void *data)
{
  return eolTrue;
}

void eol_window_draw_generic(eolWindow *win)
{
  eolFloat scaleX = 1,scaleY = 1;
  if (!win)return;
  if (!eol_window_initialized())return;
  /*draw the background*/
  if ((_eol_window_sprites[0] != NULL) &&
    (_eol_window_sprites[0]->frameWidth != 0) &&
    (_eol_window_sprites[0]->frameHeight != 0))
  {
    scaleX = win->rect.w / (float)_eol_window_sprites[0]->frameWidth;
    scaleY = win->rect.h / (float)_eol_window_sprites[0]->frameHeight;
    eol_sprite_draw_transformed(_eol_window_sprites[0],0,win->rect.x,win->rect.y,
                                scaleX,scaleY,0,0,0,
                                eol_vec3d(1,1,1),1);
  }
  /*draw the borders*/
  /*top*/
  if ((_eol_window_sprites[5] != NULL) &&
    (_eol_window_sprites[5]->frameWidth != 0) &&
    (_eol_window_sprites[5]->frameHeight != 0))
  {
    scaleX = win->rect.w / (float)_eol_window_sprites[5]->frameWidth;
    eol_sprite_draw_transformed(_eol_window_sprites[5],0,win->rect.x,
                                win->rect.y - _eol_window_sprites[5]->frameHeight,
                                scaleX,1,0,0,0,
                                eol_vec3d(1,1,1),1);
  }
  /*left*/
  if ((_eol_window_sprites[6] != NULL) &&
    (_eol_window_sprites[6]->frameWidth != 0) &&
    (_eol_window_sprites[6]->frameHeight != 0))
  {
    scaleY = win->rect.h / (float)_eol_window_sprites[6]->frameHeight;
    eol_sprite_draw_transformed(_eol_window_sprites[6],0,
                                win->rect.x - _eol_window_sprites[6]->frameWidth,
                                win->rect.y,
                                1,scaleY,0,0,0,
                                eol_vec3d(1,1,1),1);
  }
  /*bottom*/
  if ((_eol_window_sprites[7] != NULL) &&
    (_eol_window_sprites[7]->frameWidth != 0) &&
    (_eol_window_sprites[7]->frameHeight != 0))
  {
    scaleX = win->rect.w / (float)_eol_window_sprites[7]->frameWidth;
    eol_sprite_draw_transformed(_eol_window_sprites[7],0,win->rect.x,
                                win->rect.y + win->rect.h,
                                scaleX,1,0,0,0,
                                eol_vec3d(1,1,1),1);
  }
  /*right*/
  if ((_eol_window_sprites[8] != NULL) &&
    (_eol_window_sprites[8]->frameWidth != 0) &&
    (_eol_window_sprites[8]->frameHeight != 0))
  {
    scaleY = win->rect.h / (float)_eol_window_sprites[8]->frameHeight;
    eol_sprite_draw_transformed(_eol_window_sprites[8],0,
                                win->rect.x + win->rect.w,
                                win->rect.y,
                                1,scaleY,0,0,0,
                                eol_vec3d(1,1,1),1);
  }
  
  /*draw the corner stones*/
  /*UL*/
  if ((_eol_window_sprites[1] != NULL) &&
    (_eol_window_sprites[1]->frameWidth != 0) &&
    (_eol_window_sprites[1]->frameHeight != 0))
  {
    eol_sprite_draw(_eol_window_sprites[1],
                    0,
                    (win->rect.x - _eol_window_sprites[1]->frameWidth),
                    (win->rect.y - _eol_window_sprites[1]->frameHeight));
  }
  /*UR*/
  if ((_eol_window_sprites[2] != NULL) &&
    (_eol_window_sprites[2]->frameWidth != 0) &&
    (_eol_window_sprites[2]->frameHeight != 0))
  {
    eol_sprite_draw(_eol_window_sprites[2],
                    0,
                    (win->rect.x + win->rect.w),
                    (win->rect.y - _eol_window_sprites[2]->frameHeight));
  }
  /*BL*/
  if ((_eol_window_sprites[3] != NULL) &&
    (_eol_window_sprites[3]->frameWidth != 0) &&
    (_eol_window_sprites[3]->frameHeight != 0))
  {
    eol_sprite_draw(_eol_window_sprites[3],
                    0,
                    (win->rect.x - _eol_window_sprites[3]->frameWidth),
                    (win->rect.y + win->rect.h));
  }
  /*BR*/
  if ((_eol_window_sprites[4] != NULL) &&
    (_eol_window_sprites[4]->frameWidth != 0) &&
    (_eol_window_sprites[4]->frameHeight != 0))
  {
    eol_sprite_draw(_eol_window_sprites[4],
                    0,
                    (win->rect.x + win->rect.w),
                    (win->rect.y + win->rect.h));
  }
}

void eol_window_add_component(eolWindow *win,eolComponent *comp)
{
  if ((win == NULL)||(comp == NULL)||(!eol_window_initialized()))
  {
    return;
  }
  win->components = g_list_append(win->components,comp);
  if (win->components == NULL)
  {
    eol_logger_message(
      EOL_LOG_ERROR,
      "eol_window:unable to add new component!\n");
  }
}

eolComponent *eol_window_get_component_by_id(eolWindow *win,eolUint id)
{
  GList *c = NULL;
  eolComponent *comp;
  if (!win)return NULL;
  for (c = win->components;c != NULL;c = c->next)
  {
    if (c->data != NULL)
    {
      comp = (eolComponent *)c->data;
      if (comp->id == id)return comp;
    }
  }
  return NULL;
}

void eol_window_allocat_callbacks(eolWindow *win,eolUint count)
{
  if (!win)return;
  if (win->callbacks != NULL)
  {
    eol_logger_message(
      EOL_LOG_ERROR,
      "eol_window:attempted to re-allocated window callbacks for window %s\n",
      win->name);
    return;
  }
  win->callbacks = (eolWindowCallback*)malloc(sizeof(eolWindowCallback)*count);
  if (win->callbacks == NULL)
  {
    eol_logger_message(
      EOL_LOG_ERROR,
      "eol_window:cannot allocate callbacks for window %s\n",
      win->name);
    return;
  }
  memset(win->callbacks,0,sizeof(eolWindowCallback)*count);
  win->callbackCount = count;
}

eolFloat eol_window_get_relative_position(eolInt position,eolUint range)
{
  if (range == 0)return 0;
  return (eolFloat)position/(eolFloat)range;
}

/*eol@eof*/
