#pragma once
#ifdef FACE_RECOGNIZER_IMPL
#define FR_API __declspec(dllexport)
#else
#define FR_API __declspec(dllimport)
#endif
