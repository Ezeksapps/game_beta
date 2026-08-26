#pragma once

#include <cstdint>

/* Definitions of UI-related functions. Nuklear functions cannot be in here, as that would create a multiple definition */

typedef struct UiVertex {
    float   pos[2];
    float   uv[2];
    uint8_t col[4];
} UiVertex;

typedef struct FontAtlasData {
    const void* image;
    int w, h;
} FontAtlasData;

void initUi();
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
