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

#include <interface/Window.h>
#include <interface/Bitmap.h>
#include <interface/Rect.h>
#include <app/Looper.h>

#include "SpeedButton.h"

//-------------------------------------------------------------------------------
TSpeedButton::TSpeedButton(BRect FrameRect, const char* NameStr, const char* LabelStr,
													 BMessage* InvokeMsg, BBitmap* EnabledBitmap,
													 TSBStyle BtnStyle,
													 BBitmap* DisabledBitmap,
													 uint32 ResizingMask,
													 uint32 BtnFlags)
				:BControl(FrameRect, NameStr, LabelStr, InvokeMsg, ResizingMask, BtnFlags),
				fDisabledBitmap(NULL),
				fEnabledBitmap(NULL),
				fBitmapDestinationRect(0, 0, 0, 0),
				fBorder(Bounds()),
				fStyle(BtnStyle),
				fGroupIndex(-1),
				fHighlightColor(keyboard_navigation_color()),
				fLabelPos(0, 0),
				fActive(false),
				fMouseDown(false),
				fOutside(true),
				fAttachedToWindow(false),
				fSelected(false),
				fLatching(false),
				fHighlighted(false)
{
		// make a copy of the disabled bitmap if needed.
		if (DisabledBitmap != NULL)
		{
				fDisabledBitmap = new BBitmap(DisabledBitmap);
		}

		if (EnabledBitmap != NULL)
		{
				fEnabledBitmap = new BBitmap(EnabledBitmap);
				SetupBitmaps(fEnabledBitmap);
		}
		
		if( NULL != BControl::Label() )
		{
			SetupLabel();
		}
}
//-------------------------------------------------------------------------------
TSpeedButton::TSpeedButton(BMessage* Arch)
				:BControl(Arch)
{
		Arch->FindBool("Deco::TSpeedButton:fSelected", &fSelected);
		Arch->FindInt32("Deco::TSpeedButton:fGroupIndex", &fGroupIndex);

		fBorder = Bounds();

		BMessage *BitmapArchiveD = NULL;
		BMessage *BitmapArchiveE = NULL;

		Arch->FindMessage("DCL::TSpeedButton:fDisabledBitmap", BitmapArchiveD);
		Arch->FindMessage("DCL::TSpeedButton:fEnabledBitmap", BitmapArchiveE);

		fDisabledBitmap = dynamic_cast<BBitmap *>(BBitmap::Instantiate(BitmapArchiveD));
		fEnabledBitmap = dynamic_cast<BBitmap *>(BBitmap::Instantiate(BitmapArchiveE));

		if (NULL != fEnabledBitmap)
				SetupBitmaps(fEnabledBitmap);

		fActive = false;
		fAttachedToWindow = false;
}
//-------------------------------------------------------------------------------
TSpeedButton *TSpeedButton::Instantiate(BMessage* Archie)
{
		if ( validate_instantiation(Archie, "TSpeedButton"))
				return new TSpeedButton(Archie);

		return NULL;
}
//-------------------------------------------------------------------------------
status_t TSpeedButton::Archive(BMessage *Archie, bool Deep) const
{
		BView::Archive(Archie, Deep);
		Archie->AddString("class", "TSpeedButton");

		BMessage BitmapArchiveD;
		BMessage BitmapArchiveE;

		if (fDisabledBitmap != NULL)
		{
				fDisabledBitmap->Archive(&BitmapArchiveD);
				Archie->AddMessage("DCL::TSpeedButton:fDisabledBitmap", &BitmapArchiveD);
		}

		if (fEnabledBitmap != NULL)
		{
				fEnabledBitmap->Archive(&BitmapArchiveE);
				Archie->AddMessage("DCL::TSpeedButton:fDisabledBitmap", &BitmapArchiveE);
		}

		Archie->AddBool("DCL::TSpeedButton:fSelected", fSelected);
		Archie->AddInt32("DCL::TSpeedButton:fGroupIndex", fGroupIndex);

		// todo: Forward error checking on?
		return B_OK;
}
//-------------------------------------------------------------------------------
void TSpeedButton::SetupBitmaps(BBitmap* SourceBitmap)
{
		// We need EDGE_SPACE pixels clear on every side for the various button functions
		fBitmapDestinationRect = Bounds().InsetByCopy(EDGE_SPACE, EDGE_SPACE);

		// setup the Source and Destination bitmap rects, clipping when necessary.
		BRect workRect(SourceBitmap->Bounds());

		// Determine if any work needs to be done, the offset is for easy comparison
		if (workRect.OffsetByCopy(EDGE_SPACE, EDGE_SPACE) != fBitmapDestinationRect)
		{
				// The source may be taller
				if (workRect.IntegerHeight() > fBitmapDestinationRect.IntegerHeight())
				{
						// Move the source down by half the difference and clip.
						workRect.OffsetBy(0, (workRect.IntegerHeight() - fBitmapDestinationRect.IntegerHeight()) / 2);
						workRect.bottom = workRect.top + fBitmapDestinationRect.IntegerHeight();
				}

				// The source may be wider
				if (workRect.IntegerWidth() > fBitmapDestinationRect.IntegerWidth())
				{
						// Move the source over by half the difference and clip.
						workRect.OffsetBy((workRect.IntegerWidth() - fBitmapDestinationRect.IntegerWidth()) / 2, 0);
						workRect.right = workRect.left + fBitmapDestinationRect.IntegerWidth();
				}

				// The source may be shorter
				if (workRect.IntegerHeight() < fBitmapDestinationRect.IntegerHeight())
				{
						// Move the destination right by half the difference and clip
						fBitmapDestinationRect.OffsetBy(0, (fBitmapDestinationRect.IntegerHeight() - workRect.IntegerHeight()) / 2);
						fBitmapDestinationRect.bottom = fBitmapDestinationRect.top + workRect.IntegerHeight();
				}

				// The source may be narrower
				if (workRect.IntegerWidth() < fBitmapDestinationRect.IntegerWidth())
				{
						// Move the destination right by half the difference and clip
						fBitmapDestinationRect.OffsetBy((fBitmapDestinationRect.IntegerWidth() - workRect.IntegerWidth()) / 2, 0);
						fBitmapDestinationRect.right = fBitmapDestinationRect.left + workRect.IntegerWidth();
				}
		}

		fBitmapSourceRect = workRect;
}
//-------------------------------------------------------------------------------
void TSpeedButton::SetupLabel()
{
	// This will look real funny if the button is not made large enough for the label.

	// Figure out where the label lives.
	fLabelPos.y = Bounds().bottom - EDGE_SPACE;
	
	fLabelPos.x = (Bounds().Width() / 2.0) - (StringWidth(Label()) / 2.0 );
	
	// Move the destination accordingly.
	fBitmapDestinationRect = fBitmapSourceRect;
	
	font_height fontheight;
	GetFontHeight(&fontheight);
	
	float availHeight = Bounds().Height() - (fontheight.ascent +
											fontheight.descent + fontheight.leading +
											(2 * EDGE_SPACE));
	
	float top = (availHeight / 2.0) - (fBitmapSourceRect.Height() / 2.0) + EDGE_SPACE;
	float left = ( Bounds().Width() / 2.0 ) - (fBitmapSourceRect.Width() / 2.0);
	
	fBitmapDestinationRect.OffsetTo(left, top);	
}
//-------------------------------------------------------------------------------
void TSpeedButton::AttachedToWindow()
{
		SetViewColor(Parent()->ViewColor());

		fEnabledViewColor = Parent()->ViewColor();
		SetBlendingMode( B_PIXEL_ALPHA, B_ALPHA_OVERLAY );
		
		if( (NULL == fDisabledBitmap) && (NULL != fEnabledBitmap))
		{
		if ( fEnabledBitmap && (B_RGBA32 != fEnabledBitmap->ColorSpace() ))
		{
			// translate to 32 bit image ...
			BBitmap* temp = new BBitmap(fEnabledBitmap->Bounds(), B_RGBA32, true);
			BView aView(fEnabledBitmap->Bounds(), "", 0, B_WILL_DRAW);
			temp->AddChild(&aView);
			aView.Window()->Lock();
			aView.DrawBitmap(fEnabledBitmap);
			aView.Window()->Unlock();
			temp->RemoveChild(&aView);
			fDisabledBitmap = temp;
		}
		else
		{
			if (fEnabledBitmap)
				fDisabledBitmap = new BBitmap(fEnabledBitmap);
		}
		
		uint32 bitend = fDisabledBitmap->BitsLength() / 4;
		
		// shade the image
		for( uint32 i = 0; i < bitend; i++ )
		{
			rgb_color* color = (rgb_color*)(&((uint32*)(fDisabledBitmap->Bits()))[i]);

			// todo: Should we use right shift by one to speed up this loop?
			color->red += ((fEnabledViewColor.red - color->red) / 2); 
			color->blue += ((fEnabledViewColor.blue - color->blue) / 2);
			color->green += ((fEnabledViewColor.green - color->green) / 2);
		}		
		}

		fAttachedToWindow = true;
		
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
	delete fDisabledBitmap;
	delete fEnabledBitmap;
}
//-------------------------------------------------------------------------------
void TSpeedButton::Draw(BRect UpdateRect)
{
		BControl::Draw(UpdateRect);
	fBorder = Bounds();

		rgb_color viewcolour = Parent()->ViewColor();
		rgb_color highlight	= tint_color(viewcolour, B_LIGHTEN_MAX_TINT);
		rgb_color shadow		 = tint_color(viewcolour, B_DARKEN_2_TINT);


		// In order to draw with alpha, we have to do it ourselves rather than
		//	just set the background bitmap.
		drawing_mode mode = DrawingMode();
		SetDrawingMode(B_OP_ALPHA);

		if (fEnabledBitmap != NULL)
		{
				if (true == BControl::IsEnabled())
				{
						DrawBitmapAsync(fEnabledBitmap, fBitmapSourceRect, fBitmapDestinationRect);
				}
				else
				{
						if (fDisabledBitmap != NULL)
						{
								DrawBitmapAsync(fDisabledBitmap, fBitmapSourceRect, fBitmapDestinationRect);
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
		
		DrawString(Label(), fLabelPos);
	}

		TSBDrawState state = sbSFlat;

		if (true == BControl::IsEnabled())
				if (!fOutside)
						if (fMouseDown)
								state = sbSDown;

		if (fStyle == sbUp)
				if (!fMouseDown || fOutside)
						state = sbSUp;

		if (fActive)
				if (!fMouseDown && (true == IsEnabled()))
						state = sbSUp;

		if (fSelected)
				state = sbSDown;

		BeginLineArray((fHighlighted ? 8 : 4));

		switch (state)
		{
				// Draw Down
			case sbSDown:
					AddLine(fBorder.LeftTop(), fBorder.LeftBottom(), shadow);
					AddLine(fBorder.LeftBottom(), fBorder.RightBottom(), highlight);
					AddLine(fBorder.RightTop(), fBorder.RightBottom(), highlight);
					AddLine(fBorder.LeftTop(), fBorder.RightTop(), shadow);
					break;
	
				// Draw Flat
			case sbSFlat:
					AddLine(fBorder.LeftTop(), fBorder.LeftBottom(), viewcolour);
					AddLine(fBorder.LeftBottom(), fBorder.RightBottom(), viewcolour);
					AddLine(fBorder.RightTop(), fBorder.RightBottom(), viewcolour);
					AddLine(fBorder.LeftTop(), fBorder.RightTop(), viewcolour);
					break;

				// Draw Up
			case sbSUp:
					AddLine(fBorder.LeftTop(), fBorder.LeftBottom(), highlight);
					AddLine(fBorder.LeftBottom(), fBorder.RightBottom(), shadow);
					AddLine(fBorder.RightTop(), fBorder.RightBottom(), shadow);
					AddLine(fBorder.LeftTop(), fBorder.RightTop(), highlight);
					break;
		}

		if (true == fHighlighted)
		{
				BRect highlightRect(fBorder);
				highlightRect.InsetBy(1, 1);
				AddLine(highlightRect.LeftTop(),		highlightRect.LeftBottom(),	fHighlightColor);
				AddLine(highlightRect.LeftBottom(), highlightRect.RightBottom(), fHighlightColor);
				AddLine(highlightRect.RightTop(),	 highlightRect.RightBottom(), fHighlightColor);
				AddLine(highlightRect.LeftTop(),		highlightRect.RightTop(),		fHighlightColor);
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
				fMouseDown = true;

				if (false == fSelected)
				{
					rgb_color color = HighColor();
						SetHighColor(ViewColor());
						StrokeLine(fBorder.LeftTop(), fBorder.LeftBottom());
						StrokeLine(fBorder.LeftTop(), fBorder.RightTop());

						// clean up highlight drawing.
						if (true == fHighlighted)
						{
								StrokeLine(fBorder.InsetByCopy(1, 1).LeftTop(), fBorder.InsetByCopy(1, 1).LeftBottom());
								StrokeLine(fBorder.InsetByCopy(1, 1).LeftTop(), fBorder.InsetByCopy(1, 1).RightTop());
						}

						if ( Window()->Lock() )
						{
								Draw(fBorder);
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
		if ((!IsEnabled()) || (fSelected && (fGroupIndex > -1) && !fLatching))
		{
		fMouseDown = false;
				return;
		}

		// check to see if we clicked down on this one.
		if (fMouseDown)
		{
				// mouse might have moved off of us
				if (fOutside == false)
				{
						// we may not be enabled
						if (true == BControl::IsEnabled())
						{
								if (fLatching && fSelected)
								{
										fSelected = false;
										SetHighColor(ViewColor());
										StrokeLine(fBorder.LeftBottom(), fBorder.RightBottom());
										StrokeLine(fBorder.RightTop(), fBorder.RightBottom());


										// clean up highlight drawing.
										if (true == fHighlighted)
										{
												StrokeLine(fBorder.InsetByCopy(1, 1).LeftBottom(), fBorder.InsetByCopy(1, 1).RightBottom());
												StrokeLine(fBorder.InsetByCopy(1, 1).RightTop(), fBorder.InsetByCopy(1, 1).RightBottom());
										}

										fMouseDown = false;
										Draw(fBorder);
										BControl::SetValue(0);
										BControl::Invoke();
										return;
								}

								// todo: Check for impossible(?) situation where the button is not
								// attached to a window?

								if (fLatching)
								{
										fSelected = true;
										BControl::SetValue(1);
								}

								if (fGroupIndex > -1)
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
														if((true == sibling->fSelected) &&
															 (sibling->fGroupIndex == fGroupIndex) &&
															 (false == sibling->fLatching) )
														{
																sibling->Selected(false);
														}
												}
												sibView = sibView->NextSibling();
										}
										fSelected = true;
								}
								else
								{
										if (false == fLatching)
										{
												SetHighColor(ViewColor());
												StrokeLine(fBorder.LeftBottom(), fBorder.RightBottom());
												StrokeLine(fBorder.RightTop(), fBorder.RightBottom());

												// clean up highlight drawing.
												if (true == fHighlighted)
												{
														StrokeLine(fBorder.InsetByCopy(1, 1).LeftBottom(), fBorder.InsetByCopy(1, 1).RightBottom());
														StrokeLine(fBorder.InsetByCopy(1, 1).RightTop(), fBorder.InsetByCopy(1, 1).RightBottom());
												}
										}
								}
								BControl::Invoke();
						}
				}

				fMouseDown = false;
				if (false == fSelected)
				{
						Draw(fBorder);
				}
		}
		else
		{
				// We had the button down when we entered, but we are now
				// lifting up, so lets activate it.
				fOutside = false;
				fActive	= true;

				Draw(fBorder);
		}
		
		BControl::MouseUp(Where);
}
//-------------------------------------------------------------------------------
void TSpeedButton::MouseMoved(BPoint Where, uint32 Code, const BMessage* Msg)
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

					 if (buttons && !fMouseDown)
					 {
							 return;
					 }

					 // We left the button with the mouse down, but are comming back now.
					 if (fMouseDown && fOutside)
					 {
							 SetHighColor(ViewColor());
							 StrokeLine(fBorder.LeftTop(), fBorder.RightTop());
							 StrokeLine(fBorder.LeftTop(), fBorder.LeftBottom());

							 // clean up highlight drawing.
							 if (true == fHighlighted)
							 {
									 StrokeLine(fBorder.InsetByCopy(1, 1).LeftTop(), fBorder.InsetByCopy(1, 1).RightTop());
									 StrokeLine(fBorder.InsetByCopy(1, 1).LeftTop(), fBorder.InsetByCopy(1, 1).LeftBottom());
							 }

					 }

					 fOutside = false;
					 fActive = true;

					 if ( Window()->Lock() )
					 {
							 Draw(fBorder);
							 Window()->Unlock();
					 }
					 break;
			 }

		 case B_EXITED_VIEW :
			 {
					 fActive = false;
					 fOutside = true;

					 if (true == fMouseDown)
					 {
							 // Left the button with the mouse down, need to put it back to neutral.
							 SetHighColor(ViewColor());
							 StrokeLine(fBorder.LeftBottom(), fBorder.RightBottom());
							 StrokeLine(fBorder.RightTop(), fBorder.RightBottom());

							 if (true == fHighlighted)
							 {
									 StrokeLine(fBorder.InsetByCopy(1, 1).LeftBottom(), fBorder.InsetByCopy(1, 1).RightBottom());
									 StrokeLine(fBorder.InsetByCopy(1, 1).RightTop(), fBorder.InsetByCopy(1, 1).RightBottom());
							 }
					 }

					 if ( Window()->Lock() )
					 {
							 Draw(fBorder);
							 Window()->Unlock();
					 }

					 break;
			 }
		}
		
		BControl::MouseMoved(Where, Code, Msg);
}
//-------------------------------------------------------------------------------
void TSpeedButton::FrameResized(float Width, float Height)
{
		BControl::FrameResized(Width, Height);

		if ( fEnabledBitmap )
		{
				SetupBitmaps(fEnabledBitmap);
		}
}
//-------------------------------------------------------------------------------
TSBStyle TSpeedButton::Style()
{
		return fStyle;
}
//-------------------------------------------------------------------------------
void TSpeedButton::Style(TSBStyle BtnStyle)
{
		fStyle = BtnStyle;
}
//-------------------------------------------------------------------------------
void TSpeedButton::GroupIndex(int32 Index)
{
		fGroupIndex = Index;
}
//-------------------------------------------------------------------------------
int32 TSpeedButton::GroupIndex()
{
		return fGroupIndex;
}
//-------------------------------------------------------------------------------
void TSpeedButton::Selected(bool SetSelected)
{
		if (fSelected != SetSelected)
	{
			fSelected = SetSelected;

			// Reflect the state of the button appropriately
			if (true == fAttachedToWindow)
			{
					BeginLineArray(2);
					AddLine(fBorder.RightTop(), fBorder.RightBottom(), Parent()->ViewColor());
					AddLine(fBorder.LeftBottom(), fBorder.RightBottom(), Parent()->ViewColor());
					EndLineArray();
					Draw(fBorder);
			}
	}
}
//-------------------------------------------------------------------------------
bool TSpeedButton::Selected()
{
		return fSelected;
}
//-------------------------------------------------------------------------------
void TSpeedButton::Latching(bool SetLatching)
{
		fLatching = SetLatching;
}
//-------------------------------------------------------------------------------
bool TSpeedButton::Latching()
{
		return fLatching;
}
//-------------------------------------------------------------------------------
void TSpeedButton::Highlighted(bool SetHighlighted)
{
		fHighlighted = SetHighlighted;
		Invalidate();
		Draw(fBorder);
}
//-------------------------------------------------------------------------------
bool TSpeedButton::Highlighted()
{
		return fHighlighted;
}
//-------------------------------------------------------------------------------
void TSpeedButton::HighlightColor(rgb_color BtnHighlightColor)
{
		fHighlightColor = BtnHighlightColor;
		Draw(fBorder);
}
//-------------------------------------------------------------------------------
rgb_color TSpeedButton::HighlightColor()
{
		return fHighlightColor;
}
//-------------------------------------------------------------------------------
