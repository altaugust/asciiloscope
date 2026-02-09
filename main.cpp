#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <memory>
#include <array>
#include <algorithm>
#include <cstring>
#include <ncurses.h>
#include <pulse/simple.h>
#include <pulse/error.h>

// --- Technical Config ---
const int SAMPLE_RATE = 44100;
const int CHANNELS = 2;
const int SAMPLES_PER_FRAME = 735; // ~60 FPS

// --- Visual Config ---
const char CHAR_BRIGHT = '#';  
const char CHAR_DIM    = '.';  
const char CHAR_OFF    = ' ';  

enum ViewMode { XY_MODE, WAVEFORM };

// Global State
std::vector<std::string> screen_buffer;
bool transparent_bg = false;
int current_theme = 1; 

// --- Color Constants ---
#define THEME_GREEN 1
#define THEME_AMBER 2
#define THEME_CYAN  3
#define THEME_RED   4
#define THEME_WHITE 5
#define COLOR_UI_MUTE 6

// --- System Helpers ---
std::string exec_cmd(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) return "";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) result += buffer.data();
    if (!result.empty() && result.back() == '\n') result.pop_back();
    return result;
}

std::string get_default_monitor_source() {
    std::string sink = exec_cmd("pactl get-default-sink");
    return sink.empty() ? "" : sink + ".monitor";
}

// --- Graphics Engine ---
void init_theme_colors() {
    // Use -1 for transparent background if enabled, otherwise BLACK
    int bg = transparent_bg ? -1 : COLOR_BLACK;
    
    init_pair(THEME_GREEN, COLOR_GREEN, bg);
    init_pair(THEME_AMBER, COLOR_YELLOW, bg);
    init_pair(THEME_CYAN,  COLOR_CYAN, bg);
    init_pair(THEME_RED,   COLOR_RED, bg);
    init_pair(THEME_WHITE, COLOR_WHITE, bg);
    
    init_pair(COLOR_UI_MUTE, COLOR_RED, bg);
}

void resize_buffer() {
    screen_buffer.clear();
    for (int i = 0; i < LINES; i++) screen_buffer.push_back(std::string(COLS, CHAR_OFF));
}

