/* 
 * The contents of this file are subject to the Mozilla Public 
 * License Version 1.1 (the "License"); you may not use this file 
 * except in compliance with the License. You may obtain a copy of 
 * the License at http://www.mozilla.org/MPL/ 
 * 
 * Software distributed under the License is distributed on an "AS 
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or 
 * implied. See the License for the specific language governing 
 * rights and limitations under the License. 
 * 
 * The Original Code is Vision. 
 * 
 * The Initial Developer of the Original Code is The Vision Team.
 * Portions created by The Vision Team are
 * Copyright (C) 1999, 2000, 2001 The Vision Team.  All Rights
 * Reserved.
 * 
 * Contributor(s): Rene Gollent
 *                 Todd Lair
 */

#include "PrefFont.h"
#include "Vision.h"

#include <ctype.h>
#include <stdlib.h>

#include <ScrollView.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <MessageFilter.h>
#include <TextControl.h>

const uint32 M_FONT_ELEMENT_CHANGE = 'mfec';
const uint32 M_FONT_CHANGE = 'mffc';
const uint32 M_FONT_SIZE_CHANGE = 'mfsc';

struct FontStat
{
  font_family				family;
  int32						style_count;
  font_style				*styles;
};

class NumericFilter : public BMessageFilter
{
  public:
    NumericFilter (void);
    virtual ~NumericFilter (void);
    virtual filter_result Filter (BMessage *, BHandler **);
};

NumericFilter::NumericFilter (void)
    : BMessageFilter (B_ANY_DELIVERY, B_ANY_SOURCE)
{
}

NumericFilter::~NumericFilter (void)
{
}

filter_result 
NumericFilter::Filter (BMessage *msg, BHandler **)
{
  filter_result result = B_DISPATCH_MESSAGE;
  switch (msg->what)
  {
    case B_KEY_DOWN:
    {
      uint32 modifier (msg->FindInt32 ("modifiers"));
      const char *bytes (msg->FindString ("bytes"));
      
      if (!modifier)
      {
        if (bytes[0] != B_ENTER
        &&  bytes[0] != B_BACKSPACE
        &&  !isdigit (bytes[0]))
          result = B_SKIP_MESSAGE;
      }
      else if (modifier & B_SHIFT_KEY)
      {
        if (bytes[0] == B_LEFT_ARROW
        ||  bytes[0] == B_RIGHT_ARROW
        ||  bytes[0] == B_HOME
        ||  bytes[0] == B_END)
          break;
        else
          result = B_SKIP_MESSAGE;
      }
    }
    break;
  }
  return result;
}

class FontMenuItem : public BMenuItem
{
  public:
    FontMenuItem (const char *, const char *, BMessage *);
    FontMenuItem (const char *, const char *, BMenu *, BMessage* = NULL); 
    virtual ~FontMenuItem (void);
    virtual void DrawContent (void);
	
  private:
    const char *fontName;
    const char *fontStyle;
    BFont myFont;
};

FontMenuItem::FontMenuItem (const char *font, const char *style, BMessage *msg)
    : BMenuItem (style, msg),
       fontName (font),
       fontStyle (style)
{
  myFont.SetFamilyAndStyle (font, style);
}

FontMenuItem::FontMenuItem (const char *font, const char *style, BMenu *menu, BMessage *msg)
    : BMenuItem (menu, msg),
       fontName (font),
       fontStyle (style)
{
  myFont.SetFamilyAndStyle (font, style);
}

FontMenuItem::~FontMenuItem (void)
{
}

void
FontMenuItem::DrawContent (void)
{
  BMenu *menu (Menu());
  if (menu)
    menu->SetFont (&myFont, B_FONT_FAMILY_AND_STYLE);

  BMenuItem::DrawContent();
}

class FontMenu : public BMenu
{
  public:
    FontMenu (const char *);
    virtual ~FontMenu (void);
    virtual void AttachedToWindow (void);
};

FontMenu::FontMenu (const char *name)
    : BMenu (name)
{
  if (CountItems() == 0)
  {
    int32 i, family_count (count_font_families());

    FontStat *font_stat = new FontStat [family_count];

    for (i = 0; i < family_count; ++i)
    {
      uint32 flags;

      *font_stat[i].family       = '\0';
      font_stat[i].style_count   = 0;
      font_stat[i].styles        = 0;

      if (get_font_family (i, &font_stat[i].family, &flags) == B_OK
      && (font_stat[i].style_count = count_font_styles (font_stat[i].family)) > 0)
      {
        font_stat[i].styles = new font_style [font_stat[i].style_count];

        for (int32 j = 0; j < font_stat[i].style_count; ++j)
        {
          *font_stat[i].styles[j] = '\0';
          get_font_style (font_stat[i].family, j, font_stat[i].styles + j, &flags);
        }
      }
    }

    int32 j;

    for (j = 0; j < family_count; ++j)
      if (*font_stat[j].family && font_stat[j].style_count)
      {
        BMenu *menu (new BMenu (font_stat[j].family));
        FontMenuItem *fontMenu (new FontMenuItem (font_stat[j].family, "", menu));
        AddItem (fontMenu);

        for (int32 k = 0; k < font_stat[j].style_count; ++k)
        {
          BMessage *msg (new BMessage (M_FONT_CHANGE));
          FontMenuItem *item;

          msg->AddString ("family", font_stat[j].family);
          msg->AddString ("style", font_stat[j].styles[k]);
          msg->AddInt32 ("which", i);

          menu->AddItem (item = new FontMenuItem (font_stat[j].family, font_stat[j].styles[k], msg));
        }
      }

    for (i = 0; i < family_count; ++i)
      delete [] font_stat[i].styles;
    delete [] font_stat;
  }
}

