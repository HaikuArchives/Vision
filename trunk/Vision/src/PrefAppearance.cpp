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

// TODO: Color Schemes/Themes


#include "PrefAppearance.h"
#include "ColorLabel.h"
#include "Vision.h"
#include <Box.h>
#include <MenuField.h>
#include <StringView.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Point.h>

#include <stdio.h>

// data structures for font prefs

struct FontStat
{
	font_family				family;
	int32						style_count;
	font_style				*styles;
};

static const int32 FontSizes[] =
{
	6, 8, 9, 10, 11, 12, 14, 15, 16, 18, 24, 36, 48, 72, 0
};

static const char *ColorLabels[] =
{
	"Text",
	"Background",
	"URL",
	"Server text",
	"Notice",
	"Action",
	"Quit",
	"Error",
	"Nickname edges",
	"User nickname edges",
	"Nickname text",
	"Join",
	"Kick",
	"Whois",
	"Names (Normal)",
	"Names (Op)",
	"Names (Helper)",
	"Names (Voice)",
	"Names selection",
	"Names Background",
	"CTCP Request",
	"CTCP Reply",
	"Ignore",
	"Input text",
	"Input background",
	"Winlist normal status",
	"Winlist text status",
	"Winlist nick alert status",
	"Winlist selection status",
	"Winlist event status",
	"Winlist background",
	"Wallops",
};

static struct {
   int32 background;
   int32 foreground;
} colorPairs[MAX_COLORS];

static inline void
SetColorPair (int32 which, int32 background, int32 foreground)
{
  colorPairs[which].background = background;
  colorPairs[which].foreground = foreground;
}

AppearancePrefsView::AppearancePrefsView (BRect frame)
  : BView (frame, "Appearance Prefs", B_FOLLOW_NONE, B_WILL_DRAW)
{
  SetViewColor (ui_color(B_PANEL_BACKGROUND_COLOR));
  clientFont[0] = 0;
  lastwhich = 0;
  for (int32 i = 0 ; i < MAX_COLORS ; i++)
    colors[i] = vision_app->GetColor (i);
}

AppearancePrefsView::~AppearancePrefsView (void)
{
}

