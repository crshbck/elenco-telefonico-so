#include <std/mem.pat>

fn get_record_color(u128 record_start) {
    str palette[11] = {
        "3B82F6", // Blu
        "F97316", // Arancione
        "8B5CF6", // Viola
        "06B6D4", // Ciano
        "EC4899", // Rosa
        "EAB308", // Giallo
        "6366F1", // Indaco
        "14B8A6", // Turchese
        "D946EF", // Fucsia
        "84CC16", // Lime
        "F43F5E"  // Corallo
    };
    
    return palette[record_start % 11];
};

struct Entry {
    u8 type [[color(std::mem::read_unsigned($, 1) == 0x01 ? "22C55E" : "EF4444")]];
    u8 name_length;
    u8 number_length;
    
    char name[name_length] [[color(get_record_color($ - 3))]];
    char number[number_length] [[color(get_record_color($ - 3 - name_length))]];
};

Entry entries[while(!std::mem::eof())] @ 0x00;

// thanks gemini