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
 * Contributor(s): Wade Majors <guru@startrek.com>
 *                 Rene Gollent
 *                 Todd Lair
 *                 Andrew Bazan
 *                 Jamie Wilkinson
 */

#define MAX_BYTES           196000
#define MELTDOWN_BYTES      500000
#define REMOVE_BYTES        1024

#ifdef GNOME_BUILD
#  include "gnome/PopUpMenu.h"
#  include "gnome/MenuItem.h"
#elif BEOS_BUILD
#  include <PopUpMenu.h>
#  include <MenuItem.h>
#endif

#include <Window.h>

#include <list.h>
#include <ctype.h>
#include <stdio.h>

#include "VTextControl.h"
#include "Vision.h"
#include "ChannelAgent.h"
#include "ClientAgent.h"
#include "IRCView.h"

struct URL 
{ 
  int32 offset; 
  BString display, 
          url; 
  
  URL (int32 os, const char *buffer) 
    : offset (os), 
      display (buffer), 
      url (buffer) 
      { 
        if (display.ICompare ("https://", 8) == 0) 
        { 
          url  = "http://"; 
          url += display.String() + 8; 
        } 

        if (display.ICompare ("www.", 4) == 0) 
        { 
          url  = "http://"; 
          url += display; 
        } 

        if (display.ICompare ("ftp.", 4) == 0) 
        { 
          url  = "ftp://"; 
          url += display; 
        } 
      } 

  URL (void) 
    : offset (0), 
      display (""), 
      url ("") 
      { 
      } 
}; 


/* We put settings here mostly to eliminate 
 * the template crap through out all 
 * the files that use a IRCView
 */

struct IRCViewSettings 
{ 
  VTextControl  *parentInput;
  ClientAgent *parentAgent;

  BFont  urlFont;
  rgb_color  urlColor; 
  list<URL>  urls;
  
  rgb_color  myNick_color; 
};


IRCView::IRCView ( 
  BRect textframe, 
  BRect textrect, 
  VTextControl *inputControl,
  ClientAgent *fatherAgent) 

  : BTextView ( 
    textframe, 
    "IRCView",
    textrect, 
    B_FOLLOW_ALL_SIDES, 
    B_WILL_DRAW | B_FRAME_EVENTS) 
{ 
  settings = new IRCViewSettings;
  
  settings->parentAgent = fatherAgent;
  settings->parentInput = inputControl; 
  settings->urlFont     = *(vision_app->GetClientFont (F_URL)); 
  settings->urlColor    = vision_app->GetColor (C_URL);
  settings->myNick_color    = vision_app->GetColor (C_MYNICK);

  tracking = false;
  MakeEditable (false); 
  MakeSelectable (true); 
  SetStylable (true); 
} 


IRCView::~IRCView (void) 
{ 
  delete settings; 
}

void
IRCView::BuildPopUp (void)
{
  // This function checks certain criteria (text is selected,
  // TextView is editable, etc) to determine which MenuItems
  // to enable and disable
  
  bool enablecopy (true),
       enableselectall (true),
       enablelookup (false);
  BString querystring ("");
       
  int32 selstart, selfinish;
  GetSelection (&selstart, &selfinish);
  if (selstart == selfinish)
    enablecopy = false; // no selection
    
  if (!IsSelectable() || (TextLength() == 0))
    enableselectall = false; // not selectable
  
  if ((enablecopy) && ((selfinish - selstart) <= 384))
  {
    enablelookup = true; // has a selection less than 32 chars long
    char *stringchar (querystring.LockBuffer(384));
    GetText (selstart, (selfinish - selstart), stringchar);
    querystring.UnlockBuffer(-1);
  }
  
  myPopUp = new BPopUpMenu ("IRCView Context Menu", false, false); 

  BMenuItem *item;

  // get dynamic entries from ClientAgent
  settings->parentAgent->AddMenuItems (myPopUp);
  
  BMessage *lookup;  
  lookup = new BMessage (M_LOOKUP_WEBSTER);
  lookup->AddString ("string", querystring);
  item = new BMenuItem("Lookup (Dictionary)", lookup);
  item->SetEnabled (enablelookup);
  item->SetTarget (settings->parentAgent);
  myPopUp->AddItem (item);

  lookup = new BMessage (M_LOOKUP_GOOGLE);
  lookup->AddString ("string", querystring);
  item = new BMenuItem("Lookup (Google)", lookup);
  item->SetEnabled (enablelookup);
  item->SetTarget (settings->parentAgent);
  myPopUp->AddItem (item);
  
  myPopUp->AddSeparatorItem(); 

  item = new BMenuItem("Copy", new BMessage (B_COPY), 'C');
  item->SetEnabled (enablecopy);
  item->SetTarget (this);
  myPopUp->AddItem (item);
  
  item = new BMenuItem("Select All", new BMessage (B_SELECT_ALL), 'A');
  item->SetEnabled (enableselectall);
  item->SetTarget (this);
  myPopUp->AddItem (item);
  
  myPopUp->SetFont (be_plain_font);
}

