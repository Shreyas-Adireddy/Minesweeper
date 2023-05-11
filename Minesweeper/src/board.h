//
// Created by shrey on 3/30/2023.
//

#ifndef MINESWEEPER_BOARD_H
#define MINESWEEPER_BOARD_H
#include "tile.h"
using namespace sf;
class Board {
public:
    std::vector<std::vector<Tile*>> board;
    std::map<std::string, Texture*>& textures;
    RenderWindow& window;
    std::random_device& rd;
    short status;
    Sprite face;
    Sprite debug;
    Sprite play;
    Sprite leaderboard;
    bool paused;
    bool show_leaderboard;
    bool debug_true;
    bool reset_game;
    int mines;
    int total_flags;
    int height,width;
    bool revealAll;
    bool just_won;
    Image screenshot;
    Texture curr_window_texture;

    Board(RenderWindow &window, std::map<std::string, Texture*>& textures,int width, int height, int size, int mines,std::random_device& rd);
    ~Board();
    void draw();
    void left_clicked(float x, float y);
    void right_clicked(float x, float y);
    void draw_face();
    void draw_play_button();
    void init_tiles();
    bool in_bounds(int di, int dj);
    void add_adj_tiles(Tile* tile,int i, int j);
    void generate_mines();
    void init_adj_mine_count();
    bool reveal_tile(Tile* tile);
    void debug_mode(Tile* curr= nullptr);
    void reset_debug_mode();
    void reveal_all_bombs();
    void reset_board();
    void dfs_click(int row,int col);
    void win_check();
    void pause_graphics();
    void reveal_testing();
    void reveal_testing2();
};


#endif //MINESWEEPER_BOARD_H
