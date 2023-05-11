//
// Created by shrey on 3/30/2023.
//

#include <iostream>
#include <random>
#include "board.h"

Board::Board(RenderWindow& window,std::map<std::string, Texture*>& textures,
             int width, int height, int size, int mines,std::random_device& rd)
    :window(window),textures(textures),mines(mines),paused(false),total_flags(0),
    status(0),show_leaderboard(false),debug_true(false),rd(rd),
    height(height),width(width),reset_game(false),revealAll(false),just_won(false){
    Texture& img = *textures["tile_covered"];
    for(int i=0;i<height;i++) {
        std::vector<Tile*> arr;
        for (int j = 0; j < width; j++) {
            Tile* tile = new Tile(img, i, j, size,textures);
            arr.push_back(tile);
        }board.push_back(arr);
    }
    init_tiles();
    generate_mines();
    init_adj_mine_count();
    face.setPosition((width/2.0f)*32 -32,32*(height+0.5f));
    debug.setPosition((width * 32) - 304, 32 * (height+0.5f));
    debug.setTexture(*textures["debug"]);
    play.setPosition((width * 32) - 240, 32 * (height+0.5f));
    play.setTexture(*textures["play"]);
    leaderboard.setPosition((width * 32) - 176, 32 * (height+0.5f));
    leaderboard.setTexture(*textures["leaderboard"]);
    curr_window_texture.create(window.getSize().x,window.getSize().y);
    screenshot.create(window.getSize().x, window.getSize().y);
}

void Board::draw() {
    for(int i=0;i<board.size();i++) {
        for (int j = 0; j < board[0].size(); j++) {
            board[i][j]->draw(window);
        }
    }draw_face();
    window.draw(debug);
    window.draw(leaderboard);
    draw_play_button();
}

void Board::left_clicked(float x, float y) {
    int dx,dy;
    dx= (int)(x/32);
    dy=(int)(y/32);
    if(face.getGlobalBounds().contains(x,y)){
        reset_board();
        reset_game = true;
    }else if(leaderboard.getGlobalBounds().contains(x,y)){
        show_leaderboard = !show_leaderboard;
    }else if(status==1 || status==2) return;
    else if(play.getGlobalBounds().contains(x, y)){
        paused = !paused;
        revealAll = !revealAll;
        pause_graphics();
    }else if(debug.getGlobalBounds().contains(x,y) && status==0){
        debug_true = !debug_true;
        if(debug_true){
            debug_mode();
        }else if(status==0){
            reset_debug_mode();
        }
    }if (dx<25 && dy<16 && !board[dy][dx]->flagged && !paused) {
        if(reveal_tile(board[dy][dx])){
            board[dy][dx]->covered= true;
            dfs_click(dy,dx);
        }
    }if((mines-total_flags) == 0){
        win_check();
    }
}

void Board::right_clicked(float x, float y) {
    int dx= (int)(x/32);
    int dy=(int)(y/32);
    if(status==1) return;
    if (dx<25 && dy<16) {
        if(!board[dy][dx]->covered || paused) return;
        if(board[dy][dx]->flagged) board[dy][dx]->graphic.setTexture(*textures["tile_covered"]);
        board[dy][dx]->flagged = !board[dy][dx]->flagged;
        if (board[dy][dx]->flagged){
            total_flags++;
        }else{
            total_flags--;
        }
    }if((mines-total_flags) == 0){
        win_check();
    }
}

Board::~Board() {
    for(int i=0;i<board.size();i++){
        for(int j=0;j<board[0].size();j++){
            delete board[i][j];
        }
    }for(auto iter = textures.begin();iter!=textures.end();iter++){
        delete iter->second;
    }
}

void Board::draw_face() {
    if (status == 0){
        face.setTexture(*textures["face_happy"]);
    }else if(status == 1){
        face.setTexture(*textures["face_win"]);
    }else if(status == 2){
        face.setTexture(*textures["face_lose"]);
    }window.draw(face);
}

void Board::draw_play_button() {
    if (paused){
        play.setTexture(*textures["pause"]);
    }else{
        play.setTexture(*textures["play"]);
    }window.draw(play);
}

void Board::init_tiles() {
    for(int i=0;i<board.size();i++) {
        for (int j = 0; j < board[0].size(); j++) {
            add_adj_tiles(board[i][j], i, j);
        }
    }
}

bool Board::in_bounds(int di, int dj) {
    if (di<board.size() && dj<board[0].size() && dj>=0 && di>=0){
        return true;
    }return false;
}

