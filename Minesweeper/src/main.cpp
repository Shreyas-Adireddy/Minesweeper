#include <algorithm>
#include <iostream>
#include <map>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <vector>
#include <chrono>
#include <sstream>
#include <random>
#include <iomanip>
#include "board.h"

std::string HomeScreen(sf::RenderWindow& window);
void LoadLeaderBoard(sf::RenderWindow& window, std::multimap<int, std::string>& leaderboard);
void LoadAllTextures(std::map<std::string,Texture*>& map);
void LoadBoardConfig(int& width, int& height, int& mines);
int StringToSecondsConversion(std::string time);
void DisplayLeaderBoard(int width, int height, std::multimap<int, std::string> leaderboard);

void RenderTimer(int width, int height, std::map<std::string, Texture *> &textures, Sprite *timerDigits);
void UpdateCounter(int mines, std::map<std::string, Texture *> &textures, Sprite *counterDigits,int height);
int UpdateTimer(std::map<std::string, Texture *> &textures, std::chrono::high_resolution_clock::time_point &start, Sprite *counterDigits);

void UpdateLeaderBoardFile(std::multimap<int, std::string> &leaderboard);

std::string secondsToTime(int seconds) {
    int minutes = seconds / 60;
    int remainingSeconds = seconds % 60;
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << remainingSeconds;
    std::stringstream ss2;
    ss << std::setw(2) << std::setfill('0') << minutes;
    std::string time = ss2.str() + ":" + ss.str();
    return time;
}

int main() {
    using namespace sf;
    //Load board sizes and font
    int width, height, mines;
    LoadBoardConfig(width,height,mines);
    Font font;
    font.loadFromFile("files/font.ttf");
    //Home Screen
    RenderWindow window(VideoMode(width*32, (height*32)+100), "Minesweeper", Style::Close);

    std::string NameOfPlayer = HomeScreen(window);
    //Load textures
    std::map<std::string, Texture*> textures;
    std::multimap<int, std::string> leaderboard;
    LoadLeaderBoard(window, leaderboard);
    LoadAllTextures(textures);
    //Timer
    auto start = std::chrono::high_resolution_clock::now();
    Sprite* timerDigits = new Sprite[4];
    Sprite* counterDigits = new Sprite[4];
    RenderTimer(width, height, textures, timerDigits);
    std::random_device rd;
    Board game = Board(window, textures, width, height, 32,mines,rd);
    NameOfPlayer.pop_back();
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case Event::Closed:
                    window.close();
                    break;
                case Event::MouseButtonReleased:
                    if(event.mouseButton.button==Mouse::Left){
                        game.left_clicked(event.mouseButton.x, event.mouseButton.y);
                    }else if(event.mouseButton.button==Mouse::Right){
                        game.right_clicked(event.mouseButton.x, event.mouseButton.y);
                    }
                    break;
                case Event::KeyReleased:
                    if(event.key.code==Keyboard::W){
                        game.reveal_testing();
                        game.reveal_testing2();
                    }
                    break;
            }
            window.clear(Color::White);
            game.draw();
            UpdateCounter(game.mines - game.total_flags, textures, counterDigits,height);
            if(game.status==1 || game.status==2){
                game.paused = true;
                int currTime = UpdateTimer(textures, start, timerDigits);
                start = std::chrono::high_resolution_clock::now();
                if(game.just_won){
                    std::string FormatedName;
                    FormatedName = secondsToTime(currTime)+ ","+ NameOfPlayer;
                    leaderboard.insert(std::pair<int,std::string>(currTime,FormatedName));
                    UpdateLeaderBoardFile(leaderboard);
                    game.just_won = false;
                    game.face.setTexture(*textures["face_happy"]);
                    window.draw(game.face);
                    DisplayLeaderBoard(width,height,leaderboard);
                }
            }else if (game.reset_game){
                start = std::chrono::high_resolution_clock::now();
                game.reset_game = false;
            }else if(!game.paused) {
                UpdateTimer(textures, start, timerDigits);
            }if(game.show_leaderboard) {
                DisplayLeaderBoard(width, height, leaderboard);
                game.show_leaderboard = false;
            }if (game.revealAll){

            }
            for (int i = 0; i < 4; i++) {
                window.draw(timerDigits[i]);
            }for (int i=0;i < 4;i++){
                window.draw(counterDigits[i]);
            }
            window.display();
        }
    }
    delete[] counterDigits;
    delete[] timerDigits;
    return 0;
}

