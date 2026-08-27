#pragma once

#include <stdint.h>

/* Definitions of UI-related functions. Nuklear functions cannot be in here, as that would create a multiple definition */

// NOTE: functions taking a function pointer as a parameter require the _this parameter, since the non-static member functions assigned
// as their callbacks carry an implicit 'this' parameter

struct nk_rect;

typedef struct UiVertex {
    float   pos[2];
    float   uv[2];
    uint8_t col[4];
} UiVertex;

typedef struct FontAtlasData {
    const void* image;
    int w, h;
} FontAtlasData;

const uint8_t* loadStylesheet();

/* This C interface cannot use or include Diligent directly, but texture data is still required to be loaded for skinning purposes
 * initUi() takes a callback which should be a function C++-side that can generate and return the texture data as int
 */
void initUi(void* _this, int32_t (*loadSkinTex)(void* _this, const char* skinFilepath));
// Since this is a pure C interface, resource cleanup must be manually performed
void cleanupUi();

void endFontAtlas(void* texView);
// returns standard pointer to font atlas's image data created on init
FontAtlasData* getFontAtlasData();

int convertVertices(void* vertexBufferMem, void* indexBufferMem);
void drawCmdsForEach(void* _this, void (*execDrawCmd)(void* _this, struct nk_rect clipRect, void* texPtr, unsigned int elemCount));

/* DISPLAYS */

void dialogBox();
void optionBox();

/* UI ELEMENTS */

void pauseMenu();

void bagView();
void profileView();
void optionsMenu();
void multiplayerView();
void saveMenu();
void mapView();