// Bresenham line algorithm
void draw_line_buffer(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;
    while (true) {
        if (x1 >= 0 && x1 < COLS && y1 >= 0 && y1 < LINES) {
            screen_buffer[y1][x1] = CHAR_BRIGHT;
        }
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

// Fast decay for clean trace
void process_afterglow() {
    for (int y = 0; y < LINES; y++) {
        for (int x = 0; x < COLS; x++) {
            char& c = screen_buffer[y][x];
            if (c == CHAR_BRIGHT) c = CHAR_DIM;
            else if (c == CHAR_DIM) c = CHAR_OFF;
        }
    }
}

// --- Main ---
int main(int argc, char* argv[]) {
    // --- 0. CLI Arguments ---
    bool start_transparent = false;
    bool start_waveform = false;
    bool start_no_ui = false;
    int start_theme = THEME_GREEN;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            std::cout << "Usage: asciiloscope [OPTIONS]\n"
                      << "  --transparent, -t   Start with transparent background\n"
                      << "  --waveform, -w      Start in Waveform mode\n"
                      << "  --no-ui             Start with UI hidden\n"
                      << "  --color <name>      Start with color (green, amber, cyan, red, white)\n";
            return 0;
        }
        if (strcmp(argv[i], "--transparent") == 0 || strcmp(argv[i], "-t") == 0) start_transparent = true;
        if (strcmp(argv[i], "--waveform") == 0 || strcmp(argv[i], "-w") == 0) start_waveform = true;
        if (strcmp(argv[i], "--no-ui") == 0) start_no_ui = true;
        if (strcmp(argv[i], "--color") == 0 && i + 1 < argc) {
            std::string c = argv[++i];
            if (c == "amber") start_theme = THEME_AMBER;
            else if (c == "cyan") start_theme = THEME_CYAN;
            else if (c == "red") start_theme = THEME_RED;
            else if (c == "white") start_theme = THEME_WHITE;
        }
    }

    // --- 1. Audio Setup ---
    static const pa_sample_spec ss = { .format = PA_SAMPLE_S16LE, .rate = SAMPLE_RATE, .channels = CHANNELS };
    pa_buffer_attr ba;
    ba.maxlength = (uint32_t)-1; ba.tlength = (uint32_t)-1; ba.prebuf = (uint32_t)-1; ba.minreq = (uint32_t)-1;
    ba.fragsize = SAMPLES_PER_FRAME * CHANNELS * sizeof(int16_t);

    int error;
    std::string mon = get_default_monitor_source();
    pa_simple *s_sys = nullptr;
    
    if (!mon.empty()) {
        s_sys = pa_simple_new(NULL, "asciiloscope", PA_STREAM_RECORD, mon.c_str(), "Monitor", &ss, NULL, &ba, &error);
    }

    if (!s_sys) {
        std::cerr << "Error: PulseAudio Monitor source not found." << std::endl;
        return 1;
    }

    // --- 2. Ncurses Setup ---
    initscr(); 
    noecho(); 
    curs_set(0); 
    start_color(); 
    use_default_colors();
    
    // Apply initial CLI settings
    transparent_bg = start_transparent;
    current_theme = start_theme;
    init_theme_colors();

    nodelay(stdscr, TRUE);
    resize_buffer();

    std::vector<int16_t> buf_sys(SAMPLES_PER_FRAME * CHANNELS);
    
    // Program State
    bool active_audio = true;
    bool show_ui = false; 
    bool auto_gain = false; 
    float gain = 1.0f;
    ViewMode mode = start_waveform ? WAVEFORM : XY_MODE;

    while (true) {
        // Input Handling
        int ch = getch();
        if (ch == 'q') break;
        
        switch(ch) {
            case 'v': mode = (mode == XY_MODE) ? WAVEFORM : XY_MODE; break;
            case 'm': active_audio = !active_audio; break;
            case 'h': show_ui = !show_ui; break;
            case '+': gain += 0.1f; auto_gain = false; break; // Manual disables auto
            case '-': if(gain > 0.1f) gain -= 0.1f; auto_gain = false; break;
            case 'a': auto_gain = !auto_gain; break;
            case 't': 
                transparent_bg = !transparent_bg;
                init_theme_colors();
                break;
            case 'c':
                current_theme++;
                if (current_theme > 5) current_theme = 1;
                break;
        }

        if (LINES != (int)screen_buffer.size() || COLS != (int)screen_buffer[0].size()) resize_buffer();

        // Audio Read
        if (pa_simple_read(s_sys, buf_sys.data(), buf_sys.size() * sizeof(int16_t), &error) < 0) break;
        if (!active_audio) std::fill(buf_sys.begin(), buf_sys.end(), 0);

        // Auto-Gain Logic
        if (auto_gain && active_audio) {
            long max_val = 0;
            // Find peak in current frame
            for (auto s : buf_sys) {
                if (abs(s) > max_val) max_val = abs(s);
            }
            
            // Target: Peak should occupy ~80% of half-screen
            float target_gain = 1.0f;
            if (max_val > 500) {
                target_gain = 20000.0f / (float)max_val;
            }
            
            // Smooth transition (Lerp)
            gain = gain * 0.95f + target_gain * 0.05f;
            
            // Safety limits
            if (gain < 0.1f) gain = 0.1f;
            if (gain > 10.0f) gain = 10.0f;
        }

        // Visual Processing
        process_afterglow();

        int mid_y = LINES / 2, mid_x = COLS / 2;
        int scale_y = LINES / 2, scale_x = COLS / 2;

        if (mode == WAVEFORM) {
            int prev_x = 0, prev_y = mid_y;
            float step = (float)SAMPLES_PER_FRAME / (float)COLS;
            for (int x = 0; x < COLS; x++) {
                int idx = (int)(x * step) * CHANNELS;
                if (idx >= buf_sys.size()) break;
                
                int val = buf_sys[idx];
                int y = mid_y - (val * scale_y / 25000) * gain;
                
                if (x > 0) draw_line_buffer(prev_x, prev_y, x, y);
                prev_x = x; prev_y = y;
            }
        } else {
            // XY Mode
            int prev_x = mid_x, prev_y = mid_y;
            for (int i = 0; i < SAMPLES_PER_FRAME; i++) {
                int16_t left = buf_sys[i * 2];
                int16_t right = buf_sys[i * 2 + 1];

                int x = mid_x + (left * scale_x / 25000 * 1.5) * gain; 
                int y = mid_y - (right * scale_y / 25000) * gain;

                if (i > 0) draw_line_buffer(prev_x, prev_y, x, y);
                prev_x = x; prev_y = y;
            }
        }

        // Rendering
        erase();
        attron(COLOR_PAIR(current_theme)); 
        
        for (int y = 0; y < LINES; y++) mvaddnstr(y, 0, screen_buffer[y].c_str(), COLS);
        
        // UI Overlay
        if (show_ui) {
            attron(A_BOLD);
            mvprintw(0, 2, "[ ASCIILOSCOPE ]");
            
            if (auto_gain) mvprintw(1, 2, "GAIN: AUTO (%.1fx)", gain);
            else           mvprintw(1, 2, "GAIN: MANU (%.1fx)", gain);

            const char* theme_names[] = {"", "GREEN", "AMBER", "CYAN", "RED", "WHITE"};
            mvprintw(2, 2, "THEME: %s", theme_names[current_theme]);

            if (!active_audio) {
                attron(COLOR_PAIR(COLOR_UI_MUTE)); 
                mvprintw(0, 20, "[MUTED]"); 
                attron(COLOR_PAIR(current_theme));
            }

            mvprintw(LINES-2, 2, "CONTROLS:");
            mvprintw(LINES-1, 2, "'v':Mode 'c':Color 'a':AutoGain 'm':Mute 't':Transp '+/-':Gain 'h':Hide");
            attroff(A_BOLD);
        }
        
        attroff(COLOR_PAIR(current_theme));
        refresh();
    }

    endwin();
    if (s_sys) pa_simple_free(s_sys);
    return 0;
}