//******************************************************************************
//
//
//	Copyright 2001, Be Incorporated
//
//******************************************************************************


#include "ColorSelector.h"
#include "ColorSwatch.h"
#include "Vision.h"

#include <ColorControl.h>
#include <LayoutBuilder.h>
#include <MenuBar.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <Screen.h>
#include <StringView.h>
#include <Window.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ColorSelector"

namespace ColorControlPrivate {

static inline bool
CompareColors(rgb_color left, rgb_color right)
{
	return !memcmp(&left, &right, sizeof(rgb_color));
}

enum {
	CMD_CHOOSE_UI_COLOR = 'cuic',
	CMD_SET_UI_COLOR = 'suic'
};

class ColorMenuItem : public BMenuItem {
public:
	ColorMenuItem(const char* label, BMessage* msg, rgb_color color, rgb_color initial_color)
		: BMenuItem(label, msg)
	{
		fColor = color;
		fInitialColor = initial_color;
	}

	void ResetColors(rgb_color col, rgb_color initial)
	{
		if (!CompareColors(fColor, col) || !CompareColors(fInitialColor, initial)) {
			fColor = col;
			fInitialColor = initial;
			BMenu* parent = Menu();
			if (parent)
				parent->Invalidate(Frame());
		}
	}

	void SetColor(rgb_color col)
	{
		if (!CompareColors(fColor, col)) {
			fColor = col;
			BMenu* parent = Menu();
			if (parent)
				parent->Invalidate(Frame());
		}
	}

	rgb_color Color() const { return fColor; }

	rgb_color InitialColor() const { return fInitialColor; }

	virtual void DrawContent()
	{
		BRect b = Frame();
		BMenu* parent = Menu();
		BPoint loc = parent->PenLocation();

		enum {
			W_CHAR = 0,
			A_CHAR = 1,
			OPEN_CHAR = 2,
			CLOSE_CHAR = 3,
			SPACE_CHAR = 4,
			NUM_CHARS = 5
		};
		float escapements[NUM_CHARS];
		BFont font;
		parent->GetFont(&font);
		font.GetEscapements("WA() ", NUM_CHARS, escapements);
		for (int32 i = 0; i < NUM_CHARS; i++) {
			escapements[i] *= font.Size();
		}

		const float blockWidth = escapements[W_CHAR] + escapements[A_CHAR];

		const rgb_color old_col = parent->HighColor();
		font_height fh;

		const bool showInitial = !CompareColors(fInitialColor, fColor);

		b.InsetBy(1, 1);
		b.bottom -= 1;
		b.left = loc.x;

		if (showInitial) {
			parent->GetFontHeight(&fh);
			parent->DrawString("(", BPoint(b.left, loc.y + fh.ascent));
		}
		b.left += escapements[OPEN_CHAR];
		b.InsetBy(2, 2);
		b.right = b.left + escapements[W_CHAR];
		if (showInitial) {
			parent->SetHighColor(fInitialColor);
			parent->FillRect(b);
		}
		b.InsetBy(-1, -1);
		if (showInitial) {
			parent->SetHighColor(old_col);
			parent->StrokeRect(b);
		}
		b.InsetBy(-1, -1);
		if (showInitial) {
			parent->DrawString(")", BPoint(b.right + 1, loc.y + fh.ascent));
		}
		b.right += escapements[CLOSE_CHAR] + 1;

		b.left = b.right + escapements[SPACE_CHAR];
		b.right = b.left + blockWidth;

		parent->SetHighColor(fColor);
		parent->FillRect(b);
		parent->SetHighColor(old_col);
		b.InsetBy(-1, -1);
		parent->StrokeRect(b);

		parent->MovePenTo(b.right + escapements[SPACE_CHAR] * 2 + 2, loc.y);

		BMenuItem::DrawContent();
	}

	virtual void GetContentSize(float* w, float* h)
	{
		BMenuItem::GetContentSize(w, h);
		*w += Menu()->StringWidth("(W) WA  ") + 4 + 2;
	}

private:
	rgb_color fColor, fInitialColor;
};
}  // namespace ColorControlPrivate
using namespace ColorControlPrivate;

