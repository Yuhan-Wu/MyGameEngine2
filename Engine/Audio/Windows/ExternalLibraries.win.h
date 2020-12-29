#pragma once

// External Libraries
//===================
#pragma comment( lib, "xaudio2.lib" )

#if _WIN32
#pragma comment( lib, "ole32.lib" )
#endif

// Windows Media Foundation
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")