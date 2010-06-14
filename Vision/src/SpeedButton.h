#ifndef SPEEDBUTTON_H
#define SPEEDBUTTON_H

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


class BLooper;
class BBitmap;

#include <interface/Control.h>
#include <interface/Rect.h>

// How many pixels around the edge we need for comfort.
#define EDGE_SPACE 0


enum TSBStyle
{
	sbFlat,
	sbUp
};

/**
 * @short A nifty X-style button with a graphic..
 * This class is a BButton derivative with some cool features.
 * @author Alan Ellis
 */
class TSpeedButton : public BControl
{
	public:
			/**
			 * Standard stuff for BButtons with some additions.
			 * @param Label Set this to NULL if you do not wish to have a label drawn on your button.
			 * @param EnabledBitmap The bitmap which shows the button in an enabled state.
			 *											Copies are made of the bitmaps that you pass in. You still own them.
			 * @param Style Optional style. Avaliable styles are sbFlat (pops up as the mouse goes over it)
			 *							and sbUp, normal looking button.
			 * @param DisabledBitmap Optional look for a disabled bitmap.
			 *											 If this is not supplied, a disabled button will be created from the
			 *											 Enabled bitmap
			 */
			TSpeedButton(BRect Frame, const char *Name, const char* Label,
									 BMessage *Message, BBitmap *EnabledBitmap,
									 TSBStyle Style = sbFlat,
									 BBitmap *DisabledBitmap = NULL,
									 uint32 ResizingMask = B_FOLLOW_LEFT | B_FOLLOW_TOP,
									 uint32 Flags = B_WILL_DRAW);
	
			/**
			 * Standard unarchiving constructor.	
			 */					
			TSpeedButton(BMessage *Archive);
	
			/**
			 * Goes with above constructor.	
			 */					
			static TSpeedButton *Instantiate(BMessage *Archive);
	
			/**
			 * Standard destructor.	
			 */					
			virtual ~TSpeedButton();
	
			/**
			 * More archiving stuff.	
			 */					
			virtual status_t Archive(BMessage *Archive, bool Deep = true) const;
	
			virtual void AttachedToWindow();
			virtual void DetachedFromWindow();
			virtual void Draw(BRect Frame);
	
			virtual void MouseDown(BPoint where);
			virtual void MouseUp(BPoint where);
			virtual void MouseMoved(BPoint where,
															uint32 code,
															const BMessage *a_message);
	
			virtual void FrameResized(float Width, float Height);
	
			/**
			 * What style is the button? You need but ask.
			 */					
			virtual TSBStyle	 Style();
	
			/**
			 * Set the style.
			 * @param Style Set to one of 'sbFlat' or 'sbUp'. See the constructor for explanation of the vaules.
			 */
			virtual void			 Style(TSBStyle Style);
	
			/**
			 * Used to 'group' buttons owned by the same view for 'radio' functionality.
			 * @param Index All buttons belonging to the same index will be in the same group. A group index of
			 * -1 indicates a button is not part of any group.
			 */
			virtual void			 GroupIndex(int32 Index);
	
			/**
			 * Oh great oracle, to which group does my button belong? 
			 */
			virtual int32			GroupIndex();
	
			/**
			 * Primarily useful to buttons in Groups or latching, a button is seleced when it is in the 'down' state.
			 */
			virtual void			 Selected(bool Selected);
			virtual bool			 Selected();
	
			/**
			 * A button that 'latches' will stick down when pressed. If the button is part of a group the other buttons in the
			 * group will become un-latched.
			 */
			virtual void			 Latching(bool Latching);
			virtual bool			 Latching();
	
			/**
			 *	You may want to show that a button is in a special state, Highlighting can do this for you by drawing an outline
			 *	inside the edge of the button.
			 */
			virtual void			 Highlighted(bool Highlighted);
			virtual bool			 Highlighted();
			virtual void			 HighlightColor(rgb_color HighlightColor);
			virtual rgb_color	HighlightColor();
	
	protected:
	
	private:
				void		SetupBitmaps(BBitmap* Source);
				void		SetupLabel();
			
	private:
			enum TSBDrawState { sbSDown, sbSFlat, sbSUp };
	
			BBitmap*	fDisabledBitmap;
			BBitmap*	fEnabledBitmap;
			BRect		fBitmapSourceRect;
			BRect		fBitmapDestinationRect;
			BRect		fBorder;
			TSBStyle	fStyle;
			int32		fGroupIndex;
			rgb_color	fHighlightColor;
			rgb_color	fEnabledViewColor;
			BPoint		fLabelPos;
	
			bool			fActive;
			bool			fMouseDown;
			bool			fOutside;
			bool		fAttachedToWindow;
			bool			fSelected;
			bool			fLatching;
			bool			fHighlighted;	
};

#endif

