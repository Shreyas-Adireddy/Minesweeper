//
// Created by shrey on 3/30/2023.
//
#include "tile.h"

Tile::Tile(Texture& img, int i, int j, int size,std::map<std::string, Texture*>& textures):textures(textures) {
    this->i = i;
    this->j = j;
    this->size = size;
    this->x = j*size;
    this->y = i*size;
    this->graphic.setTexture(img, true);
    this->graphic.setPosition(x, y);
    flagged = false;
    covered = true;
    has_mine= false;
    adj_mine_count = 0;
}

void Tile::draw(RenderWindow& window) {
    if (flagged){
        graphic.setTexture(*textures["tile_flagged"]);
        window.draw(graphic);
    }else{

        window.draw(graphic);
    }
}

void Tile::change_sprite(Texture& chg) {
    graphic.setTexture(chg);
}

