/*
 * File: ftk_tab.h    
 * Author:  Li XianJing <xianjimli@hotmail.com>
 * Brief:   tab widget
 *
 * Copyright (c) 2009 - 2010  Li XianJing <xianjimli@hotmail.com>
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
 * 2010-09-23 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef FTK_TAB_H
#define FTK_TAB_H

#include "ftk_widget.h"

FTK_BEGIN_DECLS

enum FtkTabpageStyle
{
	FTK_TABPAGE_TOP,
	FTK_TABPAGE_LEFT,
	FTK_TABPAGE_BOTTOM,
	FTK_TABPAGE_RIGHT,
};

enum FtkTabImageType
{
    FTK_TAB_IMG_NORMAL,
    FTK_TAB_IMG_PRESSED,
    FTK_TAB_IMG_SELECTED,
    FTK_TAB_IMG_BAR_SELECTED,
    FTK_TAB_IMG_BAR_PRESSED,
};

FtkWidget* ftk_tab_create_ex(FtkWidget* parent,
        int x, int y, int width, int height,
        int tab_width, int tab_height, int style);
FtkWidget* ftk_tab_create(FtkWidget* parent, int x, int y, int width, int height);
FtkWidget* ftk_tab_page_create(FtkWidget* parent, int x, int y, int width, int height);

int     ftk_tab_get_page_count(FtkWidget* thiz);
FtkWidget* ftk_tab_get_page(FtkWidget* thiz, int index);
int        ftk_tab_get_page_index(FtkWidget* thiz, FtkWidget* page);
Ret        ftk_tab_remove_page(FtkWidget* thiz, int index);
FtkWidget* ftk_tab_add_page(FtkWidget* thiz, const char* text, FtkBitmap* icon);

/*which: enum FtkTabImageType */
int ftb_tab_set_img(FtkWidget* thiz, FtkBitmap* bitmap, int which);

/**NOTE: the following is not same with 
 *  ftk_widget_width
 *  ftk_widget_height
 */
int ftk_tab_get_width(FtkWidget* thiz);
int ftk_tab_get_height(FtkWidget* thiz);

int ftk_tab_get_active_page(FtkWidget* thiz);
Ret ftk_tab_set_active_page(FtkWidget* thiz, int index);
Ret ftk_tab_set_page_text(FtkWidget* thiz, int index, const char* text);
Ret ftk_tab_set_page_icon(FtkWidget* thiz, int index, FtkBitmap* icon);

const char* ftk_tab_get_page_text(FtkWidget* thiz, int index);
FtkBitmap* ftk_tab_get_page_icon(FtkWidget* thiz, int index);

FTK_END_DECLS

#endif/*FTK_TAB_H*/