static void
populate_colors(BMenu* dest, const BMessage& src, const BMessage& names, const BMessage* initial)
{
	type_code type;
	int32 n;
	ssize_t size;
	rgb_color* col;
	const char* name;
	src.GetInfo("color", &type, &n);
	for (int32 j = 0; j < n; j++) {
		names.FindString("color", j, &name);
		if (src.FindData("color", B_RGB_COLOR_TYPE, j, (const void**)(&col), &size) == B_OK) {
			// See if this field already exists in the menu.
			const int32 k = dest->CountItems();
			int32 i;
			bool found = false;
			for (i = 0; i < k; i++) {
				BMenuItem* it = dest->ItemAt(i);
				const char* field;
				if (it && it->Message() && it->Message()->FindString("field", &field) == B_OK
					&& strcmp(field, name) == 0) {
					ColorMenuItem* ci = dynamic_cast<ColorMenuItem*>(it);
					if (ci)
						ci->SetColor(*col);
					found = true;
					break;
				}
			}

			if (found)
				continue;

			// This color doesn't currently exist; add it in.
			BMessage* msg = new BMessage(CMD_CHOOSE_UI_COLOR);
			msg->AddString("field", name);
			msg->AddInt32("index", j);
			const char* label = name;

			// Alphabetical order.
			for (i = 0; i < n; i++) {
				BMenuItem* it = dest->ItemAt(i);
				if (it && strcmp(it->Label(), label) > 0)
					break;
			}
			rgb_color* init_col;
			if (!initial
				|| initial->FindData("color", B_RGB_COLOR_TYPE, j, (const void**)&init_col, &size)
					   != B_OK)
				*init_col = *col;

			ColorMenuItem* ci = new ColorMenuItem(label, msg, *col, *init_col);
			dest->AddItem(ci);
		}
	}
}
enum {
	FRAME = 5,
	SPACE = 5
};

ColorSelector::ColorSelector(BRect frame, const char* name, const char* label,
	const BMessage& colors, const BMessage& names, BMessage* model, uint32 resizeMask, uint32 flags)
	: BControl(name, label, model, flags),
	  fNames(names),
	  fInitColors(colors),
	  fColors(colors),
	  fSizeValid(false)
{
	fColorMenu = new BPopUpMenu(B_TRANSLATE("Colors"));
	populate_colors(fColorMenu, fColors, fNames, &fInitColors);

	fColorField = new BMenuField(
		"Color", B_TRANSLATE("Color:"), fColorMenu, B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);

	fColorField->SetFont(be_bold_font);

	fColorPalette = new BColorControl(
		BPoint(0, 0), B_CELLS_32x8, 8, "Palette", new BMessage(CMD_SET_UI_COLOR), true);
	rgb_color* color(NULL);
	ssize_t size(0);
	fColors.FindData("color", B_RGB_COLOR_TYPE, 0, (const void**)(&color), &size);
	swatch = new ColorSwatch("swatch", *color);

	// Original
	/*
		BLayoutBuilder::Group<>(this, B_VERTICAL)
			.SetInsets(B_USE_WINDOW_SPACING)
				.Add(fColorField)
				.AddGroup(B_HORIZONTAL)
					.Add(fColorPalette)
					.AddGroup(B_VERTICAL)
						.Add(swatch)
						.AddGlue()
					.End()
				.End()
			.End();
	*/

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING)
		.Add(fColorField)
		.Add(swatch)
		.Add(fColorPalette)
		.AddGlue()
		.End();
}

ColorSelector::~ColorSelector() {}

void
ColorSelector::ExtractColors(BMessage* dest, const BMessage& src)
{
	type_code type;
	int32 n;

	src.GetInfo("color", &type, &n);
	for (int32 i = 0; i < n; i++) {
		const void* data;
		ssize_t size;
		if (src.FindData("color", B_RGB_COLOR_TYPE, i, &data, &size) == B_OK
			&& size == sizeof(rgb_color))
			dest->AddData("color", B_RGB_COLOR_TYPE, data, size, true, i == 0 ? n : 1);
	}
}

void
ColorSelector::AttachedToWindow()
{
	BControl::AttachedToWindow();

	BMessenger me(this);
	fColorPalette->SetTarget(me);
	fColorMenu->SetTargetForItems(me);
	BMenuItem* it = fColorMenu->ItemAt(0);
	if (it) {
		it->SetMarked(true);
		dynamic_cast<BInvoker*>(it)->Invoke();
	}
}

void
ColorSelector::AllAttached()
{
	BControl::AllAttached();
	LayoutViews(true);
}

void
ColorSelector::LayoutViews(bool really)
{
	/*	float mw, mh, cw, ch;
		fColorField->SetDivider(fColorField->StringWidth(fColorField->Label()) + 5);
		fColorField->GetPreferredSize(&mw, &mh);
		fColorPalette->GetPreferredSize(&cw, &ch);

		if (really) {
			BRect b(Bounds());
			if (Window()) Window()->BeginViewTransaction();
			fColorField->MoveTo(b.left, b.top);
			fColorField->ResizeTo(b.Width() + 1, mh);
			fColorPalette->MoveTo(b.left, b.top + mh + SPACE);
			fColorPalette->ResizeTo(cw, ch);
			swatch->MoveTo(fColorPalette->Bounds().Width() + 5, b.top + mh + SPACE);
			swatch->ResizeTo(ch, ch);
			if (Window()) Window()->EndViewTransaction();
		}

		fPrefWidth = (mw > cw ? mw : cw) + 5 + swatch->Bounds().Width();
		fPrefHeight = mh + SPACE + ch;
		fSizeValid = true; */
}

