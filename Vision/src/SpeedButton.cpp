//-------------------------------------------------------------------------------
//
// SpeedButton
//
// Deco SpeedButton class, a nifty X-style button with a graphic and a label.
// It tries to be as lightweight as possible (but is failing miserably ;).
//
// Author: Alan Ellis
// Copyright 1999 - 2001, the Deco team. CGSoftware International
//
// Used with permission for the Vision project. (http://vision.sourceforge.net/)
//-------------------------------------------------------------------------------

#include <be/interface/Window.h>
#include <be/interface/Bitmap.h>
#include <be/interface/Rect.h>
#include <be/app/Looper.h>

#include "SpeedButton.h"

//-------------------------------------------------------------------------------
TSpeedButton::TSpeedButton(BRect Frame, const char *Name, const char* Label,
                           BMessage *Message, BBitmap *EnabledBitmap,
                           TSBStyle Style,
                           BBitmap *DisabledBitmap,
                           uint32 ResizingMask,
                           uint32 Flags)
        :BControl(Frame, Name, Label, Message, ResizingMask, Flags),
        FDisabledBitmap(NULL),
        FEnabledBitmap(NULL),
        FBitmapDestinationRect(0, 0, 0, 0),
        FBorder(Bounds()),
        FStyle(Style),
        FGroupIndex(-1),
        FHighlightColor(keyboard_navigation_color()),
        FLabelPos(0, 0),
        FActive(false),
        FMouseDown(false),
        FOutside(true),
        FAttachedToWindow(false),
        FSelected(false),
        FLatching(false),
        FHighlighted(false)
{
    // make a copy of the disabled bitmap if needed.
    if (DisabledBitmap != NULL)
    {
        FDisabledBitmap = new BBitmap(DisabledBitmap);
    }

    if (EnabledBitmap != NULL)
    {
        FEnabledBitmap = new BBitmap(EnabledBitmap);
        SetupBitmaps(FEnabledBitmap);
    }
    
    if( NULL != BControl::Label() )
    {
    	SetupLabel();
    }
}
//-------------------------------------------------------------------------------
TSpeedButton::TSpeedButton(BMessage *Archive)
        :BControl(Archive)
{
    Archive->FindBool("Deco::TSpeedButton:FSelected", &FSelected);
    Archive->FindInt32("Deco::TSpeedButton:FGroupIndex", &FGroupIndex);

    FBorder = Bounds();

    BMessage *BitmapArchiveD = NULL;
    BMessage *BitmapArchiveE = NULL;

    Archive->FindMessage("DCL::TSpeedButton:FDisabledBitmap", BitmapArchiveD);
    Archive->FindMessage("DCL::TSpeedButton:FEnabledBitmap", BitmapArchiveE);

    FDisabledBitmap = dynamic_cast<BBitmap *>(BBitmap::Instantiate(BitmapArchiveD));
    FEnabledBitmap = dynamic_cast<BBitmap *>(BBitmap::Instantiate(BitmapArchiveE));

    if (NULL != FEnabledBitmap)
        SetupBitmaps(FEnabledBitmap);

    FActive = false;
    FAttachedToWindow = false;
}
//-------------------------------------------------------------------------------
TSpeedButton *TSpeedButton::Instantiate(BMessage *Archive)
{
    if ( validate_instantiation(Archive, "TSpeedButton"))
        return new TSpeedButton(Archive);

    return NULL;
}
//-------------------------------------------------------------------------------
status_t TSpeedButton::Archive(BMessage *Archive, bool Deep) const
{
    BView::Archive(Archive, Deep);
    Archive->AddString("class", "TSpeedButton");

    BMessage BitmapArchiveD;
    BMessage BitmapArchiveE;

    if (FDisabledBitmap != NULL)
    {
        FDisabledBitmap->Archive(&BitmapArchiveD);
        Archive->AddMessage("DCL::TSpeedButton:FDisabledBitmap", &BitmapArchiveD);
    }

    if (FEnabledBitmap != NULL)
    {
        FEnabledBitmap->Archive(&BitmapArchiveE);
        Archive->AddMessage("DCL::TSpeedButton:FDisabledBitmap", &BitmapArchiveE);
    }

    Archive->AddBool("DCL::TSpeedButton:FSelected", FSelected);
    Archive->AddInt32("DCL::TSpeedButton:FGroupIndex", FGroupIndex);

    // todo: Forward error checking on?
    return B_OK;
}
//-------------------------------------------------------------------------------
void TSpeedButton::SetupBitmaps(BBitmap* SourceBitmap)
{
    // We need EDGE_SPACE pixels clear on every side for the various button functions
    FBitmapDestinationRect = Bounds().InsetByCopy(EDGE_SPACE, EDGE_SPACE);

    // setup the Source and Destination bitmap rects, clipping when necessary.
    BRect workRect(SourceBitmap->Bounds());

    // Determine if any work needs to be done, the offset is for easy comparison
    if (workRect.OffsetByCopy(EDGE_SPACE, EDGE_SPACE) != FBitmapDestinationRect)
    {
        // The source may be taller
        if (workRect.IntegerHeight() > FBitmapDestinationRect.IntegerHeight())
        {
            // Move the source down by half the difference and clip.
            workRect.OffsetBy(0, (workRect.IntegerHeight() - FBitmapDestinationRect.IntegerHeight()) / 2);
            workRect.bottom = workRect.top + FBitmapDestinationRect.IntegerHeight();
        }

        // The source may be wider
        if (workRect.IntegerWidth() > FBitmapDestinationRect.IntegerWidth())
        {
            // Move the source over by half the difference and clip.
            workRect.OffsetBy((workRect.IntegerWidth() - FBitmapDestinationRect.IntegerWidth()) / 2, 0);
            workRect.right = workRect.left + FBitmapDestinationRect.IntegerWidth();
        }

        // The source may be shorter
        if (workRect.IntegerHeight() < FBitmapDestinationRect.IntegerHeight())
        {
            // Move the destination right by half the difference and clip
            FBitmapDestinationRect.OffsetBy(0, (FBitmapDestinationRect.IntegerHeight() - workRect.IntegerHeight()) / 2);
            FBitmapDestinationRect.bottom = FBitmapDestinationRect.top + workRect.IntegerHeight();
        }

        // The source may be narrower
        if (workRect.IntegerWidth() < FBitmapDestinationRect.IntegerWidth())
        {
            // Move the destination right by half the difference and clip
            FBitmapDestinationRect.OffsetBy((FBitmapDestinationRect.IntegerWidth() - workRect.IntegerWidth()) / 2, 0);
            FBitmapDestinationRect.right = FBitmapDestinationRect.left + workRect.IntegerWidth();
        }
    }

    FBitmapSourceRect = workRect;
}
//-------------------------------------------------------------------------------
void TSpeedButton::SetupLabel()
{
	// This will look real funny if the button is not made large enough for the label.

	// Figure out where the label lives.
	FLabelPos.y = Bounds().bottom - EDGE_SPACE;
	
	FLabelPos.x = (Bounds().Width() / 2.0) - (StringWidth(Label()) / 2.0 );
	
	// Move the destination accordingly.
	FBitmapDestinationRect = FBitmapSourceRect;
	
	font_height fheight;
	GetFontHeight(&fheight);
	
	float availHeight = Bounds().Height() - (fheight.ascent +
	                    fheight.descent + fheight.leading +
	                    (2 * EDGE_SPACE));
	
	float top = (availHeight / 2.0) - (FBitmapSourceRect.Height() / 2.0) + EDGE_SPACE;
	float left = ( Bounds().Width() / 2.0 ) - (FBitmapSourceRect.Width() / 2.0);
	
	FBitmapDestinationRect.OffsetTo(left, top);	
}
//-------------------------------------------------------------------------------
void TSpeedButton::AttachedToWindow()
{
    SetViewColor(Parent()->ViewColor());

    FEnabledViewColor = Parent()->ViewColor();
    SetBlendingMode( B_PIXEL_ALPHA, B_ALPHA_OVERLAY );
    
    if( (NULL == FDisabledBitmap) && (NULL != FEnabledBitmap))
    {
		if ( FEnabledBitmap && (B_RGB32 != FEnabledBitmap->ColorSpace() ))
		{
			// translate to 32 bit image ...
			BBitmap* temp = new BBitmap(FEnabledBitmap->Bounds(), B_RGB32, true);
			BView aView(FEnabledBitmap->Bounds(), "", 0, B_WILL_DRAW);
			temp->AddChild(&aView);
			aView.Window()->Lock();
			aView.DrawBitmap(FEnabledBitmap);
			aView.Window()->Unlock();
			temp->RemoveChild(&aView);
			FDisabledBitmap = temp;
		}
		else
		{
			if (FEnabledBitmap)
				FDisabledBitmap = new BBitmap(FEnabledBitmap);
		}
		
		uint32 bitend = FDisabledBitmap->BitsLength() / 4;
		
		// shade the image
		for( uint32 i = 0; i < bitend; i++ )
		{
			rgb_color* color = (rgb_color*)(&((uint32*)(FDisabledBitmap->Bits()))[i]);

			// todo: Should we use right shift by one to speed up this loop?
			color->red += ((FEnabledViewColor.red - color->red) / 2); 
			color->blue += ((FEnabledViewColor.blue - color->blue) / 2);
			color->green += ((FEnabledViewColor.green - color->green) / 2);
		}    
    }

    FAttachedToWindow = true;
    
    BControl::AttachedToWindow();
}
//-------------------------------------------------------------------------------
void TSpeedButton::DetachedFromWindow()
{
    BControl::DetachedFromWindow();
}
//-------------------------------------------------------------------------------
TSpeedButton::~TSpeedButton()
{
	// It's safe to delete NULL pointers
	delete FDisabledBitmap;
	delete FEnabledBitmap;
}
//-------------------------------------------------------------------------------
void TSpeedButton::Draw(BRect UpdateRect)
{
    BControl::Draw(UpdateRect);
    FBorder = Bounds();

    rgb_color viewcolour = Parent()->ViewColor();
    rgb_color highlight  = tint_color(viewcolour, B_LIGHTEN_MAX_TINT);
    rgb_color shadow     = tint_color(viewcolour, B_DARKEN_2_TINT);


    // In order to draw with alpha, we have to do it ourselves rather than
    //  just set the background bitmap.
    drawing_mode mode = DrawingMode();
    SetDrawingMode(B_OP_ALPHA);

    if (FEnabledBitmap != NULL)
    {
        if (true == BControl::IsEnabled())
        {
            DrawBitmapAsync(FEnabledBitmap, FBitmapSourceRect, FBitmapDestinationRect);
        }
        else
        {
            if (FDisabledBitmap != NULL)
            {
                DrawBitmapAsync(FDisabledBitmap, FBitmapSourceRect, FBitmapDestinationRect);
            }
        }
    }

    SetDrawingMode(mode);

	if( NULL != Label() )
	{
		if( true == IsEnabled() )
		{
			SetHighColor(0, 0, 0);
		}
		else
		{
			SetHighColor(132, 130, 132);	// Damn, no ui_color() for this ...
		}
		
		DrawString(Label(), FLabelPos);
	}

    TSBDrawState state = sbSFlat;

    if (true == BControl::IsEnabled())
        if (!FOutside)
            if (FMouseDown)
                state = sbSDown;

    if (FStyle == sbUp)
        if (!FMouseDown || FOutside)
            state = sbSUp;

    if (FActive)
        if (!FMouseDown && (true == IsEnabled()))
            state = sbSUp;

    if (FSelected)
        state = sbSDown;

    BeginLineArray((FHighlighted ? 8 : 4));

    switch (state)
    {
        // Draw Down
	    case sbSDown:
	        AddLine(FBorder.LeftTop(), FBorder.LeftBottom(), shadow);
	        AddLine(FBorder.LeftBottom(), FBorder.RightBottom(), highlight);
	        AddLine(FBorder.RightTop(), FBorder.RightBottom(), highlight);
	        AddLine(FBorder.LeftTop(), FBorder.RightTop(), shadow);
	        break;
	
        // Draw Flat
	    case sbSFlat:
	        AddLine(FBorder.LeftTop(), FBorder.LeftBottom(), viewcolour);
	        AddLine(FBorder.LeftBottom(), FBorder.RightBottom(), viewcolour);
	        AddLine(FBorder.RightTop(), FBorder.RightBottom(), viewcolour);
	        AddLine(FBorder.LeftTop(), FBorder.RightTop(), viewcolour);
	        break;

        // Draw Up
	    case sbSUp:
	        AddLine(FBorder.LeftTop(), FBorder.LeftBottom(), highlight);
	        AddLine(FBorder.LeftBottom(), FBorder.RightBottom(), shadow);
	        AddLine(FBorder.RightTop(), FBorder.RightBottom(), shadow);
	        AddLine(FBorder.LeftTop(), FBorder.RightTop(), highlight);
	        break;
    }

    if (true == FHighlighted)
    {
        BRect highlightRect(FBorder);
        highlightRect.InsetBy(1, 1);
        AddLine(highlightRect.LeftTop(),    highlightRect.LeftBottom(),  FHighlightColor);
        AddLine(highlightRect.LeftBottom(), highlightRect.RightBottom(), FHighlightColor);
        AddLine(highlightRect.RightTop(),   highlightRect.RightBottom(), FHighlightColor);
        AddLine(highlightRect.LeftTop(),    highlightRect.RightTop(),    FHighlightColor);
    }

    EndLineArray();
}
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void TSpeedButton::MouseDown(BPoint Where)
{
    if (true == BControl::IsEnabled())
    {
        SetMouseEventMask(B_POINTER_EVENTS);
        FMouseDown = true;

        if (false == FSelected)
        {
        	rgb_color color = HighColor();
            SetHighColor(ViewColor());
            StrokeLine(FBorder.LeftTop(), FBorder.LeftBottom());
            StrokeLine(FBorder.LeftTop(), FBorder.RightTop());

            // clean up highlight drawing.
            if (true == FHighlighted)
            {
                StrokeLine(FBorder.InsetByCopy(1, 1).LeftTop(), FBorder.InsetByCopy(1, 1).LeftBottom());
                StrokeLine(FBorder.InsetByCopy(1, 1).LeftTop(), FBorder.InsetByCopy(1, 1).RightTop());
            }

            if ( Window()->Lock() )
            {
                Draw(FBorder);
                Window()->Unlock();
            }
            
            SetHighColor(color);
        }
    }
    
    BControl::MouseDown(Where);
}
//-------------------------------------------------------------------------------
void TSpeedButton::MouseUp(BPoint Where)
{
    // Save some clock cycles
    if ((!IsEnabled()) || (FSelected && (FGroupIndex > -1) && !FLatching))
    {
		FMouseDown = false;
        return;
    }

    // check to see if we clicked down on this one.
    if (FMouseDown)
    {
        // mouse might have moved off of us
        if (FOutside == false)
        {
            // we may not be enabled
            if (true == BControl::IsEnabled())
            {
                if (FLatching && FSelected)
                {
                    FSelected = false;
                    SetHighColor(ViewColor());
                    StrokeLine(FBorder.LeftBottom(), FBorder.RightBottom());
                    StrokeLine(FBorder.RightTop(), FBorder.RightBottom());


                    // clean up highlight drawing.
                    if (true == FHighlighted)
                    {
                        StrokeLine(FBorder.InsetByCopy(1, 1).LeftBottom(), FBorder.InsetByCopy(1, 1).RightBottom());
                        StrokeLine(FBorder.InsetByCopy(1, 1).RightTop(), FBorder.InsetByCopy(1, 1).RightBottom());
                    }

                    FMouseDown = false;
                    Draw(FBorder);
                    BControl::SetValue(0);
                    BControl::Invoke();
                    return;
                }

                // todo: Check for impossible(?) situation where the button is not
                // attached to a window?

                if (FLatching)
                {
                    FSelected = true;
                    BControl::SetValue(1);
                }

                if (FGroupIndex > -1)
                {
                    // De-select all, then select this one.
                    BView* sibView = Parent()->ChildAt(0);
                    TSpeedButton* sibling;

                    while(sibView)
                    {
                        sibling = dynamic_cast<TSpeedButton*>(sibView);

                        if ((0 != sibling) && (this != sibling))
                        {
                            // Only pop other buttons up if they are selected and they are of the same
                            // group index and they are not of the latching type.
                            // Todo: determine if this should be different in terms of the latching state ...
                            if((true == sibling->FSelected) &&
                               (sibling->FGroupIndex == FGroupIndex) &&
                               (false == sibling->FLatching) )
                            {
                                sibling->Selected(false);
                            }
                        }
                        sibView = sibView->NextSibling();
                    }
                    FSelected = true;
                }
                else
                {
                    if (false == FLatching)
                    {
                        SetHighColor(ViewColor());
                        StrokeLine(FBorder.LeftBottom(), FBorder.RightBottom());
                        StrokeLine(FBorder.RightTop(), FBorder.RightBottom());

                        // clean up highlight drawing.
                        if (true == FHighlighted)
                        {
                            StrokeLine(FBorder.InsetByCopy(1, 1).LeftBottom(), FBorder.InsetByCopy(1, 1).RightBottom());
                            StrokeLine(FBorder.InsetByCopy(1, 1).RightTop(), FBorder.InsetByCopy(1, 1).RightBottom());
                        }
                    }
                }
                BControl::Invoke();
            }
        }

        FMouseDown = false;
        if (false == FSelected)
        {
            Draw(FBorder);
        }
    }
    else
    {
        // We had the button down when we entered, but we are now
        // lifting up, so lets activate it.
        FOutside = false;
        FActive  = true;

        Draw(FBorder);
    }
    
    BControl::MouseUp(Where);
}
//-------------------------------------------------------------------------------
void TSpeedButton::MouseMoved(BPoint Where, uint32 Code, const BMessage* Message)
{
    switch(Code)
    {
	   case B_INSIDE_VIEW :
	       break;
	
	   case B_ENTERED_VIEW :
       {
           uint32 buttons = 0;
           BPoint pt;

           GetMouse(&pt, &buttons);

           if (buttons && !FMouseDown)
           {
               return;
           }

           // We left the button with the mouse down, but are comming back now.
           if (FMouseDown && FOutside)
           {
               SetHighColor(ViewColor());
               StrokeLine(FBorder.LeftTop(), FBorder.RightTop());
               StrokeLine(FBorder.LeftTop(), FBorder.LeftBottom());

               // clean up highlight drawing.
               if (true == FHighlighted)
               {
                   StrokeLine(FBorder.InsetByCopy(1, 1).LeftTop(), FBorder.InsetByCopy(1, 1).RightTop());
                   StrokeLine(FBorder.InsetByCopy(1, 1).LeftTop(), FBorder.InsetByCopy(1, 1).LeftBottom());
               }

           }

           FOutside = false;
           FActive = true;

           if ( Window()->Lock() )
           {
               Draw(FBorder);
               Window()->Unlock();
           }
           break;
       }

	   case B_EXITED_VIEW :
       {
           FActive = false;
           FOutside = true;

           if (true == FMouseDown)
           {
               // Left the button with the mouse down, need to put it back to neutral.
               SetHighColor(ViewColor());
               StrokeLine(FBorder.LeftBottom(), FBorder.RightBottom());
               StrokeLine(FBorder.RightTop(), FBorder.RightBottom());

               if (true == FHighlighted)
               {
                   StrokeLine(FBorder.InsetByCopy(1, 1).LeftBottom(), FBorder.InsetByCopy(1, 1).RightBottom());
                   StrokeLine(FBorder.InsetByCopy(1, 1).RightTop(), FBorder.InsetByCopy(1, 1).RightBottom());
               }
           }

           if ( Window()->Lock() )
           {
               Draw(FBorder);
               Window()->Unlock();
           }

           break;
       }
    }
    
    BControl::MouseMoved(Where, Code, Message);
}
//-------------------------------------------------------------------------------
void TSpeedButton::FrameResized(float Width, float Height)
{
    BControl::FrameResized(Width, Height);

    if ( FEnabledBitmap )
    {
        SetupBitmaps(FEnabledBitmap);
    }
}
//-------------------------------------------------------------------------------
TSBStyle TSpeedButton::Style()
{
    return FStyle;
}
//-------------------------------------------------------------------------------
void TSpeedButton::Style(TSBStyle Style)
{
    FStyle = Style;
}
//-------------------------------------------------------------------------------
void TSpeedButton::GroupIndex(int32 Index)
{
    FGroupIndex = Index;
}
//-------------------------------------------------------------------------------
int32 TSpeedButton::GroupIndex()
{
    return FGroupIndex;
}
//-------------------------------------------------------------------------------
void TSpeedButton::Selected(bool Selected)
{
    if (FSelected != Selected)
	{
	    FSelected = Selected;

	    // Reflect the state of the button appropriately
	    if (true == FAttachedToWindow)
	    {
	        BeginLineArray(2);
	        AddLine(FBorder.RightTop(), FBorder.RightBottom(), Parent()->ViewColor());
	        AddLine(FBorder.LeftBottom(), FBorder.RightBottom(), Parent()->ViewColor());
	        EndLineArray();
	        Draw(FBorder);
	    }
	}
}
//-------------------------------------------------------------------------------
bool TSpeedButton::Selected()
{
    return FSelected;
}
//-------------------------------------------------------------------------------
void TSpeedButton::Latching(bool Latching)
{
    FLatching = Latching;
}
//-------------------------------------------------------------------------------
bool TSpeedButton::Latching()
{
    return FLatching;
}
//-------------------------------------------------------------------------------
void TSpeedButton::Highlighted(bool Highlighted)
{
    FHighlighted = Highlighted;
    Invalidate();
    Draw(FBorder);
}
//-------------------------------------------------------------------------------
bool TSpeedButton::Highlighted()
{
    return FHighlighted;
}
//-------------------------------------------------------------------------------
void TSpeedButton::HighlightColor(rgb_color HighlightColor)
{
    FHighlightColor = HighlightColor;
    Draw(FBorder);
}
//-------------------------------------------------------------------------------
rgb_color TSpeedButton::HighlightColor()
{
    return FHighlightColor;
}
//-------------------------------------------------------------------------------
