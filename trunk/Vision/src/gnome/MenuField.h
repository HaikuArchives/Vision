#ifndef MENU_FIELD_H__
#define MENU_FIELD_H__

#include <PlatformDefines.h>
#include <Menu.h>
#include <View.h>

class FMenuBar;

/*----------------------------------------------------------------*/
/*----- FMenuField class -----------------------------------------*/

class FMenuField : public FView
{
public:
	FMenuField(BRect frame, const char *name, const char *label,
									BMenu *menu,
									uint32 resize = B_FOLLOW_LEFT|B_FOLLOW_TOP,
									uint32 flags = B_WILL_DRAW | B_NAVIGABLE); 
						FMenuField(	BRect frame,
									const char *name,
									const char *label,
									BMenu *menu,
									bool fixed_size,
									uint32 resize = B_FOLLOW_LEFT|B_FOLLOW_TOP,
									uint32 flags = B_WILL_DRAW | B_NAVIGABLE); 
	virtual					~FMenuField();

	virtual	void			Draw(BRect update);
	virtual	void			AttachedToWindow();
	virtual	void			AllAttached();
	virtual	void			MouseDown(BPoint where);
	virtual	void			KeyDown(const char *bytes, int32 numBytes);
	virtual	void			MakeFocus(bool state);
	virtual void			MessageReceived(BMessage *msg);
	virtual void			WindowActivated(bool state);
	virtual	void			MouseUp(BPoint pt);
	virtual	void			MouseMoved(BPoint pt, uint32 code, const BMessage *msg);
	virtual	void			DetachedFromWindow();
	virtual	void			AllDetached();
	virtual	void			FrameMoved(BPoint new_position);
	virtual	void			FrameResized(float new_width, float new_height);

	BMenu			*Menu() const;
	BMenuBar		*MenuBar() const;
	BMenuItem		*MenuItem() const;

	virtual	void			SetLabel(const char *label);
	const char		*Label() const;
		
	virtual void			SetEnabled(bool on);
	bool			IsEnabled() const;

	//virtual	void			SetAlignment(alignment label);
	//alignment		Alignment() const;
	virtual	void			SetDivider(float dividing_line);
	float			Divider() const;

	void			ShowPopUpMarker();
	void			HidePopUpMarker();

	virtual BHandler		*ResolveSpecifier(BMessage *msg,
										int32 index,
										BMessage *specifier,
										int32 form,
										const char *property);
	virtual status_t		GetSupportedSuites(BMessage *data);

	virtual void			ResizeToPreferred();
	virtual void			GetPreferredSize(float *width, float *height);


	virtual status_t		Perform(perform_code d, void *arg);

private:
	FMenuField		&operator=(const FMenuField &);
};


#endif