void UpdateLeaderBoardFile(std::multimap<int, std::string> &leaderboard) {
    std::ofstream file("files/leaderboard.txt");
    if(!file.is_open()) std::cout<<"File Not Open";
    int i =0;
    for(auto iter=leaderboard.begin();iter!=leaderboard.end();iter++){
        if (i==5)break;
        file<<iter->second<<std::endl;
        i++;
    }
}

int UpdateTimer(std::map<std::string, Texture *> &textures, std::chrono::high_resolution_clock::time_point &start, Sprite *counterDigits) {
    auto current = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(current - start);
    int totalSeconds = static_cast<int>(elapsed_time.count());
    int min = totalSeconds/60;
    int sec = totalSeconds%60;
    std::string minutes = std::to_string(min);
    std::string seconds = std::to_string(sec);
    minutes = minutes.size()<2 ? '0'+minutes:minutes;
    seconds = seconds.size()<2 ? '0'+seconds:seconds;
    minutes += seconds;
    for(int i=0;i<4;i++){
        std::string name = "digit_";
        name += minutes[i];
        counterDigits[i].setTexture(*textures[name]);
    }return totalSeconds;
}

void DisplayLeaderBoard(int width, int height, std::multimap<int, std::string> leaderboard) {
    RenderWindow leaderWindow(VideoMode((width*32/2), (height*32)/2+50),"Leaderboard",Style::Close);
    int windowWidth, windowHeight;
    windowWidth = (width*32/2);
    windowHeight = (height*32)/2+50;
    Font font;
    font.loadFromFile("files/font.ttf");
    Text title("LEADERBOARD",font, 20);
    title.setPosition(Vector2f((width*32/2) / 2.0f, ((height*32)/2+50) / 2.0f - 120));
    title.setFillColor(Color::White);
    title.setStyle(Text::Bold | Text::Underlined);
    FloatRect bounds = title.getLocalBounds();
    title.setOrigin(bounds.width/2,bounds.height/2);
    auto it = leaderboard.begin();
    std::string leaderboardString;
    for(int i=1;i<=5;i++){
        leaderboardString += std::to_string(i);
        leaderboardString += ".\t";
        std::stringstream ss(it->second);
        std::string curr;
        std::getline(ss,curr,',');
        leaderboardString += curr;
        leaderboardString += "\t";
        std::getline(ss,curr);
        leaderboardString += curr;
        leaderboardString += "\n\n";
        it++;
    }
    Text leaderboardText(leaderboardString, font, 18);
    leaderboardText.setFillColor(Color::White);
    leaderboardText.setPosition(Vector2f(windowWidth / 2.0f, windowHeight / 2.0f + 20));
    leaderboardText.setStyle(Text::Bold);
    bounds = leaderboardText.getLocalBounds();
    leaderboardText.setOrigin(bounds.width/2,bounds.height/2);
    while (leaderWindow.isOpen())
    {
        sf::Event event;
        while (leaderWindow.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                leaderWindow.close();
        }
        leaderWindow.clear(Color::Blue);
        leaderWindow.draw(leaderboardText);
        leaderWindow.draw(title);
        leaderWindow.display();
    }
}


void UpdateCounter(int mines, std::map<std::string, Texture *> &textures, Sprite *counterDigits, int height) {
    std::string counter = std::to_string(mines);
    if(counter[0]=='-') {
        counter.erase(0,1);
        counterDigits[3].setPosition(12, 32 * (height + 0.5f) + 16);
        counterDigits[3].setTexture(*textures["digit_-"]);
    }else{
        counterDigits[3].setTexture(Texture());
    }if (counter.size()<3)
        counter = "0"+counter;
    counterDigits[0].setPosition(33, 32 * (height+0.5f)+16);
    counterDigits[1].setPosition(54, 32 * (height + 0.5f) + 16);
    counterDigits[2].setPosition(75, 32 * (height + 0.5f) + 16);
    for (int i=0;i<3;i++){
        std::string textName;
        textName+= "digit_";
        textName+= counter[i];
        counterDigits[i].setTexture(*textures[textName]);
    }
}

void RenderTimer(int width, int height, std::map<std::string, Texture *> &textures, Sprite *timerDigits) {
    for (int i = 0; i < 2; i++) {
        timerDigits[i].setTexture(*textures["digit_0"]);
        timerDigits[i].setPosition((width*32)-97 + (i * 21), 32*(height+0.5f)+16);
    }
    for (int i = 2; i < 4; i++) {
        timerDigits[i].setTexture(*textures["digit_0"]);
        timerDigits[i].setPosition((width*32)-94 + (i * 21), 32*(height+0.5f)+16);
    }
}

