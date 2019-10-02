#ifndef __Globals_h__
#define __Globals_h__

#define LOG(format, ...) log(__FILE__, __LINE__, format, __VA_ARGS__);

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec( dllexport )
#else
#define ENGINE_API __declspec( dllimport )
#endif

ENGINE_API void log(const char file[], int line, const char* format, ...);

enum update_status
{
	UPDATE_CONTINUE = 1,
	UPDATE_STOP,
	UPDATE_ERROR
};

// Deletes a buffer
#define RELEASE( x ) \
    {\
       if( x != nullptr )\
       {\
         delete x;\
	     x = nullptr;\
       }\
    }

// Deletes an array of buffers
#define RELEASE_ARRAY( x ) \
	{\
       if( x != nullptr )\
       {\
           delete[] x;\
	       x = nullptr;\
		 }\
	 }

#define MAX(x,y) ((x>y)?x:y)
#define MIN(x,y) ((x<y)?x:y)

// Configuration -----------
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define TITLE "FRACTAL ENGINE"
#define VERSION_BUILD "0.3" // this constant is the version build for the fractal engine.
#define MAX_BONE_NAME_LENGTH 1024

#define MAX_BONE_NAME_LENGTH 1024

#endif  __Globals_h__