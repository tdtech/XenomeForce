#pragma once

#include "..\common\collection.h"

class IGUIElement
{
	friend class CGUIManager;

public:
	virtual ~IGUIElement() {}

	virtual void handleEvents() = 0;
	virtual void draw() = 0;

private:
	int mId;
};

class CGUIManager
{
public:
	static void init();
	static void release();
	static inline CGUIManager* instance() { return mInstance; }

	void addGUIElement(IGUIElement* element);
	void removeGUIElement(IGUIElement* element);
	void handleEvents();
	void drawGUIElements();
	void releaseGUIElements();

private:
	CGUIManager();
	~CGUIManager();

	CCollection<IGUIElement> mGUIElements;

	static CGUIManager*      mInstance;
};