void 
IRCView::MouseDown (BPoint myPoint) 
{ 
  int32 selstart,
        selfinish;

  GetSelection (&selstart, &selfinish);
  BMessage *inputMsg (Window()->CurrentMessage());
  int32 mousebuttons (0),
        keymodifiers (0);

  inputMsg->FindInt32 ("buttons", &mousebuttons);
  inputMsg->FindInt32 ("modifiers", &keymodifiers); 

  if (mousebuttons == B_SECONDARY_MOUSE_BUTTON
  && (keymodifiers & B_SHIFT_KEY)   == 0
  && (keymodifiers & B_OPTION_KEY)  == 0
  && (keymodifiers & B_COMMAND_KEY) == 0
  && (keymodifiers & B_CONTROL_KEY) == 0)
  {
    MakeFocus (true);
    BuildPopUp();
    myPopUp->Go (
      ConvertToScreen (myPoint),
      true,
      false);
  }
  else
  {
    BTextView::MouseDown (myPoint);   
    if (selstart == selfinish) 
    { 
      list<URL> &urls (settings->urls); 
      list<URL>::const_iterator it; 
      int32 offset (OffsetAt (myPoint));
  
      for (it = urls.begin(); it != urls.end(); ++it) 
 
      if (offset >= it->offset 
      &&  offset <  it->offset + it->display.Length()) 
      {
        vision_app->LoadURL (it->url.String()); 
        settings->parentInput->MakeFocus (true); 
      }
    }
  }
} 


void 
IRCView::KeyDown (const char * bytes, int32 numBytes) 
{
  BMessage inputMsg (M_INPUT_FOCUS); 
  BString buffer; 

  buffer.Append (bytes, numBytes); 
  inputMsg.AddString ("text", buffer.String()); 

  if (settings->parentAgent && settings->parentAgent->msgr.IsValid())
    settings->parentAgent->msgr.SendMessage (&inputMsg);

}


int32 
IRCView::DisplayChunk ( 
  const char *cData, 
  const rgb_color *color, 
  const BFont *font) 
{
  int32 urlMarker; 
  BString url, 
          data (cData);
  bool scrolling;
          
  float scrollMin, scrollMax;  
  settings->parentAgent->ScrollRange (&scrollMin, &scrollMax);
  float scrollVal (settings->parentAgent->ScrollPos());
  
  if (scrollVal == scrollMax)
  {
    // we're at the bottom... let's autoscroll
    scrolling = true;
  }
  else if (TextLength() > MELTDOWN_BYTES)
  {
    // we're getting too big, we *have* to scroll (and clip excess bytes)
    scrolling = true;
  }
  else
  {
    // we aren't at the bottom, don't scroll (or clip)
    scrolling = false;
  }
          
   

  /* Previously calling SetFontAndColor is really the wrong approach, 
   * since it does not add to the run array.  You have to pass a run array 
   * for this to work.  Otherwise, things like selecting the text 
   * will force the text to redraw in the single run array (you'll lose 
   * all your coloring)
   */

   while ((urlMarker = FirstURLMarker (data.String())) >= 0) 
   { 
     int32 theLength (URLLength (data.String() + urlMarker)); 
     BString buffer; 

     if (urlMarker) 
     { 
       data.MoveInto (buffer, 0, urlMarker); 

       text_run_array run; 
       run.count          = 1; 
       run.runs[0].offset = 0; 
       run.runs[0].font   = *font; 
       run.runs[0].color  = *color; 

       Insert (TextLength(), buffer.String(), buffer.Length(), &run); 
     } 

     data.MoveInto (buffer, 0, theLength); 

     settings->urls.push_back (URL (TextLength(), buffer.String())); 

     text_run_array run; 

     run.count          = 1; 
     run.runs[0].offset = 0; 
     run.runs[0].font   = settings->urlFont; 
     run.runs[0].color  = settings->urlColor; 

     Insert (TextLength(), buffer.String(), buffer.Length(), &run); 
   } 

   if (data.Length()) 
   { 
     text_run_array run; 
     run.count          = 1; 
     run.runs[0].offset = 0; 
     run.runs[0].font   = *font; 
     run.runs[0].color  = *color; 

     Insert (TextLength(), data.String(), data.Length(), &run); 
   } 

   if ((TextLength() > MAX_BYTES) && (scrolling)) 
   {
     // clip off excess buffer
     list<URL> &urls (settings->urls); 
     int32 bytes ((TextLength() - MAX_BYTES) + REMOVE_BYTES);
      
     const char *text (Text()); 

     while (*(text + bytes) && *(text + bytes) != '\n') 
       ++bytes; 

     while (*(text + bytes) 
     &&    (*(text + bytes) == '\n' 
     ||     *(text + bytes) == '\r')) 
       ++bytes; 

     /* woulda, coulda used a deque.. 
      * too many warnings on signed/ 
      * unsigned comparisons -- list 
      * works fine though
      */ 
     while (!urls.empty()) 
       if (urls.front().offset < bytes) 
         urls.erase (urls.begin()); 
       else 
         break; 

     list<URL>::iterator it; 
     for (it = urls.begin(); it != urls.end(); ++it) 
       it->offset -= bytes; 
               
   } 

   if (scrolling && !tracking)
     if (TextLength() > 0) 
       ScrollToOffset (TextLength()); 

   return TextLength();
} 