void LoadAllTextures(std::map<std::string,Texture*>& map){
    auto temp = new Texture();
    temp->loadFromFile("files/images/tile_hidden.png");
    map["tile_covered"] = temp;

    temp = new Texture();
    temp->loadFromFile("files/images/tile_revealed.png");
    map["tile_uncovered"] = temp;

    temp = new Texture();
    temp->loadFromFile("files/images/play.png");
    map["play"] = temp;

    temp = new Texture();
    temp->loadFromFile("files/images/pause.png");
    map["pause"] = temp;

    temp = new Texture();
    temp->loadFromFile("files/images/face_happy.png");
    map["face_happy"] = temp;

    temp = new Texture();
    temp->loadFromFile("files/images/face_lose.png");
    map["face_lose"] = temp;

    temp = new Texture();
    temp->loadFromFile("files/images/face_win.png");
    map["face_win"] = temp;

    temp = new Texture();
    temp->loadFromFile("files/images/number_8.png");
    map["number_8"] = temp;

    temp = new Texture();
    temp->loadFromFile("files/images/number_7.png");
    map["number_7"] = temp;

    temp = new Texture();
    temp->loadFromFile("files/images/number_6.png");
    map["number_6"] = temp;

    temp = new Texture();
    temp->loadFromFile("files/images/number_5.png");
    map["number_5"] = temp;

    temp = new Texture();
    temp->loadFromFile("files/images/number_4.png");
    map["number_4"] = temp;

    temp = new Texture();
    temp->loadFromFile("files/images/number_3.png");
    map["number_3"] = temp;

    temp = new Texture();
    temp->loadFromFile("files/images/number_2.png");
    map["number_2"] = temp;

    temp = new Texture();
    temp->loadFromFile("files/images/number_1.png");
    map["number_1"] = temp;

    temp = new Texture();
    temp->loadFromFile("files/images/flag.png");
    map["flag"] = temp;

    map["debug"] = new Texture();
    map["debug"]->loadFromFile("files/images/debug.png");

    map["mine"] = new Texture();
    map["mine"]->loadFromFile("files/images/mine.png");

    map["leaderboard"] = new Texture();
    map["leaderboard"]->loadFromFile("files/images/leaderboard.png");

    //Loads all the digits
    auto digitsTextures = new Texture();
    digitsTextures->loadFromFile("files/images/digits.png");
    IntRect textureRect0(0,0,21,32);
    IntRect textureRect1(21,0,21,32);
    IntRect textureRect2(42,0,21,32);
    IntRect textureRect3(63,0,21,32);
    IntRect textureRect4(84,0,21,32);
    IntRect textureRect5(105,0,21,32);
    IntRect textureRect6(126,0,21,32);
    IntRect textureRect7(147,0,21,32);
    IntRect textureRect8(168,0,21,32);
    IntRect textureRect9(189,0,21,32);
    IntRect textureRect_(210,0,21,32);

    Sprite digit0(*digitsTextures, textureRect0);
    Sprite digit1(*digitsTextures, textureRect1);
    Sprite digit2(*digitsTextures, textureRect2);
    Sprite digit3(*digitsTextures, textureRect3);
    Sprite digit4(*digitsTextures, textureRect4);
    Sprite digit5(*digitsTextures, textureRect5);
    Sprite digit6(*digitsTextures, textureRect6);
    Sprite digit7(*digitsTextures, textureRect7);
    Sprite digit8(*digitsTextures, textureRect8);
    Sprite digit9(*digitsTextures, textureRect9);
    Sprite digit_(*digitsTextures, textureRect_);

    Image img0 = digit0.getTexture()->copyToImage();
    Image img1 = digit1.getTexture()->copyToImage();
    Image img2 = digit2.getTexture()->copyToImage();
    Image img3 = digit3.getTexture()->copyToImage();
    Image img4 = digit4.getTexture()->copyToImage();
    Image img5 = digit5.getTexture()->copyToImage();
    Image img6 = digit6.getTexture()->copyToImage();
    Image img7 = digit7.getTexture()->copyToImage();
    Image img8 = digit8.getTexture()->copyToImage();
    Image img9 = digit9.getTexture()->copyToImage();
    Image img_ = digit_.getTexture()->copyToImage();

    map["digit_0"] = new Texture();
    map["digit_0"]->loadFromImage(img0, textureRect0);
    map["digit_1"] = new Texture();
    map["digit_1"]->loadFromImage(img1, textureRect1);
    map["digit_2"] = new Texture();
    map["digit_2"]->loadFromImage(img2, textureRect2);
    map["digit_3"] = new Texture();
    map["digit_3"]->loadFromImage(img3, textureRect3);
    map["digit_4"] = new Texture();
    map["digit_4"]->loadFromImage(img4, textureRect4);
    map["digit_5"] = new Texture();
    map["digit_5"]->loadFromImage(img5, textureRect5);
    map["digit_6"] = new Texture();
    map["digit_6"]->loadFromImage(img6, textureRect6);
    map["digit_7"] = new Texture();
    map["digit_7"]->loadFromImage(img7, textureRect7);
    map["digit_8"] = new Texture();
    map["digit_8"]->loadFromImage(img8, textureRect8);
    map["digit_9"] = new Texture();
    map["digit_9"]->loadFromImage(img9, textureRect9);
    map["digit_-"] = new Texture();
    map["digit_-"]->loadFromImage(img_, textureRect_);

    //Overlay tile and flag
    RenderTexture render;
    render.create(32,32);
    Sprite spt1(*map["tile_covered"]);
    Sprite spt2(*map["flag"]);
    render.draw(spt1);
    render.draw(spt2);
    render.display();
    Texture* ptr = new Texture();
    ptr->create(map["tile_covered"]->getSize().x, map["tile_covered"]->getSize().y);
    ptr->update(render.getTexture().copyToImage());
    map["tile_flagged"] = ptr;

    render.clear();
    Sprite spt3(*map["tile_uncovered"]);
    Sprite spt4(*map["number_1"]);
    render.draw(spt3);
    render.draw(spt4);
    map["tile_1"] = new Texture();
    map["tile_1"]->create(map["tile_uncovered"]->getSize().x,map["tile_uncovered"]->getSize().y);
    map["tile_1"]->update(render.getTexture().copyToImage());

    render.clear();
    Sprite spt5(*map["tile_uncovered"]);
    Sprite spt6(*map["number_2"]);
    render.draw(spt5);
    render.draw(spt6);
    map["tile_2"] = new Texture();
    map["tile_2"]->create(map["tile_uncovered"]->getSize().x,map["tile_uncovered"]->getSize().y);
    map["tile_2"]->update(render.getTexture().copyToImage());

    render.clear();
    Sprite spt7(*map["tile_uncovered"]);
    Sprite spt8(*map["number_3"]);
    render.draw(spt7);
    render.draw(spt8);
    map["tile_3"] = new Texture();
    map["tile_3"]->create(map["tile_uncovered"]->getSize().x,map["tile_uncovered"]->getSize().y);
    map["tile_3"]->update(render.getTexture().copyToImage());

    render.clear();
    Sprite spt9(*map["tile_uncovered"]);
    Sprite spt10(*map["number_4"]);
    render.draw(spt9);
    render.draw(spt10);
    map["tile_4"] = new Texture();
    map["tile_4"]->create(map["tile_uncovered"]->getSize().x,map["tile_uncovered"]->getSize().y);
    map["tile_4"]->update(render.getTexture().copyToImage());

    render.clear();
    Sprite spt11(*map["tile_uncovered"]);
    Sprite spt12(*map["number_5"]);
    render.draw(spt11);
    render.draw(spt12);
    map["tile_5"] = new Texture();
    map["tile_5"]->create(map["tile_uncovered"]->getSize().x,map["tile_uncovered"]->getSize().y);
    map["tile_5"]->update(render.getTexture().copyToImage());

    render.clear();
    Sprite spt13(*map["tile_uncovered"]);
    Sprite spt14(*map["number_6"]);
    render.draw(spt13);
    render.draw(spt14);
    map["tile_6"] = new Texture();
    map["tile_6"]->create(map["tile_uncovered"]->getSize().x,map["tile_uncovered"]->getSize().y);
    map["tile_6"]->update(render.getTexture().copyToImage());

    render.clear();
    Sprite spt15(*map["tile_uncovered"]);
    Sprite spt16(*map["number_7"]);
    render.draw(spt15);
    render.draw(spt16);
    map["tile_7"] = new Texture();
    map["tile_7"]->create(map["tile_uncovered"]->getSize().x,map["tile_uncovered"]->getSize().y);
    map["tile_7"]->update(render.getTexture().copyToImage());

    render.clear();
    Sprite spt17(*map["tile_uncovered"]);
    Sprite spt18(*map["number_8"]);
    render.draw(spt17);
    render.draw(spt18);
    map["tile_8"] = new Texture();
    map["tile_8"]->create(map["tile_uncovered"]->getSize().x,map["tile_uncovered"]->getSize().y);
    map["tile_8"]->update(render.getTexture().copyToImage());

    render.clear();
    Sprite spt19(*map["tile_covered"]);
    Sprite spt20(*map["mine"]);
    render.draw(spt19);
    render.draw(spt20);
    map["covered_mine"] = new Texture();
    map["covered_mine"]->create(map["tile_covered"]->getSize().x,map["tile_covered"]->getSize().y);
    map["covered_mine"]->update(render.getTexture().copyToImage());

    render.clear();
    Sprite spt21(*map["tile_uncovered"]);
    Sprite spt22(*map["mine"]);
    render.draw(spt21);
    render.draw(spt22);
    map["uncovered_mine"] = new Texture();
    map["uncovered_mine"]->create(map["tile_uncovered"]->getSize().x,map["tile_uncovered"]->getSize().y);
    map["uncovered_mine"]->update(render.getTexture().copyToImage());
}

