#ifndef CUNO_H
#define CUNO_H

//total num of cards -> pre 2018 type of deck
#define DECK_SIZE 108
//how many colors
#define COLOR_SIZE 13

//rand bite shift
#define RAND_M ((1U << 31) - 1)

//standard size of hand
#define HAND_SIZE 7
//max pre allocated size of hand -> TODO better solution need, no check performed yet
#define HAND_CAP HAND_SIZE*HAND_SIZE

//to keep malloc operations simple for now
#define MAX_PLAYERS 8

//default players
#define DEFAULT_PLAYERS 4
#define DEFAULT_ACTIVE 1


//helper 
extern int DEBUG;

//rand functions for shuffling the cards
extern int rand();
extern void seed();
extern int rseed;


//available colors
typedef enum COLOR {
    BLUE,
    YELLOW,
    GREEN,
    RED,
    BLACK,
    COLORS
} COLOR;

//action cards
typedef enum ACTION_CARDS {
    NEXT_PAUSE = 10,
    REVERSE,
    DRAW_TWO,
    CHOOSE_COLOR,
    DRAW_FOUR,
} ACTION_CARDS;


//enum for locations, that is, where are card is located curenty
typedef enum LOCATION {
    DECK,
    DISCARD,
    HAND,
} LOCATION;

typedef enum DIRECTION {
    CLOCKWISE,
    COUNTERCLOCKWISE
} DIRECTION;


//ids for actions
typedef enum ACTION {
    NEXT_PLAYER,
    NEXT_DRAW2,
    NEXT_DRAW4,
    NEXT_SKIP,
    CHANGE_DIR,
    SET_COLOR,
} ACTION;


//card representation
typedef struct Card {
    LOCATION location;
    int player_id;
    COLOR color;
    int value; 
    int max_len;
} Card;


//a players hand
typedef struct Hand {
    int player_id;
    int num_cards;
    int is_active;
    Card* cards[HAND_CAP];
} Hand;

//wrapper for discards
typedef struct Discards {
    Card* cards[DECK_SIZE];
    int len;
} Discards;

//table = game state
typedef struct Table {
    int num_players; //total number of players
    DIRECTION dir;
    int top_of_deck;
    int in_turn; //player whose turn it currently is
    int current_player;
    Hand* current_hand;
    COLOR current_color;
    int current_value;
    Discards* discarded;
    Hand* player_hands[MAX_PLAYERS];
} Table;


//////////// UTILS
//sorting
void insertion_sort_cards(Card* cards[], int n);

void print_delim();

//deallocates memory
void free_table(Table * table);
//enum int to strings
char* print_color(COLOR color); 
char* print_value(int value);
char* print_location(int loc);

//prints card, i.e., color and value formatted. 
//if indent is set to one, one tab characters is printed 
//with each line
void print_card(Card* card, int indent);

//iterates through the whole deck and prints the cards
void print_deck();

//prints hand of player if suggestion is set to 1, 
//an arrow will appear with the best possible card
void print_player_hand(Table* table, int suggestion);

//prints hand of player current player
void print_hand(Hand *hand);

//prints the current card on top of the discard stack
void print_discard(Table* table);

//prints state of game for active player
void print_state_for_player(Table* table, int player_id);

//DEBUG helper
void print_action(ACTION act);

//////// GAME PLAY /////
//initializes deck
void construct_deck();

//initializes discard stack
Discards* init_discards();

//shuffles deck
void shuffle_deck();

//deals hands 
void deal_hands();

//draw to discard opens the first card on the discard stack, should be called right after hands are dealt
void draw_to_discard(Table* table);

//initializes the game round, ignore actives for now
//it constructs the deck, shuffles it, deals hands and draws to discard, as well as sets the current player and his/her hand
Table set_table(int num_players, int actives);

//helper to get idx
int get_card_idx_from_hand(Hand* hand, Card* card_ptr);

//checks the highest legal discard in current players hand and returns its index, if none is found, the length of the hand is returned
int player_next_best_card_idx(Table* table); 

//returns ptr to hand of player id provided
Hand* get_player_hand(Table* table, int player_id);

//sets hand on table of current player
void set_current_hand(Table* table);

//acts for reverse card and changes play DIRECTION
//where  CLOCKWISE is ascending and COUNTERCLOCKWISE descending indices
void change_direction(Table* table);

//set current player to next player in DIRECTION
void set_current_player(Table* table); 

//draws card from deck
//and will initialize the reshuffling of discards !! NOT IMPLEMENTED YET !!
Card* get_next_card(Table* table);

//shifts cards after the one that was discarded from hand
void shift_cards(Hand* hand, int card_idx);

//draws card from deck to provided player hand
Card* draw_to_player(Table* table, int player_id);

//player discards by index
int player_discard(Table* table, int player_id, int card_idx);

//takes the last card drawn and discards it right away
void player_discard_drawn(Table* table, int player_id);

//checks the last card on the discard stack and determineshow the game proceeds
ACTION determine_action(Table* table);

//retruns the index of a card within a hand
int get_card_idx(Table* table, Card* card_ptr);

//checks if current player has same value in hand, if no,
//returns num_cards otherwise index of first matching card
int has_card_value(Hand *hand, int value);

//checks if player can stack, mostly for use of draw2 and other action cards
int check_if_can_stack(Table *table);

//checks if a player has only one card left
//and prints 'uno' to the terminal
//returns player id
int is_uno(Table *table);
//checks if a player is out of hands
//then calculates his score
//returs player id
int player_has_won(Table *table);

//calculates the scores of the 'losers'
int calculate_hands(Table *table, int winner);


//////BOT//////

//check if bot hand holds zero card, as they are the hardest to get rid of
int hand_has_zero(Hand* hand); 


// determines best next move for bot player
void bot_best_next_card(Table* table, int player_id, int no_draw);

//determines best next color for bot player by a simple count of most colors
void bot_best_next_color(Table* table);


#endif /* CUNO_H */
