#pragma once

#include <string>

#include "IRenderable.h"

using namespace std;

/*
 * A common interface for all object which
 * can display a text to a render target.
 * 
 * It's a specialization of IRenderable.
 */
class ITextRenderable : public IRenderable
{
public:
	virtual const string & GetFontPath() const = 0;
	virtual const string & GetText() const = 0;
	virtual const Uint8 & GetFontSize() const = 0;
};