void
AppearancePrefsView::AttachedToWindow (void)
{
  BView::AttachedToWindow();
  if (clientFont[0] == 0)
  {
     int32 i (0);
     BRect frame(Bounds());
     BBox *fontBox (new BBox(frame.InsetByCopy(5,5), "Sample"));
     fontBox->ResizeBy (0, 5);
     sample = new BaseColorControl(BRect(0,0,0,0), "", "", colors[0], new BMessage (M_COLOR_CHANGED));
     sampleText = new BStringView (BRect (0,0,0,0), NULL,
       "MOVE ZIG!");
     sampleText->SetFont (vision_app->GetClientFont (F_TEXT));
     sampleText->ResizeToPreferred();
     fontBox->AddChild (sample);
     sample->ResizeTo(sampleText->Bounds().Width() + 3, sampleText->Bounds().Height() + 3);
     sample->MoveTo ((fontBox->Bounds().Width() / 2 - sample->Bounds().Width() / 2),
       (fontBox->Bounds().Height()) - sample->Bounds().Height() - 5);
     sample->AddChild (sampleText);
     sampleText->MoveTo (1,1);
     BMenu *elementMenu (new BMenu("Elements"));
     for (i = 0; i < MAX_COLORS; i++)
     {
        BMessage *msg (new BMessage (M_ELEMENT_SELECTION_CHANGED));
        msg->AddInt32 ("which", i);
        elementMenu->AddItem (new BMenuItem (ColorLabels[i], msg));
     }
     elementMenu->SetTargetForItems (this);
     fontMenu = new BMenuField (BRect (0, 0, 0, 0), "font list", "fonts", elementMenu);
     fontMenu->MenuItem()->SetLabel (ColorLabels[0]);
     fontBox->SetLabel (fontMenu);
     AddChild (fontBox);

     int32 family_count (count_font_families());
     float label_width (0.0);
     float name_width (0.0);
     for (i = 0; i < MAX_COLORS; ++i)
        if (be_plain_font->StringWidth (ColorLabels[i]) > label_width)
           label_width = be_plain_font->StringWidth (ColorLabels[i]);

	
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
			 if (be_plain_font->StringWidth(font_stat[i].family) > name_width)
			   name_width = be_plain_font->StringWidth (font_stat[i].family);
			   
             for (int32 j = 0; j < font_stat[i].style_count; ++j)
             {
               *font_stat[i].styles[j] = '\0';
               get_font_style (font_stat[i].family, j, font_stat[i].styles + j, &flags);
             }
           }
        }
        BMenu *parentMenu[2][7];
        font_height fh;
        float height;

        GetFontHeight (&fh);
        height = fh.ascent + fh.descent + fh.leading + 20;

        for (i = 0; i < 7; ++i)
        {
          font_family cur_family;
          font_style  cur_style;
          float cur_size;
          int32 j;
			
          be_plain_font->GetFamilyAndStyle (&cur_family, &cur_style);
          cur_size = be_plain_font->Size();

          vision_app->GetClientFont (i)->GetFamilyAndStyle (
             &cur_family,
             &cur_style);
          cur_size = vision_app->GetClientFont (i)->Size();

          parentMenu[0][i] = new BMenu ("Font");
          parentMenu[0][i]->SetTargetForItems (this);
          parentMenu[1][i] = new BMenu ("Size");
          parentMenu[1][i]->SetTargetForItems (this);

          clientFont[i] = new BMenuField (
             fontBox->Bounds(),
             "clientFont",
             "Font: ",
             parentMenu[0][i]);
          clientFont[i]->SetLabel ("Font:    ");
          clientFont[i]->ResizeToPreferred();
          clientFont[i]->MoveTo(be_plain_font->StringWidth("gP"), be_plain_font->Size() * 3);
          clientFont[i]->SetDivider (be_plain_font->StringWidth("Font:    "));
          fontBox->AddChild(clientFont[i]);

          fontSize[i] = new BMenuField (
             fontBox->Bounds(),
             "fontSize",
             "Size: ",
             parentMenu[1][i]);
          fontSize[i]->SetLabel ("Size:  ");
          fontBox->AddChild(fontSize[i]);
#if B_BEOS_VERSION_DANO
          fontSize[i]->MoveTo(be_plain_font->StringWidth(" gP Font:") + clientFont[i]->Bounds().Width(), be_plain_font->Size() * 3);
#else
          fontSize[i]->MoveTo(be_plain_font->StringWidth("gP") + name_width * 1.7, be_plain_font->Size() * 3);
#endif
          fontSize[i]->SetDivider (be_plain_font->StringWidth("Size:  "));

          for (j = 0; j < family_count; ++j)
             if (*font_stat[j].family && font_stat[j].style_count)
             {
                BMenu *menu (new BMenu (font_stat[j].family));
                parentMenu[0][i]->AddItem (menu);

                for (int32 k = 0; k < font_stat[j].style_count; ++k)
                {
                   BMessage *msg (new BMessage (M_FONT_CHANGE));
                   BMenuItem *item;

                   msg->AddString ("family", font_stat[j].family);
                   msg->AddString ("style", font_stat[j].styles[k]);
                   msg->AddInt32 ("which", i);

                   menu->AddItem (item = new BMenuItem (font_stat[j].styles[k], msg));
	
                   if (strcmp (font_stat[j].family,     cur_family) == 0
                      &&  strcmp (font_stat[j].styles[k],  cur_style)  == 0)
                   {
                      item->SetMarked (true);
                      clientFont[i]->MenuItem()->SetLabel (font_stat[j].family);
                   }
                }
             }

             for (j = 0; FontSizes[j]; ++j)
             {
                BMessage *msg (new BMessage (M_FONT_SIZE_CHANGE));
                BMenuItem *item;
                char buffer[32];
	
                sprintf (buffer, "%ld", FontSizes[j]);
                msg->AddInt32 ("size", FontSizes[j]);
                msg->AddInt32 ("which", i);

                parentMenu[1][i]->AddItem (item = new BMenuItem (buffer, msg));
	
                if (FontSizes[j] == cur_size)
                   item->SetMarked (true);
             }
	
          }

      for (i = 0; i < family_count; ++i)
         delete [] font_stat[i].styles;
      delete [] font_stat;

      for (i = 1; i < 7; i++)
      {
         clientFont[i]->Hide();
         fontSize[i]->Hide();
      }

      for (i = 0; i < 7; ++i)
      {
         fontSize[i]->Menu()->SetLabelFromMarked (true);
		 fontSize[i]->MenuItem()->SetLabel (fontSize[i]->Menu()->FindMarked()->Label());

         fontSize[i]->Menu()->SetTargetForItems (this);
         for (int32 j = 0; j < clientFont[i]->Menu()->CountItems(); ++j)
		 {
            BMenuItem *item (clientFont[i]->Menu()->ItemAt (j));

            item->Submenu()->SetTargetForItems (this);
         }
      }
      label = new ColorLabel (BRect (0, 0, 0, 0), "colorlabel", "Color: ", colors[0], new BMessage(M_COLOR_INVOKED));
      label->ResizeToPreferred();
      label->MoveTo(be_plain_font->StringWidth("gP "), be_plain_font->Size() * 6);	  
      fontBox->AddChild (label);
      label->SetTarget (this);
      
      if ((clientFont[0]->Frame().right - fontBox->Frame().right) <= be_plain_font->StringWidth("Size: "))
      {
         for (i = 0; i < 7; i++)
           fontSize[i]->MoveTo (1.5 * (label->Frame().right + be_plain_font->StringWidth (" gP ")), be_plain_font->Size() * 5.5);
      }      


      // init sample color data

      SetColorPair (C_TEXT, C_BACKGROUND, C_TEXT);
      SetColorPair (C_BACKGROUND, C_BACKGROUND, C_TEXT);
      SetColorPair (C_NAMES, C_NAMES_BACKGROUND, C_NAMES);
      SetColorPair (C_NAMES_BACKGROUND, C_NAMES_BACKGROUND, C_NAMES);
      SetColorPair (C_URL, C_BACKGROUND, C_URL);
      SetColorPair (C_SERVER, C_BACKGROUND, C_SERVER);
      SetColorPair (C_NOTICE, C_BACKGROUND, C_NOTICE);
      SetColorPair (C_ACTION, C_BACKGROUND, C_ACTION);
      SetColorPair (C_QUIT, C_BACKGROUND, C_QUIT);
      SetColorPair (C_ERROR, C_BACKGROUND, C_ERROR);
      SetColorPair (C_NICK, C_BACKGROUND, C_NICK);
      SetColorPair (C_MYNICK, C_BACKGROUND, C_MYNICK);
      SetColorPair (C_JOIN, C_BACKGROUND, C_JOIN);
      SetColorPair (C_KICK, C_BACKGROUND, C_KICK);
      SetColorPair (C_WHOIS, C_BACKGROUND, C_WHOIS);
      SetColorPair (C_OP, C_NAMES_BACKGROUND, C_OP);
      SetColorPair (C_HELPER, C_NAMES_BACKGROUND, C_HELPER);
      SetColorPair (C_VOICE, C_NAMES_BACKGROUND, C_VOICE);
      SetColorPair (C_CTCP_REQ, C_BACKGROUND, C_CTCP_REQ);
      SetColorPair (C_CTCP_RPY, C_BACKGROUND, C_CTCP_RPY);
      SetColorPair (C_IGNORE, C_NAMES_BACKGROUND, C_IGNORE);
      SetColorPair (C_INPUT_BACKGROUND, C_INPUT_BACKGROUND, C_INPUT);
      SetColorPair (C_INPUT, C_INPUT_BACKGROUND, C_INPUT);
      SetColorPair (C_WINLIST_BACKGROUND, C_WINLIST_BACKGROUND, C_WINLIST_NORMAL);
      SetColorPair (C_WINLIST_NORMAL, C_WINLIST_BACKGROUND, C_WINLIST_NORMAL);
      SetColorPair (C_WINLIST_NEWS, C_WINLIST_BACKGROUND, C_WINLIST_NEWS);
      SetColorPair (C_WINLIST_NICK, C_WINLIST_BACKGROUND, C_WINLIST_NICK);
      SetColorPair (C_WINLIST_PAGESIX, C_WINLIST_BACKGROUND, C_WINLIST_PAGESIX);
      SetColorPair (C_WINLIST_SELECTION, C_WINLIST_SELECTION, C_WINLIST_NORMAL);
      SetColorPair (C_WALLOPS, C_BACKGROUND, C_WALLOPS);
      SetColorPair (C_NICKDISPLAY, C_BACKGROUND, C_NICKDISPLAY);
   }
}

