#pragma once

#include <cstdint>

/* Definitions of UI-related functions. Nuklear functions cannot be in here, as that would create a multiple definition */

struct UiVertex {
    float   pos[2];
    float   uv[2];
    uint8_t col[4];
};

void initUi();

void pauseMenu();

void bagView();
void profileView();
void optionsMenu();
void multiplayerView();
void saveMenu();
void mapView();

int convertVertices(void* vertexBufferMem, void* indexBufferMem);
void drawCmdsForEach(void* _this, void (*execDrawCmd)(void* _this, struct nk_rect clipRect, void* texPtr, unsigned int elemCount));
