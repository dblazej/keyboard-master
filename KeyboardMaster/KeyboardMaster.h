#pragma once

#include "resource.h"
#include <stdlib.h>
#include <time.h>
#include <map>
#include <Commdlg.h>
#include <Windows.h>

#define FORCE_PAUSE			1
#define FORCE_RESUME		2
#define BACKGROUND_DEFAULT	0
#define BACKGROUND_BITMAP	1
#define BACKGROUND_TILE		2
#define BACKGROUND_STREACH	3
#define BACKGROUND_COLOR	0
#define DEFAULT_BKG_COLOR	(HBRUSH)(COLOR_INACTIVECAPTION + 1);