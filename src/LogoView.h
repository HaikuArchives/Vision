#ifndef _LOGOVIEW_H
#define _LOGOVIEW_H


#include <Bitmap.h>
#include <View.h>
#include <String.h>


class LogoView : public BView {
public:
								LogoView(BRect frame, int32 border);
	virtual						~LogoView();

	virtual	void				AttachedToWindow();
	virtual void				Draw(BRect rect);
	virtual void				FrameResized(float width, float height);
	virtual	void				MouseDown(BPoint point);

			int32				PreferredHeight();

private:
			BBitmap*			fLogo;
			BRect				fLogoFrame;
			int32				fBorder;
};



#endif // _LOGOVIEW_H