void LoadLeaderBoard(sf::RenderWindow& window, std::multimap<int,std::string>& leaderboard){
    using namespace std;
    std::ifstream file("files/leaderboard.txt");
    if(!file.is_open()) throw std::runtime_error("can't open leaderboard.txt");
    string line;
    while(getline(file, line)){
        stringstream ss(line);
        string currTime;
        string currName;
        getline(ss,currTime,',');
        getline(ss,currName);
        int sec = StringToSecondsConversion(currTime);
        leaderboard.insert(pair<int,string>(sec,line));
    }
}
int StringToSecondsConversion(std::string time){
    std::stringstream ss(time);
    std::string minutes_str, seconds_str;

    std::getline(ss, minutes_str, ':');
    std::getline(ss, seconds_str);

    int minutes = std::stoi(minutes_str);
    int seconds = std::stoi(seconds_str);

    return minutes * 60 + seconds;
}

std::string HomeScreen(sf::RenderWindow& window) {
    Font font;
    font.loadFromFile("files/font.ttf");
    //Title
    Text title;
    title.setString("WELCOME TO MINESWEEPER!");
    title.setCharacterSize(24);
    title.setFillColor(sf::Color::White);
    title.setFont(font);
    title.setStyle(Text::Bold | Text::Underlined);
    FloatRect textBounds = title.getLocalBounds();
    title.setOrigin(textBounds.width / 2, textBounds.height / 2);
    title.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f -150);
    //Enter Name
    Text enterName;
    enterName.setString("Enter your name:");
    enterName.setCharacterSize(18);
    enterName.setFillColor(Color::Yellow);
    enterName.setFont(font);
    enterName.setStyle(Text::Bold);
    FloatRect enterNameBounds = enterName.getLocalBounds();
    enterName.setOrigin(enterNameBounds.width/2, enterNameBounds.height/2);
    enterName.setPosition(window.getSize().x/2.0f, window.getSize().y/2.0f -75);
    //Input
    Text inputText("|",font,20);
    inputText.setPosition(window.getSize().x / 2, window.getSize().y / 2 -45);
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case Event::TextEntered:
                    if (event.text.unicode < 128) {
                        if (event.text.unicode == 8 && inputText.getString() != "|") {
                            std::string temp = inputText.getString();
                            temp.pop_back();
                            temp.pop_back();
                            inputText.setString(temp + "|");
                            auto textRect = inputText.getLocalBounds();
                            inputText.setOrigin(textRect.left + textRect.width/2.0f, textRect.top  + textRect.height/2.0f);
                        }
                        else if (inputText.getString().getSize() < 10
                                 && event.text.unicode != 8 && isalpha(event.text.unicode)){
                            std::string str = inputText.getString();
                            str.pop_back();
                            str += static_cast<char>(event.text.unicode);
                            str[0] = toupper(str[0]);
                            std::transform(str.begin()+1, str.end(),
                                           str.begin()+1, tolower);
                            inputText.setString((String)str +"|");
                            auto textRect = inputText.getLocalBounds();
                            inputText.setOrigin(textRect.left + textRect.width/2.0f, textRect.top  + textRect.height/2.0f);
                        }
                    }break;
                case Event::Closed:
                    window.close();
                    break;
                case Event::KeyPressed:
                    if (event.key.code == Keyboard::Enter && inputText.getString() != "|")
                        return inputText.getString();
            }
            window.clear(Color::Blue);
            window.draw(title);
            window.draw(enterName);
            window.draw(inputText);
            window.display();
        }
    }
}

void LoadBoardConfig(int& width, int& height, int& mines){
    std::ifstream config("files/board_config.cfg");
    std::string temp;
    std::getline(config, temp);
    width = std::stoi(temp);
    std::getline(config, temp);
    height = std::stoi(temp);
    std::getline(config, temp);
    mines = std::stoi(temp);
}