/*
Open Tracker License

Terms and Conditions

Copyright (c) 1991-2000, Be Incorporated. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice applies to all licensees
and shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF TITLE, MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
BE INCORPORATED BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Be Incorporated shall not be
used in advertising or otherwise to promote the sale, use or other dealings in
this Software without prior written authorization from Be Incorporated.

Tracker(TM), Be(R), BeOS(R), and BeIA(TM) are trademarks or registered trademarks
of Be Incorporated in the United States and other countries. Other brand product
names are registered trademarks or trademarks of their respective holders.
All rights reserved.
*/

/*******************************************************************************
/
/	File:			ColumnTypes.h
/
/   Description:    Experimental classes that implement particular column/field
/					data types for use in BColumnListView.
/
/	Copyright 2000+, Be Incorporated, All Rights Reserved
/
*******************************************************************************/


#ifndef _COLUMN_TYPES_H
#define _COLUMN_TYPES_H

#include "ColumnListView.h"
#include <String.h>
#include <Font.h>
#include <Bitmap.h>


//=====================================================================
// Common base-class: a column that draws a standard title at its top.

class BTitledColumn : public BColumn
{
	public:
							BTitledColumn		(const char *title,
												 float width,
												 float minWidth,
												 float maxWidth,
												 alignment align = B_ALIGN_LEFT);
		virtual void		DrawTitle			(BRect rect,
												 BView* parent);
		virtual void		GetColumnName		(BString* into) const;

		void				DrawString			(const char*,
												 BView*,
												 BRect);
		void				SetTitle			(const char* title);
		void				Title				(BString* forTitle) const; // sets the BString arg to be the title
		float				FontHeight			() const;

	private:
		float				fFontHeight;
		BString				fTitle;
};


//=====================================================================
// Field and column classes for strings.

class BStringField : public BField
{
	public:
									BStringField		(const char* string);

				void				SetString			(const char* string);
				const char*			String				() const;
				void				SetClippedString	(const char* string);
				const char*			ClippedString		();
				void				SetWidth			(float);
				float				Width				();
	
	private:
		float				fWidth;
		BString				fString;
		BString				fClippedString;
};


//--------------------------------------------------------------------

class BStringColumn : public BTitledColumn
{
	public:
							BStringColumn		(const char *title,
												 float width,
												 float maxWidth,
												 float minWidth,
												 uint32 truncate,
												 alignment align = B_ALIGN_LEFT);
		virtual void		DrawField			(BField* field,
												 BRect rect,
												 BView* parent);
		virtual int			CompareFields		(BField* field1,
												 BField* field2);
		virtual	bool		AcceptsField        (const BField* field) const;


	private:
		uint32				fTruncate;
};


//=====================================================================
// Field and column classes for dates.

class BDateField : public BField
{
	public:
							BDateField			(time_t* t);
		void				SetWidth			(float);
		float				Width				();
		void				SetClippedString	(const char*);
		const char*			ClippedString		();
		time_t				Seconds				();
		time_t				UnixTime			();

	private:	
		struct tm			fTime;
		time_t				fUnixTime;
		time_t				fSeconds;
		BString				fClippedString;
		float				fWidth;
};


//--------------------------------------------------------------------

class BDateColumn : public BTitledColumn
{
	public:
							BDateColumn			(const char* title,
												 float width,
												 float minWidth,
												 float maxWidth,
												 alignment align = B_ALIGN_LEFT);
		virtual void		DrawField			(BField* field,
												 BRect rect,
												 BView* parent);
		virtual int			CompareFields		(BField* field1,
												 BField* field2);
	private:
		BString				fTitle;
};


//=====================================================================
// Field and column classes for numeric sizes.

class BSizeField : public BField
{
	public:
							BSizeField			(off_t size);
		void				SetSize				(off_t);
		off_t				Size				();

	private:
		off_t				fSize;
};


//--------------------------------------------------------------------

class BSizeColumn : public BTitledColumn
{
	public:
							BSizeColumn			(const char* title,
												 float width,
												 float minWidth,
												 float maxWidth,
												 alignment align = B_ALIGN_LEFT);
		virtual void		DrawField			(BField* field,
												 BRect rect,
												 BView* parent);
		virtual int			CompareFields		(BField* field1,
												 BField* field2);
};


//=====================================================================
// Field and column classes for integers.

class BIntegerField : public BField
{
	public:
							BIntegerField		(int32 number);
		void				SetValue			(int32);
		int32				Value				();

	private:
		int32				fInteger;
};


//--------------------------------------------------------------------

class BIntegerColumn : public BTitledColumn
{
	public:
							BIntegerColumn		(const char* title,
												 float width,
												 float minWidth,
												 float maxWidth,
												 alignment align = B_ALIGN_LEFT);
		virtual void		DrawField			(BField* field,
												 BRect rect,
												 BView* parent);
		virtual int			CompareFields		(BField* field1,
												 BField* field2);
};


//=====================================================================
// Field and column classes for bitmaps

class BBitmapField : public BField
{
	public:
							BBitmapField		(BBitmap* bitmap);
		const BBitmap*		Bitmap				();
		void				SetBitmap			(BBitmap* bitmap);

	private:
		BBitmap*			fBitmap;
};


//--------------------------------------------------------------------

class BBitmapColumn : public BTitledColumn
{
	public:
							BBitmapColumn		(const char* title,
												 float width,
												 float minWidth,
												 float maxWidth,
												 alignment align = B_ALIGN_LEFT);
		virtual void		DrawField			(BField*field,
												 BRect rect,
												 BView* parent);
		virtual int			CompareFields		(BField* field1, BField* field2);
		virtual	bool		AcceptsField        (const BField* field) const;
};
	

//=====================================================================
// Column to display BIntegerField objects as a graph.

class GraphColumn : public BIntegerColumn
{
	public:
							GraphColumn			(const char* name,
												 float width,
												 float minWidth,
												 float maxWidth,
												 alignment align = B_ALIGN_LEFT);
		virtual void		DrawField			(BField*field,
												 BRect rect,
												 BView* parent);
};

#endif