void
ColorSelector::MessageReceived(BMessage* msg)
{
	if (msg->WasDropped()) {
		rgb_color* color;
		ssize_t size;
		if (msg->FindData("RGBColor", B_RGB_COLOR_TYPE, (const void**)&color, &size) == B_OK) {
			if (fColorPalette) {
				fColorPalette->SetValue(*color);
				swatch->SetColor(*color);
				fColorPalette->Invoke();
			}
		} else {
			BControl::MessageReceived(msg);
		}
		return;
	}

	switch (msg->what) {
		case CMD_CHOOSE_UI_COLOR:
		{
			const char* field;
			int32 index(0);
			ssize_t size;
			if (msg->FindString("field", &field) == B_OK) {
				msg->FindInt32("index", &index);
				fCurrentField = field;
				rgb_color* color;
				if (fColors.FindData(
						"color", B_RGB_COLOR_TYPE, index, (const void**)(&color), &size)
					== B_OK) {
					fColorPalette->SetValue(*color);
					swatch->SetColor(*color);
				}
			}
		} break;

		case CMD_SET_UI_COLOR:
		{
			if (Message()) {
				BMessage upd(*Message());
				BMessage colors;
				int32 index;
				rgb_color c = fColorPalette->ValueAsColor();
				swatch->SetColor(c);
				ColorMenuItem* item((ColorMenuItem*)(fColorMenu->FindItem(fCurrentField.String())));
				if (item) {
					BMessage* curMsg(item->Message());
					curMsg->FindInt32("index", &index);
				}
				c.alpha = 255;
				colors.AddData("color", B_RGB_COLOR_TYPE, &c, sizeof(rgb_color));
				colors.AddInt32("index", index);
				vision_app->SetColor(index, c);
				Update(colors);
				Invoke(&upd);
			}
		} break;

		default:
			BControl::MessageReceived(msg);
			break;
	}
}

void
ColorSelector::FrameResized(float w, float h)
{
	// LayoutViews(true);
	BView::FrameResized(w, h);
}

void
ColorSelector::GetPreferredSize(float* width, float* height)
{
	BView::GetPreferredSize(width, height);
	//	if (!fSizeValid) LayoutViews(false);
	//	*width = fPrefWidth;
	//	*height = fPrefHeight;
}

/*------------------------------------------------------------*/

bool
ColorSelector::IsDirty() const
{
	bool dirty = false;
	type_code type;
	int32 n;
	fInitColors.GetInfo("color", &type, &n);
	for (int32 i = 0; i < n; i++) {
		const void* data1;
		const void* data2;
		ssize_t size1;
		ssize_t size2;
		if (fInitColors.FindData("color", B_RGB_COLOR_TYPE, i, &data1, &size1) == B_OK
			&& (fColors.FindData("color", B_RGB_COLOR_TYPE, i, &data2, &size2) != B_OK
				|| size1 != size2 || memcmp(data1, data2, size1) != 0)) {
			dirty = true;
			break;
		}
	}

	return dirty;
}

void
ColorSelector::SetTo(const BMessage& colors)
{
	fInitColors.MakeEmpty();
	ExtractColors(&fInitColors, colors);
	fColors = fInitColors;

	fColorMenu->RemoveItems(0, fColorMenu->CountItems(), true);
	populate_colors(fColorMenu, fColors, fNames, &fInitColors);

	if (Window())
		ColorSelector::AttachedToWindow();
}

void
ColorSelector::Update(const BMessage& changes)
{
	int32 index(0);
	ssize_t size(0);
	rgb_color* color;
	changes.FindInt32("index", &index);
	changes.FindData("color", B_RGB_COLOR_TYPE, (const void**)(&color), &size);
	fColors.ReplaceData("color", B_RGB_COLOR_TYPE, index, color, sizeof(rgb_color));
	populate_colors(fColorMenu, fColors, fNames, &fInitColors);
	ColorMenuItem* item((ColorMenuItem*)(fColorMenu->FindItem(fCurrentField.String())));
	if (item) {
		BMessage* curMsg(item->Message());
		curMsg->FindInt32("index", &index);
	}

	if (fColors.FindData("color", B_RGB_COLOR_TYPE, index, (const void**)(&color), &size) == B_OK) {
		fColorPalette->SetValue(*color);
		swatch->SetColor(*color);
	}
}

void
ColorSelector::Revert()
{
	// duplicate the message since SetTo destroys fInitColors
	BMessage OriginalColors = fInitColors;
	SetTo(OriginalColors);
	rgb_color* color;
	ssize_t size(0);

	for (int32 i = 0; i < MAX_COLORS; i++)
		if (fInitColors.FindData("color", B_RGB_COLOR_TYPE, i, (const void**)(&color), &size)
			== B_OK)
			if (!CompareColors(*color, vision_app->GetColor(i)))
				vision_app->SetColor(i, *color);
}

const BMessage&
ColorSelector::CurrentColors() const
{
	return fColors;
}

const BMessage&
ColorSelector::InitialColors() const
{
	return fInitColors;
}

/*------------------------------------------------------------*/
/*------------------------------------------------------------*/
