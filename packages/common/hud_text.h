#ifndef PC_HUD_TEXT_H
#define PC_HUD_TEXT_H

/* hud_text.h -- tiny immediate-mode GL_LINES stroke font, ported verbatim from
 * WEAKNIGHT_BEDROCK_RACERS' own copy (itself ported from GoblinFoxDragon/apps2/battlegrounds_gui's
 * own draw_char/draw_string, "ported from apps/lobby" per that file's own comment -- the same
 * SHANKPIT-lineage text renderer, not a new one, same "reference, don't reinvent" principle every
 * login screen in this monorepo follows).
 *
 * KNOWN, DELIBERATE PLACEHOLDER -- not a real font, same real gap
 * WEAKNIGHT_BEDROCK_RACERS' own copy already documents ("we are gonna want nice fonts" / "bring
 * in the nice font from parena editor" / "all of the text fields should be using parena editor
 * code") -- the real target is PARENA's own LINNEN widget framework
 * (PARENA/docs/NORTHSTAR_LINNEN.md), not this hand-rolled stroke glyph set. This file exists so
 * this repo's own login screen is real and testable NOW (draws real, readable text with zero new
 * dependencies -- no SDL_ttf/FreeType linked anywhere in this repo yet). Do not add new callers
 * expecting this to stay long-term.
 */

