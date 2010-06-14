/* 
 * The contents of this file are subject to the Mozilla Public 
 * License Version 1.1 (the "License"); you may not use this file 
 * except in compliance with the License. You may obtain a copy of 
 * the License at http://www.mozilla.org/MPL/ 
 * 
 * Software distributed under the License is distributed on an "AS 
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or 
 * implied. See the License for the specific language governing 
 * rights and limitations under the License. 
 * 
 * The Original Code is Vision. 
 * 
 * The Initial Developer of the Original Code is The Vision Team.
 * Portions created by The Vision Team are
 * Copyright (C) 1999-2010 The Vision Team.	All Rights
 * Reserved.
 * 
 * Contributor(s): Wade Majors <wade@ezri.org>
 *								 Rene Gollent
 *								 Todd Lair
 *								 Andrew Bazan
 *								 Jamie Wilkinson
 */

#include <String.h>

#include "VTextControl.h"
#include "VisionBase.h"
#include "HistoryList.h"

HistoryList::HistoryList () : 
	bufferFree (0),
	bufferPos (0)
{
}

void
HistoryList::PreviousBuffer (VTextControl *input)
{
	if (bufferPos)
	{
		if (input->TextView()->TextLength() > 0	&& 
		bufferFree < BACK_BUFFER_SIZE &&
		bufferPos == bufferFree)
			backBuffer[bufferFree++] = input->Text();

		--bufferPos;

		input->SetText (backBuffer[bufferPos].String());
		input->TextView()->Select (
			input->TextView()->TextLength(),
			input->TextView()->TextLength());
	}
}

void
HistoryList::NextBuffer (VTextControl *input)
{
	BString buffer;

	if (bufferPos + 1 < bufferFree)
	{
		++bufferPos;
		buffer = backBuffer[bufferPos].String();
	}
	else if (bufferFree > 0)
	{
		if (backBuffer[bufferFree-1] == input->Text())
		{
			buffer = "";
			++bufferPos;
		}
		else
			buffer = input->Text();
	}	
	input->SetText (buffer.String());
	input->TextView()->Select (
	input->TextView()->TextLength(),
	input->TextView()->TextLength());
}

BString
HistoryList::Submit (const char *buffer)
{
	int32 i(0);
	// All filled up
	if (bufferFree == BACK_BUFFER_SIZE)
	{
		for (i = 0; i < BACK_BUFFER_SIZE - 1; ++i)
			backBuffer[i] = backBuffer[i + 1];
	 
		bufferFree = BACK_BUFFER_SIZE - 1;
	}
	
	backBuffer[bufferFree] = buffer;

	BString cmd;
	
	for (i = 0; i < backBuffer[bufferFree].Length(); ++i)
	{
		if (backBuffer[bufferFree][i] == '\n')
			cmd += " ";
		else if (backBuffer[bufferFree][i] != '\r')
			cmd += backBuffer[bufferFree][i];
	}
	
	bufferPos = ++bufferFree;

	return cmd;
}

bool
HistoryList::HasHistory (void) const
{
	return bufferFree != 0;
}