void
AppearancePrefsView::AllAttached (void)
{
  BView::AllAttached();
  SetSampleColors (0);
}

void
AppearancePrefsView::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
      case M_COLOR_INVOKED:
		if (!picker.IsValid())
		{
			ColorPicker *win;

			win = new ColorPicker (
			BPoint(Window()->Frame().Width() / 2 + Window()->Frame().left, Window()->Frame().Height() / 2 + Window()->Frame().top),
				label->ValueAsColor(),
				BMessenger (this),
				new BMessage (M_COLOR_CHANGED));
			win->AddToSubset(Window());
			win->Show();
			picker = BMessenger (win);
		}
	    else
	    {
	      ColorPicker *win (NULL);
	      picker.Target (reinterpret_cast<BLooper **>(&win));
	      if (win)
	        win->Activate(true);
	    }
        break;
        
      case M_COLOR_CHANGED:
        {
          const rgb_color *color;
          ssize_t size;
          msg->FindData (
            "color",
            B_RGB_COLOR_TYPE,
            reinterpret_cast<const void **>(&color),
            &size);

          label->SetColor (*color);
          colors[lastwhich] = *color;
          SetSampleColors (lastwhich);
          vision_app->SetColor (lastwhich, *color);
		}
		break;
		        
     case M_FONT_CHANGE:
     {
        const char *family, *style;
        BMenuItem *item;
        int32 which;

        msg->FindInt32 ("which", &which);

        // Unmark
        for (int32 i = 0; i < clientFont[which]->Menu()->CountItems(); ++i)
        {
           BMenu *menu (clientFont[which]->Menu()->SubmenuAt (i));
				
           if ((item = menu->FindMarked()) != 0)
           {
              item->SetMarked (false);
              break;
           }
        }

        msg->FindPointer ("source", reinterpret_cast<void **>(&item));
        item->SetMarked (true);

        msg->FindString ("family", &family);
        msg->FindString ("style", &style);

        clientFont[which]->MenuItem()->SetLabel (family);
        vision_app->ClientFontFamilyAndStyle (which, family, style);
        UpdateSampleFont (which);
        break;
     }

     case M_FONT_SIZE_CHANGE:
     {
        int32 which, size;

        msg->FindInt32 ("which", &which);
        msg->FindInt32 ("size", &size);
        vision_app->ClientFontSize (which, size);
        UpdateSampleFont(which);
        break;
     }

     case M_ELEMENT_SELECTION_CHANGED:
     {
        int32 which;

        msg->FindInt32 ("which", &which);

        fontMenu->MenuItem()->SetLabel (ColorLabels[which]);
        label->SetColor(colors[which]);
        if (picker.IsValid())
        {
          BMessage msg (M_COLOR_SELECT);
          msg.AddData ("color", B_RGB_COLOR_TYPE, &colors[which], sizeof(colors[which]));
          picker.SendMessage (&msg);
        }
        
        SetSampleColors (which);
        lastwhich = which;
        
        switch (which)
        {
           case C_TEXT:
             SetFontControlState (0);
             break;
           
           case C_SERVER:
             SetFontControlState (1);
             break;
           
           case C_URL:
             SetFontControlState (2);
             break;
               
           case C_NAMES:
             SetFontControlState (3);
             break;
           
           case C_INPUT:
             SetFontControlState (4);
             break;
           
           case C_WINLIST_NORMAL:
             SetFontControlState (5);
             break;
           
           default:
             SetFontControlState (-1);
             break;
        }
        break;
     }


     default:
        BView::MessageReceived(msg);
        break;
  }
}

