#ifndef COLOR_SELECTOR_H
#define COLOR_SELECTOR_H

#include <Message.h>
#include <Control.h>
#include <String.h>

/*------------------------------------------------------------*/

class BPopUpMenu;
class BMenuField;
class BColorControl;

#if !B_BEOS_VERSION_DANO
class ColorSwatch;
#endif

class ColorSelector : public BControl {
public:
						ColorSelector(	BRect frame,
										const char* name,
										const char* label,
										const BMessage& colors,
										const BMessage& names,
										BMessage* model,
										uint32 resizeMask=B_FOLLOW_LEFT|B_FOLLOW_TOP,
										uint32 flags=B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE_JUMP);
						~ColorSelector();

static	void			ExtractColors(BMessage* target, const BMessage& src);

virtual	void			AttachedToWindow();
virtual void			AllAttached();
virtual	void			MessageReceived(BMessage *msg);
virtual	void			FrameResized(float width, float height);
virtual	void			GetPreferredSize(float* width, float* height);

		void			SetTo(const BMessage& colors);
		void			Update(const BMessage& changes);
		void			Revert();
		
		bool			IsDirty() const;
		
		const BMessage&	CurrentColors() const;
		const BMessage&	InitialColors() const;
		
private:
		void			LayoutViews(bool really);

		BMessage		fNames;
		BMessage		fInitColors;
		BMessage		fColors;
		
		BPopUpMenu*		fColorMenu;
		BMenuField*		fColorField;
		BColorControl*	fColorPalette;
		
		BString			fCurrentField;
		
		bool			fSizeValid;
		float			fPrefWidth, fPrefHeight;
#if !B_BEOS_VERSION_DANO
		ColorSwatch		*swatch;
#endif
};

/*------------------------------------------------------------*/

#endif
