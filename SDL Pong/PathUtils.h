#pragma once

#pragma region C++ Includes
#include <string>
#include <vector>
#pragma endregion

using namespace std;

/*
 * A collection of utilities for cross-platform
 * handling of paths.
 * Here we didn't handle any error related to
 * unsupported characters or empty string, this
 * class assumes a decent usage from outside.
 */
class PathUtils
{
public:
	static void Conform(string & path);
	static const string Combine(const vector<string> & parts);
	static string AddImageExtension(const string &imageFileName);
	static string AddFontExtension(const string &fontFileName);
	static string AddMusicExtension(const string &musicFileName);
	static string AddChunkExtension(const string &chunkFileName);
private:
	static string AddExtension(const string &fileName, const string &extension);
};
