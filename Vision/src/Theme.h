#ifndef THEME_H_
#define THEME_H_

#include <OS.h>
#include <GraphicsDefs.h>
#include <List.h>

class BView;

class Theme
{
	char					*name;
	rgb_color			*fores;
	rgb_color			*backs;
	BFont					*fonts;

	int16					fore_count;
	int16					back_count;
	int16					font_count;

	BList					list;
	sem_id				sid;

	public:

	static int16		TimestampFore;
	static int16		TimestampBack;
	static int16		TimestampFont;
	static int16		TimespaceFore;
	static int16		TimespaceBack;
	static int16		TimespaceFont;
	static int16		NormalFore;
	static int16		NormalBack;
	static int16		NormalFont;
	static int16		SelectionBack;

							Theme (
								const char *,
								int16,
								int16,
								int16);
							~Theme (void);

	const char			*Name (void) const
							{ return name; }

	void					ReadLock (void);
	void					ReadUnlock (void);
	void					WriteLock (void);
	void					WriteUnlock (void);

	int16					CountForegrounds (void) const;
	int16					CountBackgrounds (void) const;
	int16					CountFonts (void) const;

	const rgb_color	ForegroundAt (int16) const;
	const rgb_color	BackgroundAt (int16) const;
	const BFont			&FontAt (int16) const;

	bool					SetForeground (int16, const rgb_color);
	bool					SetForeground (int16 w, uchar r, uchar g, uchar b, uchar a = 255)
							{ rgb_color color = {r, g, b, a}; return SetForeground (w, color); }
	bool					SetBackground (int16, const rgb_color);
	bool					SetBackground (int16 w, uchar r, uchar g, uchar b, uchar a = 255)
							{ rgb_color color = {r, g, b, a}; return SetBackground (w, color); }
	bool					SetFont (int16, const BFont &);

	void					AddView (BView *);
	void					RemoveView (BView *);
};

const uint32 M_FOREGROUND_CHANGE			= 'FGch';
const uint32 M_BACKGROUND_CHANGE			= 'BGch';
const uint32 M_FONT_CHANGE					= 'FNch';

#endif
