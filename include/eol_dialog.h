#ifndef __EOL_DIALOG__
#define __EOL_DIALOG__
/*
    Copyright 2012 Engineer of Lies
    This file is part of the Engine of Lies game engine library

    The Engine of Lies (EOL) is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    EOL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with the EOL game engine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "eol_window.h"
/**
 * @purpose To provide common window dialogs useful for any game or editor
 */

/**
  * @brief pops up a text dialog box with an "OK"
  *
  * @param question the text to show in the dialog box
  */
void eol_dialog_text_block(eolLine title,
                           char   *text,
                           eolWord okText,
                           void    *data,
                           eolWindowCallback onOK);

/**
 * @brief pops up a yes / no window dialog.
 *
 * @param question the text to show in the dialog box
 * @param customData if the callbacks need to work on custom data, it can be
                     passed though this pointer
 * @param onYes the window callback to call when the YES option is selected
 * @param onNo  the window callback to call when the NO  option is selected
 */
void eol_dialog_yes_no(eolLine question,
                       void *customData,
                       eolWindowCallback onYes,
                       eolWindowCallback onNo);

/**
 * @brief pops up a yes/no dialog asking if the user wants to exit the program.
 *        it will call exit(0) on yes.
 */
void eol_dialog_quit();

#endif