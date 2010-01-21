/*
 * File: ftk_entry.c    
 * Author:  Li XianJing <xianjimli@hotmail.com>
 * Brief:   single line editor
 *
 * Copyright (c) 2009  Li XianJing <xianjimli@hotmail.com>
 *
 * Licensed under the Academic Free License version 2.1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * History:
 * ================================================================
 * 2009-11-02 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "ftk_util.h"
#include "ftk_entry.h"
#include "ftk_globals.h"
#include "ftk_text_buffer.h"
#include "ftk_source_timer.h"

typedef struct _PrivInfo
{
	int   caret;
	int   caret_visible;
	int   visible_start;
	int   visible_end;
	int   selected_start;
	int   selected_end;
	int   preedit_start;
	int   preedit_end;
	FtkSource* caret_timer;
	FtkTextBuffer* text_buffer;
}PrivInfo;

#define FTK_ENTRY_LEFT_MARGIN 4
#define FTK_ENTRY_TOP_MARGIN  5

#define TB_TEXT priv->text_buffer->buffer
#define TB_LENGTH (int)(priv->text_buffer->length)
#define HAS_TEXT(priv) (priv != NULL && priv->text_buffer != NULL && TB_LENGTH > 0) 

static Ret ftk_entry_on_paint_caret(FtkWidget* thiz);
static Ret ftk_entry_compute_visible_range(FtkWidget* thiz);

static Ret ftk_entry_move_caret(FtkWidget* thiz, int offset)
{
	DECL_PRIV0(thiz, priv);
	priv->caret_visible = 0;
	ftk_entry_on_paint_caret(thiz);

	if(!HAS_TEXT(priv))
	{
		priv->caret = 0;
		return RET_OK;
	}

	priv->caret += ftk_text_buffer_chars_bytes(priv->text_buffer, priv->caret, offset);
	priv->caret = priv->caret < 0 ? 0 : priv->caret;
	priv->caret = priv->caret >  TB_LENGTH ? TB_LENGTH : priv->caret;

	if((priv->caret) < priv->visible_start)
	{
		priv->visible_start = priv->caret;
		priv->visible_end = -1;
	}
	else if((priv->caret) > priv->visible_end)
	{
		priv->visible_end = priv->caret;
		priv->visible_start = -1;
	}

	ftk_widget_invalidate(thiz);

	return RET_OK;
}

static Ret ftk_entry_get_offset_by_pointer(FtkWidget* thiz, int x)
{
	DECL_PRIV0(thiz, priv);
	int offset = 0;
	const char* end = NULL;
	FtkCanvas* canvas = ftk_widget_canvas(thiz);
	int width = x - ftk_widget_left(thiz) - FTK_ENTRY_LEFT_MARGIN;
	return_val_if_fail(width >= 0, RET_FAIL);

	end = ftk_canvas_calc_str_visible_range(canvas, TB_TEXT, priv->visible_start, -1, width);
	offset = end - TB_TEXT - priv->caret;

	if(offset == 0)
	{
		return RET_OK;
	}
	else if(offset < 0)
	{
		offset = - utf8_count_char(end, -offset);
	}
	else if(offset > 0)
	{
		offset = utf8_count_char(TB_TEXT + priv->caret, offset);
	}
	
	return ftk_entry_move_caret(thiz, offset);
}

static Ret ftk_entry_handle_mouse_evevnt(FtkWidget* thiz, FtkEvent* event)
{
	return ftk_entry_get_offset_by_pointer(thiz, event->u.mouse.x);
}

static Ret ftk_entry_input_char(FtkWidget* thiz, char c)
{
	char str[2] = {0};
	DECL_PRIV0(thiz, priv);
	
	str[0] = c;
	ftk_text_buffer_insert(priv->text_buffer, priv->caret, str);
	ftk_entry_move_caret(thiz, 1);	

	return RET_OK;
}

static Ret ftk_entry_handle_key_event(FtkWidget* thiz, FtkEvent* event)
{
	Ret ret = RET_OK;
	DECL_PRIV0(thiz, priv);

	switch(event->u.key.code)
	{
		case FTK_KEY_HOME:
		{
			ftk_entry_move_caret(thiz, -priv->caret);
			break;
		}
		case FTK_KEY_END:
		{
			ftk_entry_move_caret(thiz, TB_LENGTH-priv->caret);
			break;
		}
		case FTK_KEY_LEFT:
		{
			ftk_entry_move_caret(thiz, -1);
			ret = RET_REMOVE;
			break;
		}
		case FTK_KEY_RIGHT:
		{
			ftk_entry_move_caret(thiz, 1);
			ret = RET_REMOVE;
			break;
		}
		case FTK_KEY_UP:
		case FTK_KEY_DOWN:break;
		case FTK_KEY_DELETE:
		{
			ftk_text_buffer_delete_chars(priv->text_buffer, priv->caret, 1);
			ftk_entry_move_caret(thiz, 0);
			break;
		}
		case FTK_KEY_BACKSPACE:
		{
			int caret = priv->caret;
			ftk_entry_move_caret(thiz, -1);
			if(ftk_text_buffer_delete_chars(priv->text_buffer, caret, -1) == RET_OK)
			{
			}
			break;
		}
		default:
		{
			if(event->u.key.code < 0xff && isprint(event->u.key.code))
			{
				ftk_entry_input_char(thiz, event->u.key.code);
			}
			break;
		}
	}

	return ret;
}

static Ret ftk_entry_on_event(FtkWidget* thiz, FtkEvent* event)
{
	Ret ret = RET_OK;
	DECL_PRIV0(thiz, priv);
	return_val_if_fail(thiz != NULL && event != NULL, RET_FAIL);

	switch(event->type)
	{
		case FTK_EVT_FOCUS_IN:
		{
			ftk_source_ref(priv->caret_timer);
			ftk_main_loop_add_source(ftk_default_main_loop(), priv->caret_timer);
			break;
		}
		case FTK_EVT_FOCUS_OUT:
		{
			ftk_main_loop_remove_source(ftk_default_main_loop(), priv->caret_timer);
			break;
		}
		case FTK_EVT_KEY_DOWN:
		case FTK_EVT_KEY_UP:
		{
			if(event->type == FTK_EVT_KEY_DOWN)
			{
				ret = ftk_entry_handle_key_event(thiz, event);
			}
			else
			{
				ret = event->u.key.code == FTK_KEY_LEFT || event->u.key.code == FTK_KEY_RIGHT 
					? RET_REMOVE : RET_OK;
			}
			break;
		}
		case FTK_EVT_MOUSE_UP:
		{
			ret = ftk_entry_handle_mouse_evevnt(thiz, event);
			break;
		}
		default:break;
	}

	return ret;
}

static Ret ftk_entry_on_paint_caret(FtkWidget* thiz)
{
	int extent = 0;
	DECL_PRIV0(thiz, priv);
	FtkGc gc = {0};
	FTK_BEGIN_PAINT(x, y, width, height, canvas);
	return_val_if_fail(thiz != NULL, RET_FAIL);

	return_val_if_fail(priv->caret >= priv->visible_start && priv->caret <= priv->visible_end, RET_FAIL);

	(void)width;
	gc.mask = FTK_GC_FG;
	if(ftk_widget_is_focused(thiz))
	{
		gc.fg = priv->caret_visible ? ftk_widget_get_gc(thiz)->fg : ftk_widget_get_gc(thiz)->bg;
		priv->caret_visible = !priv->caret_visible;

		extent = ftk_canvas_get_extent(canvas, TB_TEXT+priv->visible_start, priv->caret - priv->visible_start);

		ftk_canvas_reset_gc(canvas, &gc);
		x += extent + FTK_ENTRY_LEFT_MARGIN - 1;
		y += FTK_ENTRY_TOP_MARGIN;
		ftk_canvas_draw_vline(canvas, x, y, height - 2 * FTK_ENTRY_TOP_MARGIN);
		FTK_END_PAINT();
	}

	return RET_OK;
}

static Ret ftk_entry_on_paint(FtkWidget* thiz)
{
	FtkGc gc = {0};
	DECL_PRIV0(thiz, priv);
	int font_height = 0;
	FTK_BEGIN_PAINT(x, y, width, height, canvas);

	gc.mask = FTK_GC_FG;
	gc.fg = ftk_theme_get_border_color(ftk_default_theme(), FTK_ENTRY, ftk_widget_state(thiz));
	ftk_canvas_set_gc(canvas, &gc);
	ftk_canvas_draw_hline(canvas, x + 2, y, width-4);
	ftk_canvas_draw_hline(canvas, x + 1, y + 1, width-2);
	ftk_canvas_draw_vline(canvas, x, y + 2, height - 4);
	ftk_canvas_draw_vline(canvas, x + 1, y + 1, height - 2);
	
	ftk_canvas_draw_vline(canvas, x+width-1, y + 2, height - 4);
	ftk_canvas_draw_vline(canvas, x + width -2, y + 1, height - 2);
	ftk_canvas_draw_hline(canvas, x + 1, y + height - 1, width-2);
	ftk_canvas_draw_hline(canvas, x + 2, y + height - 2, width-4);

	ftk_canvas_set_gc(canvas, ftk_widget_get_gc(thiz)); 
	if(priv->text_buffer != NULL)
	{
		ftk_entry_compute_visible_range(thiz);
		font_height = ftk_canvas_font_height(canvas);
		x += FTK_ENTRY_LEFT_MARGIN;
		y += font_height + FTK_ENTRY_TOP_MARGIN;

		ftk_canvas_draw_string(canvas, x, y, TB_TEXT + priv->visible_start,
			priv->visible_end - priv->visible_start);
	}
	ftk_entry_on_paint_caret(thiz);

	FTK_END_PAINT();
}

static void ftk_entry_destroy(FtkWidget* thiz)
{
	if(thiz != NULL)
	{
		DECL_PRIV0(thiz, priv);
	
		ftk_source_disable(priv->caret_timer);
		ftk_main_loop_remove_source(ftk_default_main_loop(), priv->caret_timer);
		ftk_source_unref(priv->caret_timer);
		ftk_text_buffer_destroy(priv->text_buffer);
		FTK_FREE(priv);
	}

	return;
}

FtkWidget* ftk_entry_create(FtkWidget* parent, int x, int y, int width, int height)
{
	FtkWidget* thiz = (FtkWidget*)FTK_ZALLOC(sizeof(FtkWidget));
	return_val_if_fail(thiz != NULL, NULL);

	thiz->priv_subclass[0] = (PrivInfo*)FTK_ZALLOC(sizeof(PrivInfo));
	if(thiz->priv_subclass[0] != NULL)
	{
		DECL_PRIV0(thiz, priv);

		thiz->on_event = ftk_entry_on_event;
		thiz->on_paint = ftk_entry_on_paint;
		thiz->destroy  = ftk_entry_destroy;

		height = ftk_font_height(ftk_default_font()) + FTK_ENTRY_TOP_MARGIN * 2;
		ftk_widget_init(thiz, FTK_ENTRY, 0);
		ftk_widget_move(thiz, x, y);
		ftk_widget_resize(thiz, width, height);
		ftk_widget_set_attr(thiz, FTK_ATTR_TRANSPARENT|FTK_ATTR_BG_FOUR_CORNER);

		priv->caret_timer = ftk_source_timer_create(500, (FtkTimer)ftk_entry_on_paint_caret, thiz);
		priv->text_buffer = ftk_text_buffer_create(128);
		ftk_widget_append_child(parent, thiz);
	}
	else
	{
		FTK_FREE(thiz);
	}

	return thiz;
}

static Ret ftk_entry_compute_visible_range(FtkWidget* thiz)
{
	DECL_PRIV0(thiz, priv);
	int width = ftk_widget_width(thiz);
	FtkCanvas* canvas = ftk_widget_canvas(thiz);
	const char* other_side = NULL;
	return_val_if_fail(thiz != NULL && priv->text_buffer != NULL, RET_FAIL);

	if(priv->visible_start < 0 || priv->visible_end < 0)
	{
		other_side = ftk_canvas_calc_str_visible_range(canvas, TB_TEXT, 
			priv->visible_start, priv->visible_end, width - 2 * FTK_ENTRY_LEFT_MARGIN);

		if(priv->visible_start < 0)
		{
			priv->visible_start = other_side - TB_TEXT;
		}
		else if(priv->visible_end < 0)
		{
			priv->visible_end = other_side - TB_TEXT;
		}
	}

	return RET_OK;
}

Ret ftk_entry_set_text(FtkWidget* thiz, const char* text)
{
	DECL_PRIV0(thiz, priv);
	return_val_if_fail(thiz != NULL && text != NULL, RET_FAIL);

	ftk_text_buffer_delete(priv->text_buffer, 0, TB_LENGTH);
	ftk_text_buffer_insert(priv->text_buffer, 0, text);	
	priv->visible_start = -1;
	priv->visible_end = TB_LENGTH;
	priv->caret = priv->visible_end;
	ftk_widget_invalidate(thiz);

	return RET_OK;
}

Ret ftk_entry_insert_text(FtkWidget* thiz, size_t pos, const char* text)
{
	DECL_PRIV0(thiz, priv);
	return_val_if_fail(thiz != NULL && text != NULL, RET_FAIL);

	pos = pos < TB_LENGTH ? pos : TB_LENGTH;
	ftk_text_buffer_insert(priv->text_buffer, pos, text);
	priv->visible_start = -1;
	priv->visible_end = TB_LENGTH;
	priv->caret = priv->visible_end;
	ftk_widget_invalidate(thiz);

	return RET_OK;
}

const char* ftk_entry_get_text(FtkWidget* thiz)
{
	DECL_PRIV0(thiz, priv);
	return_val_if_fail(thiz != NULL, NULL);

	return TB_TEXT;
}