int32
IRCView::URLLength (const char *outTemp) 
{
  BString data (outTemp);
  int32 urlEnd = data.FindFirst(" ");

  if (urlEnd > 0)
    data.Truncate (urlEnd);

  while(data.Length() > 1)
  {
    char last = data.ByteAt (data.Length()-1);
    if (isdigit (last)
    ||  isalpha (last)
    ||  last == '/')
      break;
    else
      data.Truncate (data.Length()-1);
  }

  return data.Length();
} 


int32
IRCView::FirstURLMarker (const char *cData) 
{ 
  BString data (cData); 
  int32 urlMarkers[6], 
        marker (data.Length()), 
        pos (0); 

  const char *tags[6] = 
  { 
    "http://", 
    "https://", 
    "www.", 
    "ftp://", 
    "ftp.", 
    "file://" 
  }; 

  do 
  { 
    urlMarkers[0] = data.IFindFirst (tags[0], pos); 
    urlMarkers[1] = data.IFindFirst (tags[1], pos); 
    urlMarkers[2] = data.IFindFirst (tags[2], pos); 
    urlMarkers[3] = data.IFindFirst (tags[3], pos); 
    urlMarkers[4] = data.IFindFirst (tags[4], pos); 
    urlMarkers[5] = data.IFindFirst (tags[5], pos); 

    for (int32 i = 0; i < 6; ++i) 
      if (urlMarkers[i] != B_ERROR 
      &&  urlMarkers[i] <  marker) 
      { 
        if (URLLength (cData + urlMarkers[i]) > (int32)strlen (tags[i])) 
        { 
          marker = urlMarkers[i]; 
          pos = data.Length(); 
        } 
        else // just enough 
          pos = urlMarkers[i] + 1; 
      } 
  }
    
  while ((urlMarkers[0] != B_ERROR 
  ||      urlMarkers[1] != B_ERROR 
  ||      urlMarkers[2] != B_ERROR 
  ||      urlMarkers[3] != B_ERROR 
  ||      urlMarkers[4] != B_ERROR 
  ||      urlMarkers[5] != B_ERROR) 
  &&      pos < data.Length()); 

  return marker < data.Length() ? marker : -1; 
} 


void
IRCView::ClearView (bool all) 
{
  if (all || TextLength() < 96)
  {
    // clear all data
    list<URL> &urls (settings->urls); 
    int32 bytes (TextLength());

    while (!urls.empty()) 
      if (urls.front().offset < bytes) 
        urls.erase (urls.begin()); 
      else 
        break; 

    list<URL>::iterator it; 
    for (it = urls.begin(); it != urls.end(); ++it) 
      it->offset -= bytes;

    float scrollMin, scrollMax; 
    settings->parentAgent->ScrollRange (&scrollMin, &scrollMax); 

    Delete (0, bytes);
    ScrollToOffset (0);
  }
  else
  {
    // clear all but last couple of lines
    list<URL> &urls (settings->urls); 
    int32 bytes (TextLength() - 96);
    const char *text (Text()); 

    while (*(text + bytes) && *(text + bytes) != '\n') 
      ++bytes;

    while (*(text + bytes) 
    &&    (*(text + bytes) == '\n' 
    ||     *(text + bytes) == '\r')) 
      ++bytes; 

    while (!urls.empty()) 
      if (urls.front().offset < bytes) 
        urls.erase (urls.begin()); 
      else 
        break;

    list<URL>::iterator it; 
    for (it = urls.begin(); it != urls.end(); ++it) 
      it->offset -= bytes; 

    float scrollMin, scrollMax; 
    settings->parentAgent->ScrollRange (&scrollMin, &scrollMax); 

    Delete (0,bytes);
    ScrollToOffset (TextLength());
  }
} 


void 
IRCView::FrameResized (float width, float height) 
{ 
  BRect textrect = TextRect(); 

  textrect.right  = textrect.left + width - 7; 
  textrect.bottom = textrect.top + height - 1; 
  SetTextRect (textrect); 
  ScrollToOffset (TextLength()); 
}


void 
IRCView::SetColor (int32 which, rgb_color color) 
{ 
  if (which == C_URL) 
    settings->urlColor = color; 
}


void 
IRCView::SetFont (int32 which, const BFont *font) 
{ 
  if (which == F_URL) 
    settings->urlFont = *font; 
}
