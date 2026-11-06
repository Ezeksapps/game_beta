/* UI related function implementations and core Nuklear implementation */

#include "ui.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_IMPLEMENTATION
#include "nuklear.h"

/* Macros */

#define NUMELEMS(x)  (sizeof(x) / sizeof((x)[0])) // https://stackoverflow.com/questions/37538
#define MAX_DIALOG_CHARS 255

/* UI Stack */

#define MAX_UI_DEPTH 16

int stackPtr = 0;
UICreatorFunc stack[MAX_UI_DEPTH] = {0}; // UI Stack

void pushToStack(UICreatorFunc func) {
    if (stackPtr != MAX_UI_DEPTH) {
        stack[stackPtr] = func;
        ++stackPtr;
    }
}

void popFromStack() {
    if (stackPtr != 0) {
        --stackPtr;
        stack[stackPtr] = NULL;
    }
}

// return top-most func in stack
UICreatorFunc getCurrentUICreatorFunc() { return (stackPtr > 0) ? stack[stackPtr - 1] : NULL; }

/* Nuklear globals */

struct nk_context ctx;

int numFontTextures;
struct nk_font* font;

struct nk_buffer cmds;
struct nk_draw_null_texture texNull;

struct nk_font_atlas atlas;
struct FontAtlasData fontAtlasData;

typedef enum WndType {
    WND_TYPE_NONE,
    WND_TYPE_DIALOG,
    WND_TYPE_MENU
} WndType;

typedef struct MenuState {
    int selectedUIElem;
    int numItems;
    // these values are persisted so that they don't need to be constantly re-calculated
    int numRows;
    int widgetWidth;
    int initX;
    int initY;
} MenuState;

// NOTE: Multiple windows can exist simultaneously, this refers to the window considered to be 'focused'
enum   WndType   g_curWndType = WND_TYPE_NONE;

// UI elem type states
char g_curDialogText[MAX_DIALOG_CHARS] = "";
struct MenuState g_curMenuState = {0};

// global 'mouse' state (this mouse being a cursor controlled by keyboard input)
typedef struct vec2 {int x; int y;} vec2;
vec2 g_mousePos = (vec2) {.x = 0, .y = 0};

// constants (px)

#define WND_PADDING 4
#define WND_BORDER 3
#define MENU_ELEM_HEIGHT 20

/* Convenience function
 * makes setting the colour shorter and avoids needing to manually specify the exact index
 *
 * For example: ctx.style.text.color = setColor(stylesheetData, &currentIndex);
 * Instead of: ctx.style.text.color = nk_rgba(stylesheetData[0], stylesheetData[1], stylesheetData[2], stylesheetData[3]);
 *
 * This also means it's easier if needing to re-order or add settings to the JSON,
 * as all index numbers do not need to be manually re-typed to match the new positions.
 */
struct nk_color setColor(const uint8_t* data, int* currentIndex) {
    struct nk_color color = {
        .r = data[*currentIndex],
        .g = data[*currentIndex + 1],
        .b = data[*currentIndex + 2],
        .a = data[*currentIndex + 3]
    };
    *currentIndex += 4;

    return color;
}

void moveCursorToItem(int index) {

    int column = index / g_curMenuState.numRows;
    int row = index % g_curMenuState.numRows;

    int newX = g_curMenuState.initX + column * (g_curMenuState.widgetWidth + WND_PADDING);
    int newY = g_curMenuState.initY + row * (MENU_ELEM_HEIGHT + WND_PADDING);

    nk_input_begin(&ctx);
    nk_input_motion(&ctx, newX, newY);
    nk_input_end(&ctx);
}

// TODO: For multi-column menus, UI_RIGHT and UI_DOWN should be added
// TODO: Up and down don't handle wrapping to the next/prev column

void up() {
    if (g_curMenuState.selectedUIElem > 0) --g_curMenuState.selectedUIElem;
    else g_curMenuState.selectedUIElem = g_curMenuState.numItems - 1;
    moveCursorToItem(g_curMenuState.selectedUIElem);
}

void down() {
    if (g_curMenuState.selectedUIElem + 1 < g_curMenuState.numItems) ++g_curMenuState.selectedUIElem;
    else g_curMenuState.selectedUIElem = 0;
    moveCursorToItem(g_curMenuState.selectedUIElem);
}

