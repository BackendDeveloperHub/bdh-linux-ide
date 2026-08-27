#ifndef BDH_UI_H
#define BDH_UI_H

#include "bdh-db.h"

// டைனமிக் UI வேரியபிள்கள் (bdh-ui.c -ல் வரையறுக்கப்படும்)
extern int term_height;
extern float tree_width_ratio;

// Main State வேரியபிள்கள் (main.c -ல் இருந்து பகிரப்படுபவை)
extern int focus; 
extern int total_rows;
extern int total_cols;
extern int divider_col;

extern char db_query_buf[1024];
extern int db_query_len;
extern PGconn *db_conn;

extern char term_cmd_buf[1024];
extern int term_cmd_len;
extern char term_output[50][1024]; // 🔥 50 lines storage for terminal

// Editor Cursor Tracking (bdh-edit.c -ல் இருந்து பகிரப்படுபவை)
extern int editor_cx; 
extern int editor_cy;

// UI ஃபங்ஷன்
void draw_ui();

#endif
