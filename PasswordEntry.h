#ifndef PASSWORD_ENTRY_H
#define PASSWORD_ENTRY_H

#include <string>
#include <ctime>
#include <vector>
#include <wx/wx.h>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// ── Built-in palette (cycles for user-defined categories) ────────────────────
static const wxColour CATEGORY_PALETTE[] = {
    wxColour(220,  53,  69),   // red
    wxColour(0, 123, 255),   // blue
    wxColour(40, 167,  69),   // green
    wxColour(220, 160,   0),   // amber
    wxColour(111,  66, 193),   // purple
    wxColour(23, 162, 184),   // teal
    wxColour(253, 126,  20),   // orange
    wxColour(102, 102, 102),   // gray
};
static const int PALETTE_SIZE = 8;

// ── Category: name + display colour ─────────────────────────────────────────
struct Category {
    std::string name;
    wxColour    colour;

    Category() : name(""), colour(wxColour(150, 150, 150)) {}
    Category(const std::string& n, const wxColour& c) : name(n), colour(c) {}

    json toJson() const {
        return json{
            {"name",   name},
            {"colourR", colour.Red()},
            {"colourG", colour.Green()},
            {"colourB", colour.Blue()}
        };
    }
    static Category fromJson(const json& j) {
        return Category(
            j.value("name", ""),
            wxColour(j.value("colourR", 150),
                j.value("colourG", 150),
                j.value("colourB", 150))
        );
    }
};

// ── Password entry ────────────────────────────────────────────────────────────
struct PasswordEntry {
    std::string id;
    std::string service;
    std::string username;
    std::string password;
    std::string url;
    std::string notes;
    std::time_t created;
    std::time_t modified;
    std::time_t lastChanged;
    std::string category;   // free-form name; "" means no category

    PasswordEntry()
        : created(std::time(nullptr)),
        modified(std::time(nullptr)),
        lastChanged(std::time(nullptr)),
        category("") {
    }

    json toJson() const;
    static PasswordEntry fromJson(const json& j);
};

#endif