void initUi(void* _this, int32_t (*loadSkinTex)(void* _this, const char* skinFilepath)) {

    {
        nk_buffer_init_default(&cmds);

        // load font(s) and initialise font atlas image data

        const char* font_path = "assets/ui/font.ttf"; // PMD font, TODO: make sure to fill in missing glyphs (could probably just dump off EUR rom)
        nk_font_atlas_init_default(&atlas);
        nk_font_atlas_begin(&atlas);
        font = nk_font_atlas_add_from_file(&atlas, font_path, 13.0f, NULL);

        int w, h;
        const void* image = nk_font_atlas_bake(&atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
        fontAtlasData = (FontAtlasData) {
            .image = image,
            .w = w, .h = h
        };

        // do not end atlas yet, nk_font_atlas_end will be called via endFontAtlas() in UI renderer
        // NOTE: That function MUST be called for this to work properly
    }

    nk_init_default(&ctx, &font->handle); // init context with font
    {

        // UI Skin is not a priority, add back later, basic styling is fine for now


        /* --- colour styles and padding --- */

        // NOTE: missing property, chart, combo button, contextual button, progress bar, slider, selectable and tree (Not currently needed in UI)

        const uint8_t* stylesheetData = loadStylesheet();

        // counter for current index in data array to access, auto-incremented by 4 whenever setColor() is called
        // the order of the setColor invokations must match the order of the settings in the JSON
        int currentIndex = 0;

        /* default text colour */
        ctx.style.text.color = setColor(stylesheetData, &currentIndex);

        /* window */
        ctx.style.window.background = setColor(stylesheetData, &currentIndex);
        ctx.style.window.border_color = setColor(stylesheetData, &currentIndex);
        ctx.style.window.padding = nk_vec2(4,4); // px
        ctx.style.window.border = 3; // px

        /* window header omitted */

        /* checkbox toggle */
        {
            struct nk_style_toggle* toggle;
            toggle = &ctx.style.checkbox;
            toggle->text_normal     = setColor(stylesheetData, &currentIndex);
            toggle->text_hover      = setColor(stylesheetData, &currentIndex);
            toggle->text_active     = setColor(stylesheetData, &currentIndex);
        }

        /* option toggle */
        {
            struct nk_style_toggle* toggle;
            toggle = &ctx.style.option;
            toggle->text_normal     = setColor(stylesheetData, &currentIndex);
            toggle->text_hover      = setColor(stylesheetData, &currentIndex);
            toggle->text_active     = setColor(stylesheetData, &currentIndex);;

        }

        /* default button */

        ctx.style.button.border_color = setColor(stylesheetData, &currentIndex);
        ctx.style.button.text_background = setColor(stylesheetData, &currentIndex);
        ctx.style.button.text_normal = setColor(stylesheetData, &currentIndex);
        ctx.style.button.text_hover = setColor(stylesheetData, &currentIndex);
        ctx.style.button.text_active = setColor(stylesheetData, &currentIndex);

        /* edit */
        ctx.style.edit.normal = nk_style_item_color(setColor(stylesheetData, &currentIndex));
        ctx.style.edit.hover = nk_style_item_color(setColor(stylesheetData, &currentIndex));
        ctx.style.edit.active = nk_style_item_color(setColor(stylesheetData, &currentIndex));

        ctx.style.edit.border_color = setColor(stylesheetData, &currentIndex);
        ctx.style.edit.cursor_normal = setColor(stylesheetData, &currentIndex);
        ctx.style.edit.cursor_hover = setColor(stylesheetData, &currentIndex);
        ctx.style.edit.cursor_text_normal = setColor(stylesheetData, &currentIndex);
        ctx.style.edit.cursor_text_hover = setColor(stylesheetData, &currentIndex);;
        ctx.style.edit.text_normal = setColor(stylesheetData, &currentIndex);
        ctx.style.edit.text_hover = setColor(stylesheetData, &currentIndex);
        ctx.style.edit.text_active = setColor(stylesheetData, &currentIndex);
        // no selected
        ctx.style.edit.border = 1; // px
        ctx.style.edit.rounding = 0; // round corners are a disease

    }
}

void cleanupUi() {
    nk_font_atlas_clear(&atlas);
    nk_buffer_free(&cmds);
    nk_free(&ctx);
}

FontAtlasData* getFontAtlasData() { return &fontAtlasData; }

void endFontAtlas(void* texView) {
    nk_font_atlas_end(&atlas, nk_handle_ptr(texView), &texNull);
}

// adapted from DiligentSamples Nuklear Demo (NkDiligent.cpp)
int convertVertices(void* vertexBufferMem, void* indexBufferMem) {

    struct nk_convert_config config;

    NK_STORAGE const struct nk_draw_vertex_layout_element vertexLayout[] =
    {
        {NK_VERTEX_POSITION, NK_FORMAT_FLOAT,    NK_OFFSETOF(struct UiVertex, pos)},
        {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT,    NK_OFFSETOF(struct UiVertex, uv)},
        {NK_VERTEX_COLOR,    NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct UiVertex, col)},
        {NK_VERTEX_LAYOUT_END}
    };

    memset(&config, 0, sizeof(config));
    config.vertex_layout        = vertexLayout;
    config.vertex_size          = sizeof(struct UiVertex);
    config.vertex_alignment     = NK_ALIGNOF(struct UiVertex);
    config.global_alpha         = 1.0f;
    config.shape_AA             = NK_ANTI_ALIASING_ON;
    config.line_AA              = NK_ANTI_ALIASING_ON;
    config.circle_segment_count = 11;
    config.curve_segment_count  = 11;
    config.arc_segment_count    = 11;
    config.tex_null             = texNull;

    // setup buffers to load vertices and elements
    struct nk_buffer vertexBuffer, indexBuffer;
    nk_buffer_init_fixed(&vertexBuffer, vertexBufferMem, (size_t)512 * 1024);
    nk_buffer_init_fixed(&indexBuffer, indexBufferMem, (size_t)128 * 1024);
    return nk_convert(&ctx, &cmds, &vertexBuffer, &indexBuffer, &config);
}

