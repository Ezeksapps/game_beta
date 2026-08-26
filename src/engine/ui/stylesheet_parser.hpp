#pragma once

/* For easier editing of UI colours, the RGB values will be saved in an external stylesheet and then loaded
 * when the Renderer first initialises the UI (by calling initUI())
 * The stylesheet is written in JSON in a fairly self-explanatory format
 */

#include "json.hpp"
#include "../fileio.hpp"

#include <assert.h>

/* Because the Nuklear implementation is kept in pure C for separation purposes, I cannot use the JSON parser's
 * DOM traversal functions within the UI code to set all the options in there (unless I were to introduce a secondary
 * JSON parser like jansson/jsmn/<insert your favourite C JSON parser here>, which would be silly and take up unecessary space)
 * Instead, I'm pushing all the vec4/vec2 colour/padding values into a nicely-formatted array which is set to the exact size in bytes of
 * all the vec2/vec4 values in the stylesheet via the arraySize calculation (which is a constexpr, so this is being done once at compile-time)
 * The format of the stylesheet will never really change (unless slots for new values matching new UI elems are added, which is easy enough to do --
 * just add a new row to the arraySize calc),
 * so C-side, the code can access the indices corresponding to the values it needs to set. For example:
 * ctx.style.window.background = nk_rgba(stylesheetData[0], stylesheetData[1], stylesheetData[2], stylesheetData[3]);
 *
 * IMPORTANT NOTE: The indices in the array need to be arranged in the same order as the objects appear in the JSON stylesheet
 */

struct vec2 {int x; int y;};
struct vec4 {int r; int g; int b; int a;};

constexpr uint16_t arraySize = (
    sizeof(vec4)                      // default text colour
    + sizeof(vec4) * 7 + sizeof(vec2) // window
    + 0                               // header (UNUSED)
    + sizeof(vec4) * 3                // checkbox
    + sizeof(vec4) * 3                // option
    + sizeof(vec4) * 5                // button
    + sizeof(vec4) * 11               // edit
);

using json = nlohmann::json;

inline const int* loadStylesheet() { // inline is fine, this func is only used once anyway

    const char* stylesheet = readJsonAsset("assets/ui/styles.json");
    json root = json::parse(stylesheet);

    static int stylesheetData[arraySize];
    int index = 0; // counts current index, used to insert data at correct position in stylesheetData array

    for (const json& obj : root) { // for every object in JSON (holding styles for a specific UI elem)

        /* NOTE: Order must be consistent, you cannot change the order of the keys in an object without affecting the values RGBA/XY
         * that get assigned to the different UI components/actions, as the array order expected C-side is hard-coded!
         */

        if (obj.empty()) continue;

        for (const json& vec : obj) { // for every key whose value is either a colour (vec4) or padding data (vec2)
            // each vec is a fixed-size array
            int vecSize = vec.size();
            if (vecSize == 2) { // vec2
                for (int i = 0; i < 1; ++i) {
                    stylesheetData[index] = vec[i];
                    ++index;
                }
            }
            else if (vecSize == 4) { // vec4
                for (int i = 0; i < 3; ++i) {
                    stylesheetData[index] = vec[i];
                    ++index;
                }
            }
            else throw std::runtime_error("Cannot parse stylesheet, an key is assigned to an array of incorrect size (must be either vec2 or vec4)");
        }
    }
    // By this point, all data should be successfully copied to the array
    assert(index == sizeof(stylesheetData));

    return stylesheetData;
}
