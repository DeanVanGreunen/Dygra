#include "physfs.h"

#include <string>

#include "util.h"

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

std::string file_to_string(const char* filename)
{
	PHYSFS_file* file = PHYSFS_openRead( filename );

	int file_length = PHYSFS_fileLength(file);

	char* buffer;
	buffer = new char[ file_length ];
	int length_read = PHYSFS_read ( file, buffer, 1, file_length );

	std::string result;
	result.append(buffer, length_read);

	delete buffer;

	PHYSFS_close( file );

	return result;
}

// trim from start
std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}