// adapted from DiligentSamples Nuklear Demo (NkDiligent.cpp)
void drawCmdsForEach(void* _this, void (*execDrawCmd)(void* _this, struct nk_rect clipRect, void* texPtr, unsigned int elemCount)) {

    const struct nk_draw_command* cmd = nullptr;

    nk_draw_foreach(cmd, &ctx, &cmds) {
        if (!cmd->elem_count) continue;
        execDrawCmd(_this, cmd->clip_rect, cmd->texture.ptr, cmd->elem_count);
    }
    nk_clear(&ctx);
}



const char* genWindowID(char* buffer, int* counter) {
    ++*counter;
    sprintf(buffer, "wnd%d", *counter);
    return buffer;
}

// nk_begin creates window w/ no header (title is only for in-code ID), nk_begin_titles created window with header
// For now, I'm not going to use header, might be done in future, UI is currently in early stage


/* --- BASE UI ELEM TYPES --- */

// https://stackoverflow.com/questions/37538
void createMenu(struct nk_rect dimensions, const int numColumns, const UICreatorFunc creatorFunc, int* UIElemCounter,
                const int numLabels, const char* labels[], const UICreatorFunc itemFuncs[]) {

    // if the caller hasn't pushed itself to the stack, then the menu state needs to be initialised
    if (getCurrentUICreatorFunc() != creatorFunc) {
        g_curWndType = WND_TYPE_MENU;
        // initialise menu state
        g_curMenuState = (MenuState) {
            .selectedUIElem = 0,
            .numItems = numLabels,
            // https://stackoverflow.com/a/26122628
            // https://stackoverflow.com/questions/64008512
            .numRows = (numLabels + numColumns - 1) / numColumns,
            // -------
            .widgetWidth = (int)(dimensions.w - WND_PADDING - WND_BORDER) * (1 / numColumns),
            .initX = (int)dimensions.x + WND_BORDER + WND_PADDING,
            .initY = (int)dimensions.y + WND_BORDER + WND_PADDING
        };
        // set initial cursor position
        nk_input_begin(&ctx);
        nk_input_motion(&ctx, g_curMenuState.initX, g_curMenuState.initY);
        nk_input_end(&ctx);
    }

    char windowID[16] = {};
    if (nk_begin(&ctx, genWindowID(windowID, UIElemCounter), dimensions,
        NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {

        nk_layout_row_dynamic(&ctx, 20, numColumns);

        for (int row = 0; row < g_curMenuState.numRows; ++row) { // for each row
            for (int col = 0; col < numColumns; ++col) { // for each column in this row
                int index = col * g_curMenuState.numRows + row;

                if (index >= g_curMenuState.numItems) continue;
                if (nk_button_label(&ctx, labels[index])) {
                    printf("Button pressed"); // on click function body not executing?
                    itemFuncs[index]();
                }
            }
        }
    }
    nk_end(&ctx);
}

void setDialogText(char* text) { strcpy(g_curDialogText, text); }

// TODO: adapt text wrap
void dialogBox(struct nk_rect dimensions, int* UIElemCounter) { // 270 * 480

    g_curWndType = WND_TYPE_DIALOG;

    char windowID[16] = {};
    if (nk_begin(&ctx, genWindowID(windowID, UIElemCounter), dimensions,
        NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {

        nk_layout_row_dynamic(&ctx, 20, 1);
        nk_text(&ctx, g_curDialogText, strlen(g_curDialogText), NK_TEXT_ALIGN_LEFT);
    }
    nk_end(&ctx);
}

// END ---


/* TODO: IMPLEMENT ALL NECESSARY UI MENUS/VIEWS */
/* TODO: Create screen 'stack' for screen history, so pressing ESC (equiv B) will properly return to previous screen */

void pokemonView() {}
void bagView() {
    int counter = 0;
    char* testText = "This is the bag view";
    setDialogText(testText);
    dialogBox(nk_rect(0, 210, 480, 60), &counter);

    if (getCurrentUICreatorFunc() != bagView) {
        pushToStack(bagView);
    }
}
void profileView() {}
void optionsMenu() {}
void multiplayerView() {}
void saveMenu() {}
void mapView() {}

// in-game pause menu
void pauseMenu() {

    const char* itemLabels[] = {"Pokemon", "Bag", "Profile", "Options", "Multiplayer", "Save", "Map"};
    const UICreatorFunc itemFuncs[] = {pokemonView, bagView, profileView, optionsMenu, multiplayerView, saveMenu, mapView};

    int counter = 0;
    createMenu(nk_rect(0, 0, 140, 240), 1, pauseMenu, &counter, NUMELEMS(itemLabels), itemLabels, itemFuncs);

    if (getCurrentUICreatorFunc() != pauseMenu) {
        pushToStack(pauseMenu);
    }
}

void progressUI() {
    if (g_curWndType == WND_TYPE_NONE) {
        initiateDialog();
    }
    else if (g_curWndType == WND_TYPE_DIALOG) {
        advanceDialog();
    }
    else {
        nk_input_begin(&ctx);
        nk_input_button(&ctx, NK_BUTTON_LEFT, g_mousePos.x, g_mousePos.y, 1);
        nk_input_end(&ctx);
    }
    // else if menu should run creator func for next UI elem
}

void entityDialogBox() {
    int counter = 0;
    dialogBox(nk_rect(0, 210, 480, 60), &counter);

    if (getCurrentUICreatorFunc() != entityDialogBox) {
        pushToStack(entityDialogBox);
    }
}

// run is disabled for now, it will serve a purpose later
void battleDashboard() {

    int counter = 0;

    char* text = "What will TEST ENTITY do?"; // TEMP
    setDialogText(text); // battle manager should call this function
    dialogBox(nk_rect(0, 210, 240, 60), &counter);

    const char* labels[] = {"Fight", "Pokemon", "Bag", "Run"};
    const UICreatorFunc itemFuncs[] = {NULL, NULL, NULL, NULL};

    createMenu(nk_rect(240, 210, 240, 60), 2, battleDashboard, &counter, NUMELEMS(labels), labels, itemFuncs);

    if (getCurrentUICreatorFunc() != battleDashboard) pushToStack(battleDashboard);
}
