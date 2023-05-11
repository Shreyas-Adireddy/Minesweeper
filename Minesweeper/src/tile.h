//
// Created by shrey on 3/30/2023.
//

#ifndef MINESWEEPER_TILE_H
#define MINESWEEPER_TILE_H
#include <SFML/Graphics.hpp>
using namespace sf;
struct Tile{
    Tile()=default;
    Tile(Texture& img, int i, int j, int size,std::map<std::string, Texture*>& textures);
    std::map<std::string, Texture*>& textures;
    Sprite graphic;
    int i, j;
    int x,y;
    int size;
    bool covered;
    bool flagged;
    bool has_mine;
    int adj_mine_count;
    const Texture* last_state;
    std::vector<Tile*> adjacent_tiles;
    void draw(RenderWindow& window);
    void change_sprite(Texture& chg);
};
#endif //MINESWEEPER_TILE_H
