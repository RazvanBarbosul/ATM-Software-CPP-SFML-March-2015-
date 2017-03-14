#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

class Atm
{
private:
    //- Title and Version
    std::string title = "ATM Software";
    std::string ver = "1.0";

    //- Screen Size
    const int SCREEN_WIDTH = 960, SCREEN_HEIGHT = 620;

    //- States
    bool card_visible = true, cash_large_visible = false, cash_small_visible = false, receipt_visible = false;
    bool declicked = false;
    unsigned short int scr_state = 1;
    unsigned short int pin = 0; unsigned short int pin_count = 0; unsigned short int pin_retry = 0;
    int amount = 0; unsigned short int amount_count = 0;
    bool blocked = false;
    bool flag = false;

    //- User Data Structure
    struct User
    {
        std::string iban;
        std::string last_name;
        std::string first_name;
        unsigned short int pin;
        unsigned long long int balance;
    };

    //- General
    sf::RenderWindow window;
    sf::Event event;
    sf::Font font;
    sf::Clock frame_clock;
    sf::Clock clock;
    sf::Time elapsed;

    //- Textures and Sprites
    sf::Texture backgnd_texture;           sf::Sprite backgnd_sprite;
    sf::Texture card_texture;              sf::Sprite card_sprite;
    sf::Texture cash_large_texture;        sf::Sprite cash_large_sprite;
    sf::Texture cash_small_texture;        sf::Sprite cash_small_sprite;
    sf::Texture receipt_texture;           sf::Sprite receipt_sprite;

    //- Sound Buffers and Sounds
    sf::SoundBuffer card_snd_buf;          sf::Sound card_snd;
    sf::SoundBuffer menu_snd_buf;          sf::Sound menu_snd;
    sf::SoundBuffer click_snd_buf;         sf::Sound click_snd;
    sf::SoundBuffer key_snd_buf;           sf::Sound key_snd;
    sf::SoundBuffer cash_snd_buf;          sf::Sound cash_snd;
    sf::SoundBuffer print_receipt_snd_buf; sf::Sound print_receipt_snd;

    //- Text
    sf::Text scr_clock;
    sf::Text username_scr;
    sf::Text iban_scr;
    sf::Text L1_txt;
    sf::Text R1_txt;
    sf::Text R3_txt;
    sf::Text dialog;
    sf::Text live_txt;

    //- Shapes
    sf::RectangleShape pin_border_shape;
    sf::RectangleShape amount_border_shape;

    //- Database
    std::vector<User> client;

    //- Text files
    std::ifstream database;
    std::ofstream log;

    //- Out String Stream
    std::ostringstream oss;

    //- Screen Info Strings
    std::ostringstream username_scr_str;
    std::ostringstream iban_scr_str;
    std::ostringstream convert;
    std::ostringstream balance;

    //- Chat arrays for live text
    std::string pin_live_txt = "****"; std::string amount_live_txt = "";

