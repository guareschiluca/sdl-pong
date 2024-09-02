#include "PathUtils.h"

#pragma region C++ Includes
#include <sstream>
#include <algorithm>
#pragma endregion

#pragma region Constant Parameters
#define PATH_SEPARATOR_WINDOWS '\\'
#define PATH_SEPARATOR_UNIX '/'
#ifdef _WIN32
#define PATH_SEPARATOR PATH_SEPARATOR_WINDOWS
#define UNSUPPORTED_PATH_SEPARATOR PATH_SEPARATOR_UNIX
#else
#define PATH_SEPARATOR PATH_SEPARATOR_UNIX
#define UNSUPPORTED_PATH_SEPARATOR PATH_SEPARATOR_WINDOWS
#endif

#define EXTENSION_SEPARATOR '.'

#ifndef __EMSCRIPTEN__
#define IMAGE_EXTENSION "png"
#define FONT_EXTENSION "ttf"
#define MUSIC_EXTENSION "mp3"
#define CHUNK_EXTENSION "wav"
#else
#define IMAGE_EXTENSION "png"
#define FONT_EXTENSION "ttf"
#define MUSIC_EXTENSION "mp3"
#define CHUNK_EXTENSION "wav"
#endif
#pragma endregion

void PathUtils::Conform(string & path)
{
	//	Replace all unsupported path separator characters with the supported ones
	replace(path.begin(), path.end(), UNSUPPORTED_PATH_SEPARATOR, PATH_SEPARATOR);
}

const string PathUtils::Combine(const vector<string> & parts)
{
	/*
	 * Take each part of path and combine them
	 * using the correct path separator
	 */
	ostringstream fullPath;
	for(string part : parts)
	{
		//	Make sure passed paths have the correct format
		Conform(part);

		//	After the first segment, alwasy add a valid path separator
		string partialPath = fullPath.str();
		if(
			!partialPath.empty() &&
			partialPath.back() != PATH_SEPARATOR
			)
			fullPath << PATH_SEPARATOR;

		//	Enqueue the path part to the full path
		fullPath << part;
	}
	return fullPath.str();
}

string PathUtils::AddImageExtension(const string & imageFileName)
{
	return AddExtension(imageFileName, IMAGE_EXTENSION);
}

string PathUtils::AddFontExtension(const string & fontFileName)
{
	return AddExtension(fontFileName, FONT_EXTENSION);
}

string PathUtils::AddMusicExtension(const string & musicFileName)
{
	return AddExtension(musicFileName, MUSIC_EXTENSION);
}

string PathUtils::AddChunkExtension(const string & chunkFileName)
{
	return AddExtension(chunkFileName, CHUNK_EXTENSION);
}

string PathUtils::AddExtension(const string & fileName, const string & extension)
{
	ostringstream composite;

	//	First add the file name (or path), without extension
	composite << fileName;

	//	If the file name doesn't end with . and the extension doesn't start with ., add the .
	if(
		fileName.back() != EXTENSION_SEPARATOR &&
		extension.front() != EXTENSION_SEPARATOR
	)
		composite << EXTENSION_SEPARATOR;

	//	Finally, add the extension
	composite << extension;

	return composite.str();
}
