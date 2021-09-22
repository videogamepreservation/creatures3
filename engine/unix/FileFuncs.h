// BenC 30Dec99

#ifndef FILEFUNCS_H
#define FILEFUNCS_H

#ifndef _WIN32
// win32 replacements
bool DeleteFile( const char* filename );
bool MoveFile( const char* src, const char* dest );
bool CopyFile( const char* src, const char* dest, bool overwrite );
void CreateDirectory(const char* name, void* ignored);
#endif


// returns true if file exists
bool FileExists( const char* filename );


#endif // FILEFUNCS_H