static void pc_draw_char(char c, float x, float y, float s) {
    if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A'); /* fold lowercase -- one glyph set, not two */
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    if (c >= '0' && c <= '9') {
        int seg_top = 0, seg_top_left = 0, seg_top_right = 0, seg_mid = 0;
        int seg_bot_left = 0, seg_bot_right = 0, seg_bot = 0;
        switch (c) {
        case '0': seg_top = seg_top_left = seg_top_right = seg_bot_left = seg_bot_right = seg_bot = 1; break;
        case '1': seg_top_right = seg_bot_right = 1; break;
        case '2': seg_top = seg_top_right = seg_mid = seg_bot_left = seg_bot = 1; break;
        case '3': seg_top = seg_top_right = seg_mid = seg_bot_right = seg_bot = 1; break;
        case '4': seg_top_left = seg_top_right = seg_mid = seg_bot_right = 1; break;
        case '5': seg_top = seg_top_left = seg_mid = seg_bot_right = seg_bot = 1; break;
        case '6': seg_top = seg_top_left = seg_mid = seg_bot_left = seg_bot_right = seg_bot = 1; break;
        case '7': seg_top = seg_top_right = seg_bot_right = 1; break;
        case '8': seg_top = seg_top_left = seg_top_right = seg_mid = seg_bot_left = seg_bot_right = seg_bot = 1; break;
        case '9': seg_top = seg_top_left = seg_top_right = seg_mid = seg_bot_right = seg_bot = 1; break;
        }
        if (seg_top) { glVertex2f(x, y + s); glVertex2f(x + s, y + s); }
        if (seg_top_left) { glVertex2f(x, y + s); glVertex2f(x, y + s / 2); }
        if (seg_top_right) { glVertex2f(x + s, y + s); glVertex2f(x + s, y + s / 2); }
        if (seg_mid) { glVertex2f(x, y + s / 2); glVertex2f(x + s, y + s / 2); }
        if (seg_bot_left) { glVertex2f(x, y + s / 2); glVertex2f(x, y); }
        if (seg_bot_right) { glVertex2f(x + s, y + s / 2); glVertex2f(x + s, y); }
        if (seg_bot) { glVertex2f(x, y); glVertex2f(x + s, y); }
    } else if (c == 'W') {
        glVertex2f(x, y + s); glVertex2f(x + s * 0.25f, y);
        glVertex2f(x + s * 0.25f, y); glVertex2f(x + s * 0.5f, y + s * 0.6f);
        glVertex2f(x + s * 0.5f, y + s * 0.6f); glVertex2f(x + s * 0.75f, y);
        glVertex2f(x + s * 0.75f, y); glVertex2f(x + s, y + s);
    } else if (c == 'I') {
        glVertex2f(x + s / 2, y); glVertex2f(x + s / 2, y + s);
    } else if (c == 'N') {
        glVertex2f(x, y); glVertex2f(x, y + s);
        glVertex2f(x, y + s); glVertex2f(x + s, y);
        glVertex2f(x + s, y); glVertex2f(x + s, y + s);
    } else if (c == 'L') {
        glVertex2f(x, y + s); glVertex2f(x, y);
        glVertex2f(x, y); glVertex2f(x + s, y);
    } else if (c == 'O') {
        glVertex2f(x, y); glVertex2f(x + s, y);
        glVertex2f(x + s, y); glVertex2f(x + s, y + s);
        glVertex2f(x + s, y + s); glVertex2f(x, y + s);
        glVertex2f(x, y + s); glVertex2f(x, y);
    } else if (c == 'S') {
        glVertex2f(x + s, y + s); glVertex2f(x, y + s);
        glVertex2f(x, y + s); glVertex2f(x, y + s / 2);
        glVertex2f(x, y + s / 2); glVertex2f(x + s, y + s / 2);
        glVertex2f(x + s, y + s / 2); glVertex2f(x + s, y);
        glVertex2f(x + s, y); glVertex2f(x, y);
    } else if (c == 'E') {
        glVertex2f(x + s, y); glVertex2f(x, y);
        glVertex2f(x, y); glVertex2f(x, y + s);
        glVertex2f(x, y + s); glVertex2f(x + s, y + s);
        glVertex2f(x, y + s / 2); glVertex2f(x + s * 0.8f, y + s / 2);
    } else if (c == 'U') {
        glVertex2f(x, y + s); glVertex2f(x, y);
        glVertex2f(x, y); glVertex2f(x + s, y);
        glVertex2f(x + s, y); glVertex2f(x + s, y + s);
    } else if (c == 'Y') {
        glVertex2f(x, y + s); glVertex2f(x + s / 2, y + s / 2);
        glVertex2f(x + s, y + s); glVertex2f(x + s / 2, y + s / 2);
        glVertex2f(x + s / 2, y + s / 2); glVertex2f(x + s / 2, y);
    } else if (c == 'H') {
        glVertex2f(x, y); glVertex2f(x, y + s);
        glVertex2f(x + s, y); glVertex2f(x + s, y + s);
        glVertex2f(x, y + s / 2); glVertex2f(x + s, y + s / 2);
    } else if (c == 'P') {
        glVertex2f(x, y); glVertex2f(x, y + s);
        glVertex2f(x, y + s); glVertex2f(x + s, y + s);
        glVertex2f(x + s, y + s); glVertex2f(x + s, y + s / 2);
        glVertex2f(x + s, y + s / 2); glVertex2f(x, y + s / 2);
    } else if (c == ' ') {
        /* nothing to stroke */
    } else if (c == 'A') {
        glVertex2f(x, y); glVertex2f(x + s / 2, y + s);
        glVertex2f(x + s / 2, y + s); glVertex2f(x + s, y);
        glVertex2f(x + s * 0.25f, y + s * 0.4f); glVertex2f(x + s * 0.75f, y + s * 0.4f);
    } else if (c == 'B') {
        glVertex2f(x, y); glVertex2f(x, y + s);
        glVertex2f(x, y + s); glVertex2f(x + s * 0.7f, y + s);
        glVertex2f(x + s * 0.7f, y + s); glVertex2f(x + s * 0.7f, y + s / 2);
        glVertex2f(x + s * 0.7f, y + s / 2); glVertex2f(x, y + s / 2);
        glVertex2f(x, y + s / 2); glVertex2f(x + s * 0.7f, y + s / 2);
        glVertex2f(x + s * 0.7f, y + s / 2); glVertex2f(x + s * 0.7f, y);
        glVertex2f(x + s * 0.7f, y); glVertex2f(x, y);
    } else if (c == 'C') {
        glVertex2f(x + s, y); glVertex2f(x, y);
        glVertex2f(x, y); glVertex2f(x, y + s);
        glVertex2f(x, y + s); glVertex2f(x + s, y + s);
    } else if (c == 'D') {
        glVertex2f(x, y); glVertex2f(x, y + s);
        glVertex2f(x, y + s); glVertex2f(x + s * 0.6f, y + s);
        glVertex2f(x + s * 0.6f, y + s); glVertex2f(x + s, y + s * 0.7f);
        glVertex2f(x + s, y + s * 0.7f); glVertex2f(x + s, y + s * 0.3f);
        glVertex2f(x + s, y + s * 0.3f); glVertex2f(x + s * 0.6f, y);
        glVertex2f(x + s * 0.6f, y); glVertex2f(x, y);
    } else if (c == 'F') {
        glVertex2f(x, y); glVertex2f(x, y + s);
        glVertex2f(x, y + s); glVertex2f(x + s, y + s);
        glVertex2f(x, y + s / 2); glVertex2f(x + s * 0.8f, y + s / 2);
    } else if (c == 'G') {
        glVertex2f(x + s, y); glVertex2f(x, y);
        glVertex2f(x, y); glVertex2f(x, y + s);
        glVertex2f(x, y + s); glVertex2f(x + s, y + s);
        glVertex2f(x + s, y + s); glVertex2f(x + s, y + s * 0.5f);
        glVertex2f(x + s * 0.5f, y + s * 0.5f); glVertex2f(x + s, y + s * 0.5f);
    } else if (c == 'J') {
        glVertex2f(x + s * 0.7f, y + s); glVertex2f(x + s * 0.7f, y + s * 0.2f);
        glVertex2f(x + s * 0.7f, y + s * 0.2f); glVertex2f(x + s * 0.3f, y);
        glVertex2f(x + s * 0.3f, y); glVertex2f(x, y + s * 0.2f);
    } else if (c == 'K') {
        glVertex2f(x, y); glVertex2f(x, y + s);
        glVertex2f(x, y + s / 2); glVertex2f(x + s, y + s);
        glVertex2f(x, y + s / 2); glVertex2f(x + s, y);
    } else if (c == 'M') {
        glVertex2f(x, y); glVertex2f(x, y + s);
        glVertex2f(x, y + s); glVertex2f(x + s / 2, y + s / 2);
        glVertex2f(x + s / 2, y + s / 2); glVertex2f(x + s, y + s);
        glVertex2f(x + s, y + s); glVertex2f(x + s, y);
    } else if (c == 'Q') {
        glVertex2f(x, y); glVertex2f(x + s, y);
        glVertex2f(x + s, y); glVertex2f(x + s, y + s);
        glVertex2f(x + s, y + s); glVertex2f(x, y + s);
        glVertex2f(x, y + s); glVertex2f(x, y);
        glVertex2f(x + s * 0.55f, y + s * 0.35f); glVertex2f(x + s, y);
    } else if (c == 'R') {
        glVertex2f(x, y); glVertex2f(x, y + s);
        glVertex2f(x, y + s); glVertex2f(x + s, y + s);
        glVertex2f(x + s, y + s); glVertex2f(x + s, y + s / 2);
        glVertex2f(x + s, y + s / 2); glVertex2f(x, y + s / 2);
        glVertex2f(x + s / 2, y + s / 2); glVertex2f(x + s, y);
    } else if (c == 'T') {
        glVertex2f(x, y + s); glVertex2f(x + s, y + s);
        glVertex2f(x + s / 2, y + s); glVertex2f(x + s / 2, y);
    } else if (c == 'V') {
        glVertex2f(x, y + s); glVertex2f(x + s / 2, y);
        glVertex2f(x + s / 2, y); glVertex2f(x + s, y + s);
    } else if (c == 'X') {
        glVertex2f(x, y); glVertex2f(x + s, y + s);
        glVertex2f(x, y + s); glVertex2f(x + s, y);
    } else if (c == 'Z') {
        glVertex2f(x, y + s); glVertex2f(x + s, y + s);
        glVertex2f(x + s, y + s); glVertex2f(x, y);
        glVertex2f(x, y); glVertex2f(x + s, y);
    } else if (c == '-') {
        glVertex2f(x, y + s / 2); glVertex2f(x + s, y + s / 2);
    } else if (c == '+') {
        glVertex2f(x, y + s / 2); glVertex2f(x + s, y + s / 2);
        glVertex2f(x + s / 2, y); glVertex2f(x + s / 2, y + s);
    } else if (c == '\'' || c == '"') {
        glVertex2f(x + s * 0.5f, y + s * 0.75f); glVertex2f(x + s * 0.5f, y + s);
    } else if (c == '.') {
        glVertex2f(x + s * 0.4f, y); glVertex2f(x + s * 0.6f, y);
    } else if (c == ',') {
        glVertex2f(x + s * 0.5f, y); glVertex2f(x + s * 0.3f, y - s * 0.25f);
    } else if (c == ':') {
        glVertex2f(x + s * 0.4f, y + s * 0.7f); glVertex2f(x + s * 0.6f, y + s * 0.7f);
        glVertex2f(x + s * 0.4f, y + s * 0.25f); glVertex2f(x + s * 0.6f, y + s * 0.25f);
    } else if (c == '!') {
        glVertex2f(x + s / 2, y + s); glVertex2f(x + s / 2, y + s * 0.3f);
        glVertex2f(x + s * 0.4f, y); glVertex2f(x + s * 0.6f, y);
    } else if (c == '(') {
        glVertex2f(x + s * 0.7f, y + s); glVertex2f(x + s * 0.3f, y + s * 0.5f);
        glVertex2f(x + s * 0.3f, y + s * 0.5f); glVertex2f(x + s * 0.7f, y);
    } else if (c == ')') {
        glVertex2f(x + s * 0.3f, y + s); glVertex2f(x + s * 0.7f, y + s * 0.5f);
        glVertex2f(x + s * 0.7f, y + s * 0.5f); glVertex2f(x + s * 0.3f, y);
    } else if (c == '%') {
        glVertex2f(x, y); glVertex2f(x + s, y + s);
        glVertex2f(x + s * 0.15f, y + s * 0.85f); glVertex2f(x + s * 0.15f, y + s * 0.7f);
        glVertex2f(x + s * 0.85f, y + s * 0.3f); glVertex2f(x + s * 0.85f, y + s * 0.15f);
    } else if (c == '?') {
        glVertex2f(x + s * 0.15f, y + s * 0.8f); glVertex2f(x + s * 0.5f, y + s);
        glVertex2f(x + s * 0.5f, y + s); glVertex2f(x + s * 0.85f, y + s * 0.8f);
        glVertex2f(x + s * 0.85f, y + s * 0.8f); glVertex2f(x + s * 0.5f, y + s * 0.55f);
        glVertex2f(x + s * 0.5f, y + s * 0.55f); glVertex2f(x + s * 0.5f, y + s * 0.35f);
        glVertex2f(x + s * 0.4f, y); glVertex2f(x + s * 0.6f, y);
    } else if (c == ';') {
        glVertex2f(x + s * 0.4f, y + s * 0.7f); glVertex2f(x + s * 0.6f, y + s * 0.7f);
        glVertex2f(x + s * 0.5f, y); glVertex2f(x + s * 0.3f, y - s * 0.25f);
    } else if (c == '/') {
        glVertex2f(x, y); glVertex2f(x + s, y + s);
    } else if (c == '@') {
        glVertex2f(x, y); glVertex2f(x + s, y);
        glVertex2f(x + s, y); glVertex2f(x + s, y + s);
        glVertex2f(x + s, y + s); glVertex2f(x, y + s);
        glVertex2f(x, y + s); glVertex2f(x, y + s * 0.3f);
        glVertex2f(x, y + s * 0.3f); glVertex2f(x + s * 0.7f, y + s * 0.3f);
        glVertex2f(x + s * 0.7f, y + s * 0.3f); glVertex2f(x + s * 0.7f, y + s * 0.65f);
    } else if (c == '_') {
        glVertex2f(x, y - s * 0.1f); glVertex2f(x + s, y - s * 0.1f);
    } else if (c == '*') {
        glVertex2f(x, y + s * 0.5f); glVertex2f(x + s, y + s * 0.5f);
        glVertex2f(x + s * 0.15f, y + s * 0.85f); glVertex2f(x + s * 0.85f, y + s * 0.15f);
        glVertex2f(x + s * 0.15f, y + s * 0.15f); glVertex2f(x + s * 0.85f, y + s * 0.85f);
    } else if (c == '&') {
        glVertex2f(x + s, y); glVertex2f(x + s * 0.3f, y + s * 0.55f);
        glVertex2f(x + s * 0.3f, y + s * 0.55f); glVertex2f(x + s * 0.65f, y + s * 0.8f);
        glVertex2f(x + s * 0.65f, y + s * 0.8f); glVertex2f(x + s * 0.4f, y + s);
        glVertex2f(x + s * 0.4f, y + s); glVertex2f(x + s * 0.1f, y + s * 0.75f);
        glVertex2f(x + s * 0.1f, y + s * 0.75f); glVertex2f(x + s * 0.75f, y);
    } else {
        glVertex2f(x, y); glVertex2f(x + s, y);
        glVertex2f(x + s, y); glVertex2f(x + s, y + s);
        glVertex2f(x + s, y + s); glVertex2f(x, y + s);
        glVertex2f(x, y + s); glVertex2f(x, y);
    }
    glEnd();
}

static void pc_draw_string(const char *str, float x, float y, float size) {
    while (*str) {
        pc_draw_char(*str, x, y, size);
        x += size * 1.2f;
        str++;
    }
}

#endif