FontMenu::~FontMenu()
{
}

void
FontMenu::AttachedToWindow (void)
{
  BMenu::AttachedToWindow();
}

static const char *FontControlLabels[] =
{
  "Text",
  "Server messages",
  "URLs",
  "Names list",
  "Input text",
  "Window List",
  "Channel List",
  "Timestamp",
	0
};


FontPrefsView::FontPrefsView (BRect frame)
  : BView (frame, "Command prefs", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS),
    fontMenuField (NULL),
    fontElementField (NULL),
    activeFont (0)
{
  SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  BMenu *elementMenu (new BMenu ("elements"));
  for (int32 i = 0; FontControlLabels[i]; i++)
  {
  	BMessage *msg (new BMessage (M_FONT_ELEMENT_CHANGE));
  	msg->AddInt32 ("index", i);
    elementMenu->AddItem (new BMenuItem (FontControlLabels[i], msg));
  }
  fontElementField = new BMenuField (BRect (10, 10, 200, 50), "elements", "Element: ",
    elementMenu);
  AddChild (fontElementField);
  FontMenu *menu (new FontMenu ("fonts"));
  fontMenuField = new BMenuField (BRect (10, 10, 200, 50), "fonts", "Font: ", menu);
  AddChild (fontMenuField);
  textControl = new BTextControl (BRect (60, 60, 200, 90), "", "Size: ", "",
  	new BMessage (M_FONT_SIZE_CHANGE));
  textControl->TextView()->AddFilter (new NumericFilter());
  AddChild (textControl);
}

FontPrefsView::~FontPrefsView (void)
{
}

void
FontPrefsView::AttachedToWindow (void)
{
  BView::AttachedToWindow ();
}

void
FontPrefsView::AllAttached (void)
{
  BView::AllAttached ();
  fontElementField->ResizeToPreferred();
  fontElementField->SetDivider (fontElementField->StringWidth(
    fontElementField->Label()) + 5);
  fontMenuField->SetDivider (fontMenuField->StringWidth (fontMenuField->Label()) + 5);
  textControl->SetDivider (textControl->StringWidth (textControl->Label()) + 5);
  BMenu *menu (fontElementField->Menu());
  textControl->SetTarget (this);
  menu->SetTargetForItems (this);
  menu->SetLabelFromMarked (true);

  BMenuItem *it (menu->ItemAt (0));

  if (it)
  	dynamic_cast<BInvoker *>(it)->Invoke();

  BRect frame (fontElementField->Frame());
  fontMenuField->MoveTo (frame.left + 20, frame.bottom + 20);
  menu = fontMenuField->Menu();
  menu->SetTargetForItems (this);
  menu->SetLabelFromMarked (true);
  float width;
  float height;
  fontMenuField->GetPreferredSize(&width, &height);
  textControl->ResizeToPreferred();
  textControl->MoveTo (fontMenuField->Frame().left + width + 5,
    fontMenuField->Frame().top);

  for (int32 i = 0; i < menu->CountItems(); i++)
  	menu->ItemAt(i)->Submenu()->SetTargetForItems (this);

  const BFont *current (vision_app->GetClientFont (0));
  font_family family;
  font_style style;
  current->GetFamilyAndStyle (&family, &style);
  it = menu->FindItem (family);

  if (it)
  	dynamic_cast<BInvoker *>(it)->Invoke();
}

void
FontPrefsView::FrameResized (float width, float height)
{
  BView::FrameResized (width, height);
}

static inline void
UnsetMarked (BMenuField *field)
{
  BMenuItem *item;
  for (int32 i = 0; i < field->Menu()->CountItems(); ++i)
  {
    BMenu *menu (field->Menu()->SubmenuAt (i));
     
    if ((item = menu->FindMarked()) != 0)
    {
      item->SetMarked (false);
      break;
    }
  }
}

void
FontPrefsView::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
  	case M_FONT_CHANGE:
      {
        BMenuItem *item (NULL);
        const char *family (NULL);
        const char *style (NULL);
        UnsetMarked (fontMenuField);

        msg->FindPointer ("source", reinterpret_cast<void **>(&item));
		item->SetMarked (true);
        msg->FindString ("family", &family);
        msg->FindString ("style", &style);
        vision_app->ClientFontFamilyAndStyle (activeFont, family, style);
        fontMenuField->MenuItem()->SetLabel (family);
      }
  	  break;
  	
  	case M_FONT_SIZE_CHANGE:
  	  {
  	    const char *text (textControl->TextView()->Text());
  	    int32 size (atoi (text));
  	    vision_app->ClientFontSize (activeFont, size);
  	  }
      break;
      
  	case M_FONT_ELEMENT_CHANGE:
  	{
  	   activeFont = msg->FindInt32 ("index");
       UnsetMarked (fontMenuField);
       const BFont *font (vision_app->GetClientFont (activeFont));
       font_family family;
       font_style style;
       font->GetFamilyAndStyle (&family, &style);
       char line[100];
       memset (line, 0, sizeof(line));
       sprintf(line, "%ld", (long)(font->Size()));
       textControl->TextView()->SetText (line);
       BMenuItem *it = fontMenuField->Menu()->FindItem (family);
       if (it)
         it->SetMarked (true);
  	}
  	break;
  	
    default:
      BView::MessageReceived (msg);
  }
}
