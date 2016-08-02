#ifndef BDN_config_H_
#define BDN_config_H_

#include <cstdint>
#include <climits>
#include <cstddef>

#if WCHAR_MAX <= 0xffff
	#define BDN_WCHAR_SIZE 2

#elif WCHAR_MAX<=0xffffffff
	#define BDN_WCHAR_SIZE 4

#else
	#error Unsupported wchar size

#endif


// BDN_PLATFORM_WEB cannot be detected automatically. It must be defined
// via compiler commandline options.
#if !defined(BDN_PLATFORM_DETECTED) && defined(BDN_PLATFORM_WEB)
    #undef BDN_PLATFORM_WEB
    #define BDN_PLATFORM_WEB 1
    #define BDN_PLATFORM_DETECTED 1

#endif


#if !defined(BDN_PLATFORM_DETECTED) && defined(__cplusplus_cli)
	#define BDN_PLATFORM_DOTNET 1
	#define BDN_PLATFORM_DETECTED 1
#endif


#if !defined(BDN_PLATFORM_DETECTED) && (defined(WIN32) || defined(_WINDOWS) || defined(WINAPI_FAMILY) )

#if defined(WINAPI_FAMILY)

	#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		#define BDN_PLATFORM_WIN32 1
	#else
		#define BDN_PLATFORM_WINRT 1
	#endif

#else
	#define BDN_PLATFORM_WIN32 1

#endif

	#define BDN_PLATFORM_WINDOWS 1
	#define BDN_PLATFORM_DETECTED 1
	
#endif

#if !defined(BDN_PLATFORM_DETECTED) && defined(__APPLE__)

	#include <TargetConditionals.h>

	#if TARGET_OS_IPHONE
		#define BDN_PLATFORM_IOS 1
	#else
		#define BDN_PLATFORM_OSX 1
	#endif

	#define BDN_PLATFORM_DETECTED 1

#endif


#if !defined(BDN_PLATFORM_DETECTED) && (defined(__ANDROID__) || defined(ANDROID))
	#define BDN_PLATFORM_ANDROID 1
	#define BDN_PLATFORM_DETECTED 1
#endif


#if !defined(BDN_PLATFORM_DETECTED) && defined(__linux__)
	#define BDN_PLATFORM_LINUX 1
	#define BDN_PLATFORM_DETECTED 1

#endif

#if !defined(BDN_PLATFORM_DETECTED) && defined(_POSIX_VERSION)
	// some generic Posix/Unix system
	#define BDN_PLATFORM_POSIX 1
	#define BDN_PLATFORM_DETECTED 1
#endif


#if !defined(BDN_PLATFORM_DETECTED)
	#error Unable to determine target system type

#endif


#if !defined(BDN_PLATFORM_POSIX)
	#if BDN_PLATFORM_IOS || BDN_PLATFORM_OSX || BDN_PLATFORM_LINUX || BDN_PLATFORM_ANDROID
		#define BDN_PLATFORM_POSIX 1
    #endif
#endif


#if BDN_PLATFORM_WEB

	#if __EMSCRIPTEN_major__<1 || (__EMSCRIPTEN_major__==1 && __EMSCRIPTEN_minor__ < 36 )
		// Older emscripten versions had compiler bugs that cause major troubles with some integral
		// parts of the library (exception handling not working correctly in some cases, etc.).
		// So we cannot use those.
		#pragma error("Emscripten 1.36.0 or higher required.")
	#endif


    // no support for threads (yet). There is actually an implementation from Emscripten
    // but as of the time of this writing it only works if the Javascript code is executed
    // with Firefox nightly. It depends on an extension of the web worker specification, which
    // is not standardized yet. So we disable threading support.
    #define BDN_HAVE_THREADS 0

#else
    // all other platforms support multi-threading
    #define BDN_HAVE_THREADS 1

#endif


// The C++11 standard requires that local static variables
// must be initialized in a thread-safe manner (so that they are
// constructed exactly once). This feature is also called "magic statics"
// by some.

#if defined(_MSC_VER) && _MSC_VER<1700
// Visual Studio versions before 2015 did not support the thread
// safe static requirement.
#define BDN_COMPILER_STATIC_CONSTRUCTION_THREAD_SAFE 0

#else
// since the standard demands this, we assume that statics are safe by default.
#define BDN_COMPILER_STATIC_CONSTRUCTION_THREAD_SAFE 1
#endif


#if BDN_PLATFORM_ANDROID
// the c++_shared and c++_static standard libraries do not implement the full locale
// api and the android docs state that support for wchar_t is "limited".
// So we hard-code UTF-8 encoding - it is the only encoding that android supports anyway.
#define BDN_OVERRIDE_LOCALE_ENCODING_UTF8 1
#endif


#endif

