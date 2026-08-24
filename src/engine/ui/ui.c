/* UI related function implementations and core Nuklear implementation */

#include "ui.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_IMPLEMENTATION
#include "nuklear.h"

struct nk_font* font;
struct nk_context ctx;

struct media {
    int id;
    struct nk_image menu;
    struct nk_image check;
    struct nk_image check_cursor;
    struct nk_image option;
    struct nk_image option_cursor;
    struct nk_image header;
    struct nk_image window;
    struct nk_image scrollbar_inc_button;
    struct nk_image scrollbar_inc_button_hover;
    struct nk_image scrollbar_dec_button;
    struct nk_image scrollbar_dec_button_hover;
    struct nk_image button;
    struct nk_image button_hover;
    struct nk_image button_active;
    struct nk_image slider;
    struct nk_image slider_hover;
    struct nk_image slider_active;
};

struct nk_buffer cmds;
struct nk_draw_null_texture texNull;

int numFontTextures;

struct nk_font_atlas atlas;
struct media media;


void initUi() {

    {
        const void* image; int w, h;
        const char* font_path = "assets/ui/font.tff"; // using PMD font for now, but make sure to fill in missing glyphs
        nk_font_atlas_init_default(&atlas);
        nk_font_atlas_begin(&atlas);
        font = nk_font_atlas_add_from_file(&atlas, font_path, 13.0f, NULL);
        image = nk_font_atlas_bake(&atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
        // REPLACE w/ Renderer func to gen texture image from atlas
        // device_upload_atlas(&device, image, w, h);
        // nk_font_atlas_end(&atlas, nk_handle_id((int)device.font_tex), &texNull);

    }

    nk_init_default(&ctx, &font->handle);
    {
       // media.id = //image_load("assets/ui/skin.png"); << SKIN WIP
       //  media.check = nk_subimage_id(media.id, 512,512, nk_rect(464,32,15,15));
    }

}

// nk_begin creates window w/ no header (title is only for in-code ID), nk_begin_titles created window with header

/* TODO: IMPLEMENT ALL NECESSARY UI MENUS/VIEWS */
/* TODO: Create screen 'stack' for screen history, so pressing ESC (equiv B) will properly return to previous screen */


void bagView() {}
void profileView() {}
void optionsMenu() {}
void multiplayerView() {}
void saveMenu() {}
void mapView() {}


// in-game pause menu
void pauseMenu() {
    if (nk_begin(&ctx, "Pause Menu", nk_rect(0, 0, 100, 240),
        NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {

        // Nuklear only has rows as a UI container, each row has a specified number of colums
        // a column container with specified rows doesn't exist

        /* Layout of pause menu
         * -=-=-=-=-=-=-=-=-=-=-=-
         *
         * Pokemon
         * Bag
         * Profile
         * Options
         * Multiplayer
         * Save
         * Map
         */

        nk_layout_row_static(&ctx, 20, 100, 1);
        if (nk_button_label(&ctx, "Pokemon")) {}

        nk_layout_row_static(&ctx, 20, 100, 1);
        if (nk_button_label(&ctx, "Bag")) {/*handle event*/}

        nk_layout_row_static(&ctx, 20, 100, 1);
        if (nk_button_label(&ctx, "Profile")) {/*handle event*/}

        nk_layout_row_static(&ctx, 20, 100, 1);
        if (nk_button_label(&ctx, "Options")) {/*handle event*/}

        nk_layout_row_static(&ctx, 20, 100, 1);
        if (nk_button_label(&ctx, "Multiplyr")) {/*handle event*/}

        nk_layout_row_static(&ctx, 20, 100, 1);
        if (nk_button_label(&ctx, "Save")) {/*handle event*/}

        nk_layout_row_static(&ctx, 20, 100, 1);
        if (nk_button_label(&ctx, "Map")) {/*handle event*/}

        // Press ESC to exit

    }
}