void Board::add_adj_tiles(Tile* tile, int i, int j) {
    int shift[8][2] = {{0,1},{1,0},{-1,0},{0,-1},
                       {1,1},{1,-1},{-1,1},{-1,-1}};
    int di,dj;
    for(auto s: shift){
        di = i+s[0];
        dj = j+s[1];
        if(in_bounds(di, dj)){
            tile->adjacent_tiles.push_back(board[di][dj]);
        }
    }
}

void Board::generate_mines() {
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> xDist(0, board.size() - 1);
    std::uniform_int_distribution<int> yDist(0, board[0].size() - 1);
    for (int i = 0; i < mines; i++){
        int x = xDist(gen);
        int y = yDist(gen);
        if (board[x][y]->has_mine)
            i--;
        else
            board[x][y]->has_mine = true;
    }
}

void Board::init_adj_mine_count() {
    for(auto& row:board){
        for(auto& tile: row){
            for(auto &adj: tile->adjacent_tiles){
                if (adj->has_mine){
                    tile->adj_mine_count++;
                }
            }
        }
    }
}

bool Board::reveal_tile(Tile* tile) {
    if(tile->has_mine){
        status=2;
        reveal_all_bombs();
        return false;
    }
    if(tile->adj_mine_count==0) {
        tile->change_sprite(*textures["tile_uncovered"]);
        tile->covered = false;
        return true;
    }tile->covered = false;
    if(tile->adj_mine_count>8)
        throw std::runtime_error("Too many mines");
    std::string temp = "tile_";
    temp += std::to_string(tile->adj_mine_count);
    tile->graphic.setTexture(*textures[temp]);
    return false;
}

void Board::debug_mode(Tile *curr) {
    for(auto& row:board){
        for(auto& tile: row){
            if(tile->has_mine){
                tile->graphic.setTexture(*textures["covered_mine"]);
            }
        }
    }
}

void Board::reset_debug_mode() {
    for(auto& row:board){
        for(auto& tile: row){
            if(tile->has_mine){
                tile->graphic.setTexture(*textures["tile_covered"]);
            }
        }
    }
}

void Board::reveal_all_bombs() {
    for(auto& row:board){
        for(auto& tile: row){
            if(tile->has_mine){
                tile->graphic.setTexture(*textures["uncovered_mine"]);
            }
        }
    }
}

void Board::reset_board() {
    for(auto& row:board){
        for(auto& tile: row){
            tile->graphic.setTexture(*textures["tile_covered"]);
            tile->has_mine= false;
            tile->flagged= false;
            tile->covered= true;
            tile->adj_mine_count=0;
        }
    }paused = false;
    show_leaderboard= false;
    debug_true= false;
    status=0;
    total_flags=0;
    generate_mines();
    init_adj_mine_count();
}

void Board::dfs_click(int row,int col) {
    if (row < 0 || row >= board.size() || col < 0 || col >= board[0].size()
        || !board[row][col]->covered /*|| board[row][col]->flagged || board[row][col]->has_mine*/) {
        return;
    }
    reveal_tile(board[row][col]);
    if (board[row][col]->adj_mine_count == 0) {
        dfs_click(row - 1, col - 1);
        dfs_click(row - 1, col);
        dfs_click(row - 1, col + 1);
        dfs_click(row, col - 1);
        dfs_click(row, col + 1);
        dfs_click(row + 1, col - 1);
        dfs_click(row + 1, col);
        dfs_click(row + 1, col + 1);
    }
}

void Board::win_check() {
    int currentMines=0;
    int revealedTiles =0;

    for(auto& row:board){
        for(auto& tile: row){
            if(tile->has_mine && tile->flagged){
                currentMines++;
            }
        }
    }for(auto& row:board){
        for(auto& tile: row){
            if(!tile->covered){
                revealedTiles++;
            }
        }
    }
    if(currentMines==mines && revealedTiles==(width*height - mines)){
        status=1;
        just_won = true;
    }
}

void Board::pause_graphics() {
    for(auto& row:board){
        for(auto& tile: row){
            if(revealAll){
                tile->last_state = tile->graphic.getTexture();
                tile->graphic.setTexture(*textures["tile_uncovered"]);
            }else{
                tile->graphic.setTexture(*tile->last_state);
            }
        }
    }
}

void Board::reveal_testing() {
    for(auto& row:board){
        for(auto& tile: row){
            if(!tile->has_mine){
                reveal_tile(tile);
            }
        }
    }
}

void Board::reveal_testing2() {
    for(auto& row:board){
        for(auto& tile: row){
            if(tile->has_mine){
                tile->flagged= true;
                total_flags++;
            }
        }
    }
}




