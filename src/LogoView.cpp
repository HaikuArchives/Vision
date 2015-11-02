#include <Application.h>
#include <Message.h>
#include <Resources.h>

#include "LogoView.h"
#include "Vision.h"


LogoView::LogoView(BRect frame)
	:
	BView(frame, "LogoView", B_FOLLOW_NONE, B_WILL_DRAW | B_FRAME_EVENTS),
	fLogo(NULL)
{
	BResources* rsrcs = BApplication::AppResources();
	size_t size = 0;
	char* data = (char*)rsrcs->FindResource('MSGG', "vision-logo", &size);
	BMessage archive;

	if (data != NULL && archive.Unflatten(data) == B_OK)
		fLogo = new BBitmap(&archive);

	FrameResized(frame.Width(), frame.Height());
}


LogoView::~LogoView()
{
	delete fLogo;
}


void
LogoView::AttachedToWindow()
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetDrawingMode(B_OP_ALPHA);
	SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_COMPOSITE);
}


void
LogoView::Draw(BRect rect)
{
	if (fLogo != NULL)
		DrawBitmap(fLogo, fLogo->Bounds(), fLogoFrame);
}


void
LogoView::FrameResized(float width, float height)
{
	if (fLogo == NULL)
		return;

	BRect rect = fLogo->Bounds();

	fLogoFrame = rect;
	fLogoFrame.OffsetBy(width / 2 - rect.Width() / 2,
		height / 2 - rect.Height() / 2);

	fLogoFrame = fLogoFrame & Bounds();
	fLogoFrame.InsetBy(kItemSpacing, kItemSpacing);

	Invalidate();
}


void
LogoView::MouseDown(BPoint point)
{
	if (fLogo != NULL && !fLogoFrame.Contains(point))
		return;

	vision_app->LoadURL("http://vision.sourceforge.net");
}


int32
LogoView::PreferredHeight()
{
	if (fLogo == NULL)
		return 250;

	return (int32)fLogo->Bounds().Height();
}


