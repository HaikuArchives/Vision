#
SHELL := /bin/sh
#
ROOT = ../..
srcdirrel = shell/gnome
SRCS = \
	Application.cpp \
	Alert.cpp \
	Bitmap.cpp \
	Box.cpp \
	Button.cpp \
	CheckBox.cpp \
	Color.cpp \
	Control.cpp \
	DataIO.cpp \
	Directory.cpp \
	Entry.cpp \
	File.cpp \
	FilePanel.cpp \
	FindDirectory.cpp \
	Font.cpp \
	Handler.cpp \
	Invoker.cpp \
	Locker.cpp \
	Looper.cpp \
	Menu.cpp \
	Message.cpp \
	Messenger.cpp \
	Node.cpp \
	NodeMonitor.cpp \
	Path.cpp \
	PopUpMenu.cpp \
	Rect.cpp \
	Screen.cpp \
	ScrollBar.cpp \
	ScrollView.cpp \
	StringView.cpp \
	SymLink.cpp \
	TextControl.cpp \
	TextView.cpp \
	Thread.cpp \
	View.cpp \
	Window.cpp \
	GnomeUtils.cpp \
	GnomeClipboardHandler.cpp \
	IOChannel.cpp \
	ObjectGlue.cpp\
	PThread.cpp \
	image.cpp \
#


include $(ROOT)/makedefs

ARCHIVE_RESULT = $(odir)gnome.lib

.PHONY: all
all: odirs $(odir)depsfile $(ARCHIVE_RESULT)

include $(ROOT)/makerules

