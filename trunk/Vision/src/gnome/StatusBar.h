#ifndef	STATUS_BAR_H__
#define	STATUS_BAR_H__

#include <PlatformDefines.h>
#include <GraphicDefs.h>
#include <View.h>

class FStatusBar : public FView {
public:
	FStatusBar(BRect frame, const char *name, const char *label = NULL,
		   const char *trailing_label = NULL);
	virtual				~FStatusBar();
	
	virtual	void		AttachedToWindow();
	virtual	void		MessageReceived(BMessage *msg);
	virtual	void		Draw(BRect updateRect);
	
	virtual	void		SetBarColor(rgb_color color);
	virtual	void		SetBarHeight(float height);
	virtual	void		SetText(const char *str);
	virtual	void		SetTrailingText(const char *str);
	virtual	void		SetMaxValue(float max);
	
	virtual	void		Update(	float delta,
								const char *main_text = NULL,
								const char *trailing_text = NULL);
	virtual	void		Reset(	const char *label = NULL,
								const char *trailing_label = NULL);

	float		CurrentValue() const;
	float		MaxValue() const;
	rgb_color	BarColor() const;
	float		BarHeight() const;
	const char	*Text() const;
	const char	*TrailingText() const;
	const char	*Label() const;
	const char	*TrailingLabel() const;

	virtual	void		MouseDown(BPoint pt);
	virtual	void		MouseUp(BPoint pt);
	virtual	void		WindowActivated(bool state);
	virtual	void		MouseMoved(BPoint pt, uint32 code, const BMessage *msg);
	virtual	void		DetachedFromWindow();
	virtual	void		FrameMoved(BPoint new_position);
	virtual	void		FrameResized(float new_width, float new_height);
	
	virtual void		ResizeToPreferred();
	virtual void		GetPreferredSize(float *width, float *height);
	virtual void		MakeFocus(bool state = true);
	virtual void		AllAttached();
	virtual void		AllDetached();
private:

	FStatusBar	&operator=(const FStatusBar &);
};


#endif