    void init_win()
    {
        window.create(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), program_title(), sf::Style::Titlebar | sf::Style::Close);
        window.setFramerateLimit(60);
        window.setKeyRepeatEnabled(false);
    }

    void init()
    {
        //- Create new log file
        log.open(get_name_log().c_str());

        //- Initialize Window
        init_win();

        //- Initialize CLI
        system("color 0A");
        oss << "================================================================================"; log_out(oss.str());
        oss << "==================================ATM Software=================================="; log_out(oss.str());
        oss << "================================================================================"; log_out(oss.str());
        oss << get_time_cli() << "ATM is now powered on"; log_out(oss.str());

        //- Load database
        database.open("res/database/database.txt");
        if(!database.fail())
        {
            oss << get_time_cli() << "User database loaded"; log_out(oss.str());
        }
        else
        {
            oss << get_time_cli() << "User database not found"; log_out(oss.str());
            window.close();
        }
        load_clients();

        //- Load fonts
        if(font.loadFromFile("res/courier_new.ttf"))
        {
            oss << get_time_cli() << "Font loaded"; log_out(oss.str());
        }
        else
        {
            oss << get_time_cli() << "Font not found"; log_out(oss.str());
            window.close();
        }

        //- Load textures
        if(!backgnd_texture.loadFromFile("res/backgnd_texture.png") ||
            !card_texture.loadFromFile("res/card_texture.png") ||
            !cash_large_texture.loadFromFile("res/cash_large_texture.jpg") ||
            !cash_small_texture.loadFromFile("res/cash_small_texture.jpg") ||
            !receipt_texture.loadFromFile("res/receipt_texture.jpg"))
        {
            oss << get_time_cli() << "One or more textures not found"; log_out(oss.str());
            window.close();
        }
        else
        {
            oss << get_time_cli() << "Textures loaded"; log_out(oss.str());
        }

        //- Load sounds in the sound buffer
        if(!card_snd_buf.loadFromFile("res/card_snd.wav") ||
            !menu_snd_buf.loadFromFile("res/menu_snd.wav") ||
            !click_snd_buf.loadFromFile("res/click_snd.wav") ||
            !key_snd_buf.loadFromFile("res/key_snd.wav") ||
            !cash_snd_buf.loadFromFile("res/cash_snd.wav") ||
            !print_receipt_snd_buf.loadFromFile("res/print_receipt_snd.wav"))
        {
            oss << get_time_cli() << "One or more sounds not found"; log_out(oss.str());
            window.close();
        }
        else
        {
            oss << get_time_cli() << "Sounds loaded"; log_out(oss.str());
        }

        //- Ready to go
        oss << get_time_cli() << "ATM is ready to use"; log_out(oss.str());

        //- Assign font to text
        scr_clock.setFont(font);
        username_scr.setFont(font);
        iban_scr.setFont(font);
        L1_txt.setFont(font);
        R1_txt.setFont(font);
        R3_txt.setFont(font);
        dialog.setFont(font);
        live_txt.setFont(font);

        //- Assign texture to sprite
        backgnd_sprite.setTexture(backgnd_texture);
        card_sprite.setTexture(card_texture);             card_sprite.setPosition(740, 200);
        cash_large_sprite.setTexture(cash_large_texture); cash_large_sprite.setPosition(90, 370);
        cash_small_sprite.setTexture(cash_small_texture); cash_small_sprite.setPosition(695, 463);
        receipt_sprite.setTexture(receipt_texture);       receipt_sprite.setPosition(740, 54);

        //- Assign buffer to sounds
        card_snd.setBuffer(card_snd_buf);
        menu_snd.setBuffer(menu_snd_buf);
        click_snd.setBuffer(click_snd_buf);
        key_snd.setBuffer(key_snd_buf);
        cash_snd.setBuffer(cash_snd_buf);
        print_receipt_snd.setBuffer(print_receipt_snd_buf);
    }

    void init_states()
    {
        //- Initialize States
        card_visible = true; cash_large_visible = false; cash_small_visible = false; receipt_visible = false;
        declicked = false;
        scr_state = 1;
        pin = 0; pin_count = 0; pin_retry = 0;
        amount = 0; amount_count = 0;
        amount_live_txt = "";
        convert.str("");
        balance.str("");
    }

    void handle_events(sf::Event &event)
    {
        while(window.pollEvent(event))
        {
            switch(event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::MouseButtonReleased:
                declicked = false;
                break;
            }
            if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && !declicked)
            {
                if((385 <= sf::Mouse::getPosition(window).x) && (sf::Mouse::getPosition(window).x <= 455) && !cash_large_visible) //- Column 4 Keypad
                {
                    if((410 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 449)) //- Button: Cancel
                    {
                        if(!card_visible)
                        {
                            menu_snd.play();
                            if(scr_state != 1 && scr_state != 2 && scr_state != 21 && scr_state != 22 && scr_state != 23)
                            {
                                oss << get_time_cli() << client.at(0).last_name << " " << client.at(0).first_name << " canceled the session"; log_out(oss.str());
                            }
                            event_routine(2);
                        }
                        declicked = true;
                    }
                }
                if((12 <= sf::Mouse::getPosition(window).x) && (sf::Mouse::getPosition(window).x <= 92) && !declicked) //- Exit Button (x axis)
                {
                    if((563 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 603)) //- Exit Button (y axis)
                    {
                        click_snd.play();
                        window.close();
                    }
                }
            }
        }
    }

    int get_mouse_input()
    {
        //===============================
        //Input Codes (get_mouse_input())
        //===============================
        //Screen Buttons:
        //L1 = 1    R1 = 5
        //L2 = 2    R2 = 6
        //L3 = 3    R3 = 7
        //L4 = 4    R4 = 8
        //===============================
        //Keys:
        //1 = 9     2 = 12    3 = 16
        //4 = 10    5 = 13    6 = 17
        //7 = 11    8 = 14    9 = 18
        //          0 = 15
        //===============================
        //Action Buttons:
        //Cancel = (NOT HANDLED HERE)
        //Clear  = 19
        //OK     = 20
        //===============================
        //Objects:
        //card       = 21
        //cash_large = 22
        //cash_small = 23
        //receipt    = 24
        //===============================
        //===============================

        if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && (!declicked))
        {
            if((11 <= sf::Mouse::getPosition(window).x) && (sf::Mouse::getPosition(window).x <= 55)) //- Left Column Screen Buttons
            {
                if((125 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 163)) //- Button: L1
                {
                    declicked = true;
                    return 1;
                }
                if((174 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 210)) //- Button: L2
                {
                    declicked = true;
                    return 2;
                }
                if((221 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 259)) //- Button: L3
                {
                    declicked = true;
                    return 3;
                }
                if((269 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 305)) //- Button: L4
                {
                    declicked = true;
                    return 4;
                }
            }
            if((588 <= sf::Mouse::getPosition(window).x) && (sf::Mouse::getPosition(window).x <= 632)) //- Right Column Screen Buttons
            {
                if((127 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 163)) //- Button: R1
                {
                    declicked = true;
                    return 5;
                }
                if((175 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 212)) //- Button: R2
                {
                    declicked = true;
                    return 6;
                }
                if((223 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 259)) //- Button: R3
                {
                    declicked = true;
                    return 7;
                }
                if((270 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 308)) //- Button: R4
                {
                    declicked = true;
                    return 8;
                }
            }
            if((209 <= sf::Mouse::getPosition(window).x) && (sf::Mouse::getPosition(window).x <= 255) && !cash_large_visible) //- Column 1 Keypad
            {
                if((410 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 449)) //- Key: 1
                {
                    declicked = true;
                    return 9;
                }
                if((457 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 496)) //- Key: 4
                {
                    declicked = true;
                    return 10;
                }
                if((504 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 543)) //- Key: 7
                {
                    declicked = true;
                    return 11;
                }
            }
            if((264 <= sf::Mouse::getPosition(window).x) && (sf::Mouse::getPosition(window).x <= 310) && !cash_large_visible) //- Column 2 Keypad
            {
                if((410 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 449)) //- Key: 2
                {
                    declicked = true;
                    return 12;
                }
                if((457 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 496)) //- Key: 5
                {
                    declicked = true;
                    return 13;
                }
                if((504 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 543)) //- Key: 8
                {
                    declicked = true;
                    return 14;
                }
                if((551 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 590)) //- Key 0
                {
                    declicked = true;
                    return 15;
                }
            }
            if((319 <= sf::Mouse::getPosition(window).x) && (sf::Mouse::getPosition(window).x <= 365) && !cash_large_visible) //- Column 3 Keypad
            {
                if((410 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 449)) //- Key: 3
                {
                    declicked = true;
                    return 16;
                }
                if((457 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 496)) //- Key: 6
                {
                    declicked = true;
                    return 17;
                }
                if((504 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 543)) //- Key: 9
                {
                    declicked = true;
                    return 18;
                }
            }
            if((385 <= sf::Mouse::getPosition(window).x) && (sf::Mouse::getPosition(window).x <= 455) && !cash_large_visible) //- Column 4 Keypad
            {
                if((457 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 496)) //- Button: Clear
                {
                    declicked = true;
                    return 19;
                }
                if((504 <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= 543)) //- Button: OK
                {
                    declicked = true;
                    return 20;
                }
            }
            if((card_sprite.getGlobalBounds().left <= sf::Mouse::getPosition(window).x) && (sf::Mouse::getPosition(window).x <= (card_sprite.getGlobalBounds().left + card_sprite.getGlobalBounds().width))) //- Object: card (x axis)
            {
                if((card_sprite.getGlobalBounds().top <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= (card_sprite.getGlobalBounds().top + card_sprite.getGlobalBounds().height)) && card_visible) //- Object: card (y axis) and visibility
                {
                    declicked = true;
                    return 21;
                }
            }
            if((cash_large_sprite.getGlobalBounds().left <= sf::Mouse::getPosition(window).x) && (sf::Mouse::getPosition(window).x <= (cash_large_sprite.getGlobalBounds().left + cash_large_sprite.getGlobalBounds().width))) //- Object: cash_large (x axis)
            {
                if((cash_large_sprite.getGlobalBounds().top <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= (cash_large_sprite.getGlobalBounds().top + cash_large_sprite.getGlobalBounds().height)) && cash_large_visible) //- Object: cash_large (y axis) and visibility
                {
                    declicked = true;
                    return 22;
                }
            }
            if((cash_small_sprite.getGlobalBounds().left <= sf::Mouse::getPosition(window).x) && (sf::Mouse::getPosition(window).x <= (cash_small_sprite.getGlobalBounds().left + cash_small_sprite.getGlobalBounds().width))) //- Object: cash_small (x axis)
            {
                if((cash_small_sprite.getGlobalBounds().top <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= (cash_small_sprite.getGlobalBounds().top + cash_small_sprite.getGlobalBounds().height)) && cash_small_visible) //- Object: cash_small (y axis) and visibility
                {
                    declicked = true;
                    return 23;
                }
            }
            if((receipt_sprite.getGlobalBounds().left <= sf::Mouse::getPosition(window).x) && (sf::Mouse::getPosition(window).x <= (receipt_sprite.getGlobalBounds().left + receipt_sprite.getGlobalBounds().width))) //- Object: receipt (x axis)
            {
                if((receipt_sprite.getGlobalBounds().top <= sf::Mouse::getPosition(window).y) && (sf::Mouse::getPosition(window).y <= (receipt_sprite.getGlobalBounds().top + receipt_sprite.getGlobalBounds().height)) && receipt_visible) //- Object: receipt (y axis) and visibility
                {
                    declicked = true;
                    return 24;
                }
            }
        }
        return 0;
    }

    void update()
    {
        //======================================================================================================================================================================================================================
        //Screen States (scr_state)
        //======================================================================================================================================================================================================================
        //(1)Insert card --> (23)Processing --> (2)Insert PIN --> (3)MAIN MENU --Withdraw----------> (4)Enter Amount --> (5)Confirm -------------------------> (6)Processing -----> (7)Receipt? --y/n--> (8)Another transaction?
        //                                                    |                |                                     |
        //                                                    |                |                                     |
        //                                                    |                |                                     |
        //                                                    |                |                                     --> (10)Not enough funds
        //                                                    |                |
        //                                                    |                --Deposit-----------> (11)Enter amount --> (12)Confirm --> (13)Insert cash ---> (24)Processing -->(14)Receipt? --y/n---> (15)Another transaction?
        //                                                    |                |
        //                                                    |                |
        //                                                    |                |
        //                                                    |                --Account Balance---> (17)Processing ----> (18)Balance = ***. Receipt? -y/n---> (19)Another transaction?
        //                                                    |
        //                                                    |
        //                                                    |
        //                                                    --> (21)Wrong PIN
        //                                                    |
        //                                                    --> (22)Account Blocked (3 Wrong Attempts)
        //======================================================================================================================================================================================================================
        //======================================================================================================================================================================================================================

        switch(scr_state)
        {
        case 1: //- (1) Insert Card
            if(get_mouse_input() == 21)
            {
                event_routine(1);
                scr_state = 23;
            }
            break;
        case 2: //- (2) Insert PIN
            if(pin_count < 4)
            {
                switch(get_mouse_input())
                {
                case 9: //- 1
                    event_routine(3);
                    pin = pin * 10 + 1;
                    pin_count++;
                    break;
                case 10://- 4
                    event_routine(3);
                    pin = pin * 10 + 4;
                    pin_count++;
                    break;
                case 11://- 7
                    event_routine(3);
                    pin = pin * 10 + 7;
                    pin_count++;
                    break;
                case 12://- 2
                    event_routine(3);
                    pin = pin * 10 + 2;
                    pin_count++;
                    break;
                case 13://- 5
                    event_routine(3);
                    pin = pin * 10 + 5;
                    pin_count++;
                    break;
                case 14://- 8
                    event_routine(3);
                    pin = pin * 10 + 8;
                    pin_count++;
                    break;
                case 15://- 0
                    event_routine(3);
                    pin = pin * 10 + 0;
                    pin_count++;
                    break;
                case 16://- 3
                    event_routine(3);
                    pin = pin * 10 + 3;
                    pin_count++;
                    break;
                case 17://- 6
                    event_routine(3);
                    pin = pin * 10 + 6;
                    pin_count++;
                    break;
                case 18://- 9
                    event_routine(3);
                    pin = pin * 10 + 9;
                    pin_count++;
                    break;
                case 19://- Clear
                    event_routine(4);
                    pin_count = 0;
                    pin = 0;
                    break;
                }
            }
            if(pin_count == 4)
            {
                switch(get_mouse_input())
                {
                case 19://- Clear
                    event_routine(4);
                    pin_count = 0;
                    pin = 0;
                    break;
                case 20://- OK
                    event_routine(4);
                    if(pin == client.at(0).pin)
                    {
                        oss << get_time_cli() << "Cardholder successfully authenticated:"; log_out(oss.str());
                        oss << "\t\t\t  Full Name: " << client.at(0).last_name << " " << client.at(0).first_name; log_out(oss.str());
                        oss << "\t\t\t  IBAN: " << client.at(0).iban; log_out(oss.str());
                        scr_state = 3;
                    }
                    else
                    {
                        pin_retry++;
                        if(pin_retry == 3)
                        {
                            oss << get_time_cli() << "Cardholder entered a wrong PIN 3 times in a row"; log_out(oss.str());
                            scr_state = 22;
                        }
                        else
                        {
                            oss << get_time_cli() << "Cardholder entered a wrong PIN"; log_out(oss.str());
                            scr_state = 21;
                        }
                    }
                    pin = 0;
                    pin_count = 0;
                    break;
                }
            }
            break;
        case 3: //- (3) MAIN MENU
            switch(get_mouse_input())
            {
                case 1:
                    event_routine(4);
                    scr_state = 4;
                    break;
                case 5:
                    event_routine(4);
                    scr_state = 11;
                    break;
                case 7:
                    event_routine(4);
                    scr_state = 17;
                    break;
            }
            break;
        case 4: //- (4) Enter amount (Withdraw)
            if(amount_count < 7)
            {
                switch(get_mouse_input())
                {
                case 9: //- 1
                    event_routine(3);
                    amount = amount * 10 + 1;
                    amount_count++;
                    break;
                case 10://- 4
                    event_routine(3);
                    amount = amount * 10 + 4;
                    amount_count++;
                    break;
                case 11://- 7
                    event_routine(3);
                    amount = amount * 10 + 7;
                    amount_count++;
                    break;
                case 12://- 2
                    event_routine(3);
                    amount = amount * 10 + 2;
                    amount_count++;
                    break;
                case 13://- 5
                    event_routine(3);
                    amount = amount * 10 + 5;
                    amount_count++;
                    break;
                case 14://- 8
                    event_routine(3);
                    amount = amount * 10 + 8;
                    amount_count++;
                    break;
                case 15://- 0
                    if(amount)
                    {
                        event_routine(3);
                        amount = amount * 10 + 0;
                        amount_count++;
                    }
                    break;
                case 16://- 3
                    event_routine(3);
                    amount = amount * 10 + 3;
                    amount_count++;
                    break;
                case 17://- 6
                    event_routine(3);
                    amount = amount * 10 + 6;
                    amount_count++;
                    break;
                case 18://- 9
                    event_routine(3);
                    amount = amount * 10 + 9;
                    amount_count++;
                    break;
                case 19://- Clear
                    event_routine(4);
                    amount = 0;
                    amount_count = 0;
                    convert.str("");
                    amount_live_txt = convert.str();
                    break;
                case 20://- OK
                    if(amount)
                    {
                        event_routine(4);
                        amount_count = 0;
                        if(amount <= client.at(0).balance)
                            scr_state = 5;
                        else
                        {
                            scr_state = 10;
                            amount = 0;
                        }
                        amount_live_txt = "";
                        convert.str("");
                    }
                    break;
                }
            }
            if(amount_count == 7)
            {
                switch(get_mouse_input())
                {
                case 19://- Clear
                    event_routine(4);
                    amount = 0;
                    amount_count = 0;
                    convert.str("");
                    amount_live_txt = convert.str();
                    break;
                case 20://- OK
                    event_routine(4);
                    amount_count = 0;
                    if(amount <= client.at(0).balance)
                        scr_state = 5;
                    else
                    {
                        scr_state = 10;
                        amount = 0;
                    }
                    amount_live_txt = "";
                    convert.str("");
                    break;
                }
            }
            convert.str("");
            convert << amount;
            amount_live_txt = convert.str();
            break;
        case 5: //- (5) Confirm (Withdraw)
            switch(get_mouse_input())
            {
            case 1: //- Yes (L1)
                event_routine(4);
                scr_state = 6;
                break;
            case 7: //- No (R3)
                event_routine(4);
                scr_state = 4;
                amount = 0; amount_count = 0;
                amount_live_txt = "";
                convert.str("");
                break;
            }
            break;
        case 6: //- (6) Processing (Withdraw)
            client.at(0).balance = client.at(0).balance - amount;
            oss << get_time_cli() << client.at(0).last_name << " " << client.at(0).first_name << " withdrew " << amount << " RON"; log_out(oss.str());
            amount = 0; amount_count = 0;
            amount_live_txt = "";
            convert.str("");
            event_routine(5);
            scr_state = 7;
            break;
        case 7: //- (7) Receipt? (Withdraw)
            switch(get_mouse_input())
            {
            case 1: //- Yes (L1)
                if(!cash_large_visible)
                {
                    event_routine(4);
                    event_routine(7);
                    scr_state = 8;
                }
                break;
            case 7: //- No (R3)
                if(!cash_large_visible)
                {
                    event_routine(4);
                    scr_state = 8;
                }
                break;
            case 22: //- Cash Large
                cash_large_visible = false;
                break;
            }
            break;
        case 8: //- (8) Another transaction? (Withdraw)
            switch(get_mouse_input())
            {
            case 1: //- Yes (L1)
                if(!receipt_visible)
                {
                    event_routine(4);
                    scr_state = 3;
                }
                break;
            case 7: //- No (R3)
                if(!receipt_visible)
                {
                    if(!card_visible)
                    {
                        event_routine(4);
                        oss << get_time_cli() << client.at(0).last_name << " " << client.at(0).first_name << " finished the session"; log_out(oss.str());
                        event_routine(2);
                    }
                    declicked = true;
                }
                break;
            case 24: //- Receipt
                receipt_visible = false;
                break;
            }
            break;
        case 10: //- (10) Not Enough Funds
            switch(get_mouse_input())
            {
            case 7:
                event_routine(4);
                scr_state = 4;
                break;
            }
            break;
        case 11: //- (11) Enter Amount (Deposit)
            if(amount_count < 7)
            {
                switch(get_mouse_input())
                {
                case 9: //- 1
                    event_routine(3);
                    amount = amount * 10 + 1;
                    amount_count++;
                    break;
                case 10://- 4
                    event_routine(3);
                    amount = amount * 10 + 4;
                    amount_count++;
                    break;
                case 11://- 7
                    event_routine(3);
                    amount = amount * 10 + 7;
                    amount_count++;
                    break;
                case 12://- 2
                    event_routine(3);
                    amount = amount * 10 + 2;
                    amount_count++;
                    break;
                case 13://- 5
                    event_routine(3);
                    amount = amount * 10 + 5;
                    amount_count++;
                    break;
                case 14://- 8
                    event_routine(3);
                    amount = amount * 10 + 8;
                    amount_count++;
                    break;
                case 15://- 0
                    if(amount)
                    {
                        event_routine(3);
                        amount = amount * 10 + 0;
                        amount_count++;
                    }
                    break;
                case 16://- 3
                    event_routine(3);
                    amount = amount * 10 + 3;
                    amount_count++;
                    break;
                case 17://- 6
                    event_routine(3);
                    amount = amount * 10 + 6;
                    amount_count++;
                    break;
                case 18://- 9
                    event_routine(3);
                    amount = amount * 10 + 9;
                    amount_count++;
                    break;
                case 19://- Clear
                    event_routine(4);
                    amount = 0;
                    amount_count = 0;
                    convert.str("");
                    amount_live_txt = convert.str();
                    break;
                case 20://- OK
                    if(amount)
                    {
                        event_routine(4);
                        amount_count = 0;
                        scr_state = 12;
                        amount_live_txt = "";
                        convert.str("");
                    }
                    break;
                }
            }
            if(amount_count == 7)
            {
                switch(get_mouse_input())
                {
                case 19://- Clear
                    event_routine(4);
                    amount = 0;
                    amount_count = 0;
                    convert.str("");
                    amount_live_txt = convert.str();
                    break;
                case 20://- OK
                    event_routine(4);
                    amount_count = 0;
                    scr_state = 12;
                    amount_live_txt = "";
                    convert.str("");
                    break;
                }
            }
            convert.str("");
            convert << amount;
            amount_live_txt = convert.str();
            break;
        case 12: //- Confirm (Deposit)
            switch(get_mouse_input())
            {
            case 1: //- Yes (L1)
                event_routine(4);
                scr_state = 13;
                break;
            case 7: //- No (R3)
                event_routine(4);
                scr_state = 11;
                amount = 0; amount_count = 0;
                amount_live_txt = "";
                convert.str("");
                break;
            }
            break;
        case 13: //- Insert Cash
            cash_small_visible = true;
            switch(get_mouse_input())
            {
            case 23:
                event_routine(6);
                scr_state = 24;
                break;
            }
            break;
        case 14: //- Receipt? (Deposit)
            switch(get_mouse_input())
            {
            case 1: //- Yes (L1)
                if(!cash_small_visible)
                {
                    event_routine(4);
                    event_routine(7);
                    scr_state = 15;
                }
                break;
            case 7: //- No (R3)
                if(!cash_small_visible)
                {
                    event_routine(4);
                    scr_state = 15;
                }
                break;
            }
        case 15: //- Another transaction? (Deposit)
            switch(get_mouse_input())
            {
            case 1: //- Yes (L1)
                if(!receipt_visible)
                {
                    event_routine(4);
                    scr_state = 3;
                }
                break;
            case 7: //- No (R3)
                if(!receipt_visible)
                {
                    if(!card_visible)
                    {
                        event_routine(4);
                        oss << get_time_cli() << client.at(0).last_name << " " << client.at(0).first_name << " finished the session"; log_out(oss.str());
                        event_routine(2);
                    }
                    declicked = true;
                }
                break;
            case 24: //- Receipt
                receipt_visible = false;
                break;
            }
            break;
        case 17: //- Processing (Account Balance)
            sf::sleep(card_snd_buf.getDuration());
            oss << get_time_cli() << client.at(0).last_name << " " << client.at(0).first_name << "'s balance is: " << client.at(0).balance << " RON"; log_out(oss.str());
            amount = 0; amount_count = 0;
            amount_live_txt = "";
            convert.str("");
            scr_state = 18;
            break;
        case 18: //- Balance = ***. Receipt?
            switch(get_mouse_input())
            {
            case 1: //- Yes (L1)
                event_routine(4);
                event_routine(7);
                scr_state = 19;
                break;
            case 7: //- No (R3)
                event_routine(4);
                scr_state = 19;
                break;
            }
            balance.str("");
            balance << client.at(0).balance << " RON";
            amount_live_txt = balance.str();
            break;
        case 19: //- Another Transaction? (Account Balance)
            switch(get_mouse_input())
            {
            case 1: //- Yes (L1)
                if(!receipt_visible)
                {
                    event_routine(4);
                    scr_state = 3;
                }
                break;
            case 7: //- No (R3)
                if(!receipt_visible)
                {
                    if(!card_visible)
                    {
                        event_routine(4);
                        oss << get_time_cli() << client.at(0).last_name << " " << client.at(0).first_name << " finished the session"; log_out(oss.str());
                        event_routine(2);
                    }
                    declicked = true;
                }
                break;
            case 24: //- Receipt
                receipt_visible = false;
                break;
            }
            break;
        case 21: //- (21) Wrong PIN
            if(get_mouse_input() == 20)
            {
                event_routine(4);
                scr_state = 2;
            }
            break;
        case 22: //- (22) Account suspended
            blocked = true;
            if(!flag)
            {
                oss << get_time_cli() << "ACCOUNT SUSPENDED"; log_out(oss.str());
                flag = true;
            }
            if(get_mouse_input() == 20)
            {
                event_routine(4);
                event_routine(2);
            }
            break;
        case 23: //- (23) Processing (for card in)
            sf::sleep(card_snd_buf.getDuration());
            if(!blocked)
                scr_state = 2;
            else
                scr_state = 22;
            break;
        case 24: //- (24) Processing (deposit)
            client.at(0).balance = client.at(0).balance + amount;
            oss << get_time_cli() << client.at(0).last_name << " " << client.at(0).first_name << " deposited " << amount << " RON"; log_out(oss.str());
            sf::sleep(card_snd_buf.getDuration());
            amount = 0; amount_count = 0;
            amount_live_txt = "";
            convert.str("");
            scr_state = 14;
            break;
        }
    }

    void render(sf::RenderWindow &window)
    {
        window.clear();

        window.draw(backgnd_sprite);
        if(card_visible)
            window.draw(card_sprite);
        if(cash_large_visible)
            window.draw(cash_large_sprite);
        if(cash_small_visible)
            window.draw(cash_small_sprite);
        if(receipt_visible)
            window.draw(receipt_sprite);
        scr_render();

        window.display();
    }

    void scr_render()
    {
        //- Show Live "OK" Instruction
        if(pin_count == 4 || amount_count == 7)
        {
            R3_txt.setPosition(350, 200);
            R3_txt.setString("Apasati OK");
            R3_txt.setCharacterSize(18);
            R3_txt.setColor(sf::Color::Yellow);
            R3_txt.setStyle(sf::Text::Bold);
            window.draw(R3_txt);
        }

        //- Screen Clock Text Setup
        scr_clock.setPosition(490, 25);
        scr_clock.setString(get_time_gui());
        scr_clock.setCharacterSize(13);
        scr_clock.setColor(sf::Color::Red);
        scr_clock.setStyle(sf::Text::Bold);
        window.draw(scr_clock);

        //- Client Name and IBAN Text Setup
        if(scr_state != 1 && scr_state != 2 && scr_state != 21 && scr_state != 22 && scr_state != 23)
        {
            username_scr.setPosition(85, 25);
            username_scr.setString(username_scr_str.str());
            username_scr.setCharacterSize(13);
            username_scr.setColor(sf::Color::Cyan);
            window.draw(username_scr);

            iban_scr.setPosition(85, 290);
            iban_scr.setString(iban_scr_str.str());
            iban_scr.setCharacterSize(13);
            iban_scr.setColor(sf::Color::White);
            window.draw(iban_scr);
        }

        //- Processing
        if(scr_state == 23 || scr_state == 17 || scr_state == 6 || scr_state == 24)
        {
            R3_txt.setPosition(250, 200);
            R3_txt.setString("In curs de procesare...");
            R3_txt.setCharacterSize(20);
            R3_txt.setColor(sf::Color::Red);
            R3_txt.setStyle(sf::Text::Bold);
            window.draw(R3_txt);
        }

        //- Receipt?
        if(scr_state == 7 || scr_state == 14 || scr_state == 18)
        {
            if(scr_state == 18)
            {
                live_txt.setPosition(280, 150);
                live_txt.setString(amount_live_txt);
                live_txt.setCharacterSize(23);
                live_txt.setColor(sf::Color::White);
                live_txt.setStyle(sf::Text::Bold);
                window.draw(live_txt);
            }
            dialog.setPosition(90, 50);
            dialog.setString("Doriti bonul aferent tranzactiei?");
            dialog.setCharacterSize(22);
            dialog.setColor(sf::Color::Green);
            dialog.setStyle(sf::Text::Bold);
            window.draw(dialog);
            L1_txt.setPosition(85, 130);
            L1_txt.setString("<--- Da");
            L1_txt.setCharacterSize(20);
            L1_txt.setColor(sf::Color::White);
            L1_txt.setStyle(sf::Text::Bold);
            window.draw(L1_txt);
            R3_txt.setPosition(465, 225);
            R3_txt.setString("Nu --->");
            R3_txt.setCharacterSize(20);
            R3_txt.setColor(sf::Color::White);
            R3_txt.setStyle(sf::Text::Bold);
            window.draw(R3_txt);
        }

        //- Confirm?
        if(scr_state == 5 || scr_state == 12)
        {
            dialog.setPosition(255, 50);
            dialog.setString("Confirmare");
            dialog.setCharacterSize(22);
            dialog.setColor(sf::Color::Green);
            dialog.setStyle(sf::Text::Bold);
            window.draw(dialog);
            L1_txt.setPosition(85, 130);
            L1_txt.setString("<--- Da");
            L1_txt.setCharacterSize(20);
            L1_txt.setColor(sf::Color::White);
            L1_txt.setStyle(sf::Text::Bold);
            window.draw(L1_txt);
            R3_txt.setPosition(465, 225);
            R3_txt.setString("Nu --->");
            R3_txt.setCharacterSize(20);
            R3_txt.setColor(sf::Color::White);
            R3_txt.setStyle(sf::Text::Bold);
            window.draw(R3_txt);
        }

        //- Another Transaction?
        if(scr_state == 8 || scr_state == 15 || scr_state == 19)
        {
            dialog.setPosition(200, 50);
            dialog.setString("Doriti sa efectuati\no noua tranzactie?");
            dialog.setCharacterSize(22);
            dialog.setColor(sf::Color::Green);
            dialog.setStyle(sf::Text::Bold);
            window.draw(dialog);
            L1_txt.setPosition(85, 130);
            L1_txt.setString("<--- Da");
            L1_txt.setCharacterSize(20);
            L1_txt.setColor(sf::Color::White);
            L1_txt.setStyle(sf::Text::Bold);
            window.draw(L1_txt);
            R3_txt.setPosition(465, 225);
            R3_txt.setString("Nu --->");
            R3_txt.setCharacterSize(20);
            R3_txt.setColor(sf::Color::White);
            R3_txt.setStyle(sf::Text::Bold);
            window.draw(R3_txt);
        }

        //- Enter amount
        if(scr_state == 4 || scr_state == 11)
        {
            dialog.setPosition(210, 50);
            dialog.setString("Introduceti suma");
            dialog.setCharacterSize(22);
            dialog.setColor(sf::Color::Green);
            dialog.setStyle(sf::Text::Bold);
            window.draw(dialog);
            pin_border_shape.setPosition(230, 150);
            pin_border_shape.setSize(sf::Vector2f(180, 30));
            pin_border_shape.setFillColor(sf::Color::Black);
            pin_border_shape.setOutlineColor(sf::Color::White);
            pin_border_shape.setOutlineThickness(2);
            window.draw(pin_border_shape);
            live_txt.setPosition(270, 150);
            live_txt.setString(amount_live_txt);
            live_txt.setCharacterSize(23);
            live_txt.setColor(sf::Color::White);
            live_txt.setStyle(sf::Text::Bold);
            window.draw(live_txt);
            R3_txt.setPosition(425, 150);
            R3_txt.setString("RON");
            R3_txt.setCharacterSize(23);
            R3_txt.setColor(sf::Color::White);
            R3_txt.setStyle(sf::Text::Bold);
            window.draw(R3_txt);
        }

        //- Main Screen Setup
        switch(scr_state)
        {
        case 1:
            dialog.setPosition(180, 50);
            dialog.setString("    Bun venit!\nIntroduceti cardul");
            dialog.setCharacterSize(24);
            dialog.setColor(sf::Color::Green);
            dialog.setStyle(sf::Text::Bold);
            window.draw(dialog);
            break;
        case 2:
            dialog.setPosition(170, 50);
            dialog.setString("Introduceti codul PIN");
            dialog.setCharacterSize(22);
            dialog.setColor(sf::Color::Green);
            dialog.setStyle(sf::Text::Bold);
            window.draw(dialog);
            amount_border_shape.setPosition(230, 150);
            amount_border_shape.setSize(sf::Vector2f(180, 30));
            amount_border_shape.setFillColor(sf::Color::Black);
            amount_border_shape.setOutlineColor(sf::Color::White);
            amount_border_shape.setOutlineThickness(2);
            window.draw(amount_border_shape);
            switch(pin_count)
            {
            case 0:
                pin_live_txt = "";
                break;
            case 1:
                pin_live_txt = "*";
                break;
            case 2:
                pin_live_txt = "**";
                break;
            case 3:
                pin_live_txt = "***";
                break;
            case 4:
                pin_live_txt = "****";
                break;
            }
            live_txt.setPosition(290, 150);
            live_txt.setString(pin_live_txt);
            live_txt.setCharacterSize(25);
            live_txt.setColor(sf::Color::White);
            live_txt.setStyle(sf::Text::Bold);
            window.draw(live_txt);
            break;
        case 3:
            L1_txt.setPosition(85, 130);
            L1_txt.setString("<--- Retragere");
            L1_txt.setCharacterSize(20);
            L1_txt.setColor(sf::Color::White);
            L1_txt.setStyle(sf::Text::Bold);
            window.draw(L1_txt);
            R1_txt.setPosition(390, 130);
            R1_txt.setString("Depunere --->");
            R1_txt.setCharacterSize(20);
            R1_txt.setColor(sf::Color::White);
            R1_txt.setStyle(sf::Text::Bold);
            window.draw(R1_txt);
            R3_txt.setPosition(300, 225);
            R3_txt.setString("Interogare Sold --->");
            R3_txt.setCharacterSize(20);
            R3_txt.setColor(sf::Color::White);
            R3_txt.setStyle(sf::Text::Bold);
            window.draw(R3_txt);
            break;
        case 10:
            dialog.setPosition(210, 50);
            dialog.setString("Sold insuficient");
            dialog.setCharacterSize(22);
            dialog.setColor(sf::Color::Green);
            dialog.setStyle(sf::Text::Bold);
            window.draw(dialog);
            R3_txt.setPosition(300, 225);
            R3_txt.setString("Modificati suma --->");
            R3_txt.setCharacterSize(20);
            R3_txt.setColor(sf::Color::White);
            R3_txt.setStyle(sf::Text::Bold);
            window.draw(R3_txt);
            break;
        case 13:
            dialog.setPosition(120, 50);
            dialog.setString("Plasati numerarul in bancomat");
            dialog.setCharacterSize(22);
            dialog.setColor(sf::Color::Green);
            dialog.setStyle(sf::Text::Bold);
            window.draw(dialog);
            break;
        case 21:
            dialog.setPosition(110, 50);
            dialog.setString("Ati introdus un PIN incorect\n        OK | Cancel?");
            dialog.setCharacterSize(24);
            dialog.setColor(sf::Color::Green);
            dialog.setStyle(sf::Text::Bold);
            window.draw(dialog);
            break;
        case 22:
            dialog.setPosition(105, 50);
            dialog.setString("3 incercari succesive eronate\n  Contul dvs este suspendat\n      Apasati tasta OK");
            dialog.setCharacterSize(24);
            dialog.setColor(sf::Color::Green);
            dialog.setStyle(sf::Text::Bold);
            window.draw(dialog);
        }
    }

    void event_routine(unsigned short int routine)
    {
        //=======================
        //Routine Codes (routine)
        //=======================
        //1 = Card In
        //2 = Card Out
        //3 = Key Sound
        //4 = Menu Sound
        //5 = Cash Large Out
        //6 = Cash Small In
        //7 = Receipt Out
        //======================
        //======================

        switch(routine)
        {
        case 1:
            flag = false;
            card_visible = false;
            card_snd.play();
            render(window);
            sf::sleep(card_snd_buf.getDuration());
            oss << get_time_cli() << "The cardholder inserted a VISA Classic Card"; log_out(oss.str());
            break;
        case 2:
            if(scr_state != 1)
            {
                card_snd.play();
                render(window);
                sf::sleep(card_snd_buf.getDuration());
                oss << get_time_cli() << "The card was ejected"; log_out(oss.str());
            }
            init_states();
            break;
        case 3:
            key_snd.play();
            break;
        case 4:
            menu_snd.play();
            break;
        case 5:
            cash_snd.play();
            sf::sleep(cash_snd_buf.getDuration());
            cash_large_visible = true;
            break;
        case 6:
            if(scr_state == 13)
            {
                cash_small_visible = false;
                cash_snd.play();
                render(window);
                sf::sleep(cash_snd_buf.getDuration());
            }
            break;
        case 7:
            print_receipt_snd.play();
            sf::sleep(print_receipt_snd_buf.getDuration());
            receipt_visible = true;
            break;
        }
    }

    void load_clients()
    {
        User u;
        int nr, i;
        database >> nr;
        for(i = 0; i < nr; i++)
        {
            database >> u.iban >> u.last_name >> u.first_name >> u.pin >> u.balance;
            client.push_back(u);
        }
        username_scr_str << client.at(0).last_name << " " << client.at(0).first_name;
        iban_scr_str << client.at(0).iban;
    }

    std::string program_title()
    {
        std::ostringstream stream;
        stream << title << " | v" << ver;
        return stream.str();
    }

    std::string get_time_cli()
    {
        SYSTEMTIME systime;
        GetLocalTime(&systime);
        char str[21];
        sprintf(str, "%02d-%02d-%04d | %02d:%02d:%02d --> ", systime.wDay, systime.wMonth, systime.wYear, systime.wHour, systime.wMinute, systime.wSecond);
        return str;
    }

    std::string get_time_gui()
    {
        SYSTEMTIME systime;
        GetLocalTime(&systime);
        char str[8];
        sprintf(str, "%02d:%02d:%02d", systime.wHour, systime.wMinute, systime.wSecond);
        return str;
    }

    std::string get_name_log()
    {
        SYSTEMTIME systime;
        GetLocalTime(&systime);
        char str[28];
        sprintf(str, "res/logs/log-%04d.%02d.%02d-%02d.%02d.%02d.txt", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);
        return str;
    }

    void log_out(std::string str)
    {
        std::cout << str << std::endl;
        log << str << std::endl;
        oss.str("");
        oss.clear();
    }

    void terminate()
    {
        oss << get_time_cli() << "The ATM is now powered off"; log_out(oss.str());
        database.close();
        log.close();
        system("pause");
    }

public:
    void run()
    {
        //- Global Initialization
        init();
        while(window.isOpen())
        {
            //- Handle events
            handle_events(event);
            //- Update
            update();
            //- Render
            render(window);
        }
        //- Terminate process
        terminate();
    }
};

int main()
{
    Atm atm;
    atm.run();
    return 0;
}