void
AppearancePrefsView::SetFontControlState(int32 which)
{
  for (int32 i = 0; i < 7; i++)
  {
      if (!clientFont[i]->IsHidden())
      {
        if (which == -1)
        {
          clientFont[i]->MenuItem()->SetEnabled (false);
          fontSize[i]->MenuItem()->SetEnabled (false);
          break;
        }
        else
        {
          clientFont[i]->MenuItem()->SetEnabled (true);
          fontSize[i]->MenuItem()->SetEnabled (true);
        }
        clientFont[i]->Hide();
        fontSize[i]->Hide();
        clientFont[which]->Show();
        fontSize[which]->Show();
        UpdateSampleFont (which);
        break;
     }
  }    
}

void
AppearancePrefsView::UpdateSampleFont (int32 which)
{
    BBox *fontBox ((BBox *)sample->Parent());

//  this should not happen but just in case. 
  if (!fontBox)
    return;
  sampleText->SetFont (vision_app->GetClientFont (which));
  sampleText->ResizeToPreferred();
  sample->ResizeTo(sampleText->Bounds().Width() + 3, sampleText->Bounds().Height() + 3);
  sample->MoveTo ((fontBox->Bounds().Width() / 2 - sample->Bounds().Width() / 2),
   (fontBox->Bounds().Height()) - sample->Bounds().Height() - 5);  sampleText->Invalidate();
}

void
AppearancePrefsView::SetSampleColors (int32 which)
{
  sampleText->SetHighColor (colors[colorPairs[which].foreground]);
  sampleText->SetLowColor (colors[colorPairs[which].background]);
  sampleText->SetViewColor (colors[colorPairs[which].background]);
  sample->SetColor (colors[colorPairs[which].background]);
  sample->Invalidate();
  sampleText->Invalidate();
}
