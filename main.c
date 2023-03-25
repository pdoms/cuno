#include<stdio.h>
#include<stdlib.h>

#include<string.h>
#include<time.h>

#define DEBUG 1

#define DECK_SIZE 108
#define COLOR_SIZE 13
#define RAND_M ((1U << 31) - 1)
#define NUM_PLAYER 4
#define HAND_SIZE 7
#define HAND_CAP HAND_SIZE*HAND_SIZE
#define MAX_PLAYERS 8
#define FLAG_CAP 8 

#define DEFAULT_PLAYERS 4
#define DEFAULT_ACTIVE 1

int rand();
void seed();
int rseed = 0;


inline void seed(int x) {
    rseed = x;
}

inline int rand() {
    return rseed = (rseed * 1103515245 + 12345) & RAND_M;
}


typedef enum COLOR {
    BLUE,
    YELLOW,
    GREEN,
    RED,
    BLACK,
    COLORS
} COLOR;

typedef enum ACTION_CARDS {
    NEXT_PAUSE = 10,
    REVERSE,
    DRAW_TWO,
    CHOOSE_COLOR,
    DRAW_FOUR,
} ACTION_CARDS;

typedef enum LOCATION {
    DECK,
    DISCARD,
    HAND,
} LOCATION;

typedef enum DIRECTION {
    CLOCKWISE,
    COUNTERCLOCKWISE
} DIRECTION;

typedef enum ACTION {
    NEXT_PLAYER,
    NEXT_DRAW2,
    NEXT_DRAW4,
    NEXT_SKIP,
    CHANGE_DIR,
    SET_COLOR,
} ACTION;



typedef struct Card {
    LOCATION location;
    int player_id;
    COLOR color;
    int value; 
    int max_len;
} Card;

Card deck[DECK_SIZE];
char* convert[10] = {"0","1","2","3","4","5","6","7","8","9" };

typedef struct Hand {
    int player_id;
    int num_cards;
    int is_active;
    Card* cards[HAND_CAP];
} Hand;


typedef struct Discards {
    Card* cards[DECK_SIZE];
    int len;
} Discards;

typedef struct Table {
    int num_players;
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


void construct_deck() {
    int i = 0;
    int cards = 0;
    for (i; i<COLORS-1; ++i) {
        int j = 1;
        for (j; j<COLOR_SIZE; ++j) {
             Card card = {
                .location = DECK,
                .player_id = 0,
                .color = i,
                .value = j,
            };
            int idx = (i*(COLOR_SIZE-1)) + j;
            int idx2 = (COLORS-1)*(COLOR_SIZE-1) + ((i*(COLOR_SIZE-1)) + j);
            deck[idx]= card;
            deck[idx2] = card;
            cards =  cards + 2;
        }
    }
    i = 0;
    for (i; i<COLORS-1; ++i) {
        cards++;
        Card zero = {
            .location = DECK,
            .player_id = 0,
            .color = i,
            .value = 0,
        };
        deck[cards] = zero;
    }
    i=0;
    for (i;i<8;++i) {
        cards++;
       if (i%2==0) {
        Card  wild = {
            .location = DECK,
            .player_id = 0,
            .color = BLACK,
            .value = CHOOSE_COLOR,
        };
        deck[cards] = wild;
       } else {
        Card wild = {
            .location = DECK,
            .player_id = 0,
            .color = BLACK,
            .value = DRAW_FOUR,
        };
        deck[cards] = wild;
       }
    }
}

char* print_color(COLOR color) {
    switch(color) {
        case BLUE:
            return "blue";
        case YELLOW:
            return "yellow";
        case GREEN:
            return "green";
        case RED: 
            return "red";
        case BLACK:
            return "wild";
        default:
            printf("Unknown color id %d\n", color);
            exit(1);
    }
}

char* print_value(int value) {
    switch(value) {

        case NEXT_PAUSE:
            return "skip";
        case REVERSE:
            return "reverse";
        case DRAW_TWO:
            return "draw2";
        case CHOOSE_COLOR: 
            return "color";
        case DRAW_FOUR:
            return "four";
        default:
            if (value < 10) {
                return convert[value];
            } else {
                printf("Unknown value id %d\n", value);
                exit(1);
            }
    }
}

char* print_location(int loc) {
    switch (loc) {
        case DECK:
            return "deck";
        case DISCARD:
            return "discard stack";
        case HAND:
            return "hand";
   };
}

void print_card(Card* card, int indent) {
    if (indent == 1) {
        printf("\t\tcolor: %s\n\t\tvalue: %s", print_color(card->color), print_value(card->value));
    } else {
        printf("color: %s\nvalue: %s", print_color(card->color), print_value(card->value));
    }
    if (DEBUG==1) {
        if (indent == 1) {
            printf("\n\t\tloc: %s\n\t\tplayer: %d\n", print_location(card->location), card->player_id);
        } else {
            printf("\nloc: %s\nplayer: %d\n", print_location(card->location), card->player_id);
        }
    } else {
        printf("\n");
    }
    if (indent == 1) {
        printf("\t\t");
    }
    printf("----------------\n");
}

void print_deck() {
    int i = 0;
    for (i; i < DECK_SIZE; ++i) {
        print_card(&deck[i], 0);
    }
}


void shuffle_deck() {
    printf("[DEBUG] shuffling deck\n");
    int i = DECK_SIZE - 1;
    int j;
    for (i; i >= 1; i--) {
        j = rand()%i;
        Card tmp = deck[j];
        deck[j] = deck[i];
        deck[i] = tmp;
    }
}

void deal_hands(Table *table) {
    printf("[DEBUG] dealing hands\n");
    int i = 0;
    for (i; i < table->num_players; i++) {
        int j = 0;
        for (j;j < HAND_SIZE;j++) {
            // j * NUM_PLAYERS + i 
            int deck_idx = j * table->num_players + i;
            deck[deck_idx].player_id = i+1;
            table->player_hands[i]->cards[j] = &deck[deck_idx];
            table->top_of_deck++;
        }
    }
}

void print_hand(Hand *hand) {
    printf("+++ Player %d's Hand", hand->player_id);
    if (hand->is_active == 1) {
        printf(" (active)\n");
    } else {
        printf("\n");
    }
    int i = 0;
    for (i;i<hand->num_cards;i++) {
        printf("\t#%d -> color: %s; value: %s\n", i+1, 
                print_color(hand->cards[i]->color), 
                print_value(hand->cards[i]->value));

    }
}

Discards* init_discards() {
    Discards* discarded = (Discards*)malloc(sizeof(Discards));
    discarded->len = 0;
    for (int i = 0; i < DECK_SIZE; i++) {
        discarded->cards[i] = NULL;
    }
    return discarded;
}

//should be called only once
void draw_to_discard(Table* table) {
    printf("[DEBUG] drawing initial discard\n");
    int top = table->discarded->len;
    table->discarded->cards[top] = &deck[table->top_of_deck]; 
    table->current_color = deck[table->top_of_deck].color;
    table->current_value = deck[table->top_of_deck].value;
    deck[table->top_of_deck].location = DISCARD;
    table->discarded->len++;
    table->top_of_deck++;
}
void print_discard(Table* table) {
    printf("\tDiscarded: \n");
    int top = table->discarded->len;
    print_card(table->discarded->cards[top-1], 1);
}

Table set_table(int num_players, int actives) {
    construct_deck();
    shuffle_deck();
    int j = 0;
    int offset = 0;
    Table table = {
        .num_players = num_players,
        .dir = CLOCKWISE,
        .top_of_deck = 0,
        .current_player = 1,
        .discarded = init_discards(),
        .in_turn = 1,
    };
    int k = 0;
    for (k; k < MAX_PLAYERS; k++) {
        if (k < num_players) {
            Hand* hand = (Hand*)malloc(sizeof(Hand) + sizeof(Card*)*HAND_CAP);
            hand->player_id = k+1;
            hand->num_cards = HAND_SIZE;
            hand->is_active = k < actives;
            table.player_hands[k] = hand;
            int m = 0;
            for (m;m<HAND_CAP;m++) {
                table.player_hands[k]->cards[m] = NULL;
            }
        } else {
            table.player_hands[k] = NULL;
        }
    }
    deal_hands(&table);
    draw_to_discard(&table);
    return table;
}


Hand* get_player_hand(Table* table, int player_id) {
    int i =0;
    for (i; i < table->num_players; i++) {
        if (player_id == table->player_hands[i]->player_id) {
            break;
        }
    }
    return table->player_hands[i];
}

void set_current_hand(Table* table) {
    int i =0;
    for (i; i < table->num_players; i++) {
        if (table->current_player == table->player_hands[i]->player_id) {
            table->current_hand = get_player_hand(table, table->player_hands[i]->player_id);
            break;
        }
    }
}

void change_direction(Table* table) {
    if (table->dir == CLOCKWISE) {
        table->dir = COUNTERCLOCKWISE;
    } else {
        table->dir = CLOCKWISE;
    }
}

void set_current_player(Table* table) {
    printf("changing from %d", table->current_player);
    if (table->dir == CLOCKWISE) {
        printf("DIRECTION is CLOCKWISE\n");
        if (table->current_player < table->num_players) {
            table->current_player = table->current_player + 1;
        } else {
            table->current_player = 1;
        }
    } else {
        printf("DIRECTION is COUNTERCLOCKWISE\n");
        printf("NUM PLAYERS %d", table->num_players);
        if (table->current_player == 1) {
            printf("SETTING TO LAST PLAYER\n");
            table->current_player = table->num_players;
        } else {
            table->current_player = table->current_player-1;
        }
    }
    printf(" to %d\n", table->current_player);
}

Card* get_next_card(Table* table) {
    Card* card = &deck[table->top_of_deck];
    table->top_of_deck++;
    return card;
}

void shift_cards(Hand* hand, int card_idx) {
    int i = card_idx+1;
    for (i; i<hand->num_cards;i++) {
        hand->cards[i-1] = hand->cards[i];
    }
    hand->cards[hand->num_cards] = NULL;
    hand->num_cards--;
}

Card* draw_to_player(Table* table, int player_id) {
    printf("[DEBUG] drawing card for player %d\n", player_id);
    Hand* hand = get_player_hand(table, player_id);
    Card* card = get_next_card(table);
    card->player_id = player_id;
    hand->cards[hand->num_cards] = card;
    hand->num_cards++;
    return card;
}

void player_discard(Table* table, int player_id, int card_idx) {
    Hand* hand = get_player_hand(table, player_id);
    int top = table->discarded->len;
    table->discarded->cards[top] = hand->cards[card_idx]; 
    table->current_color = table->discarded->cards[top]->color;
    table->current_value = table->discarded->cards[top]->value;
    deck[table->top_of_deck].location = DISCARD;
    table->discarded->len++;
    table->top_of_deck++;
    shift_cards(hand, card_idx);
}


void player_discard_drawn(Table* table, int player_id) {
    Hand* hand = get_player_hand(table, player_id);
    int card_idx = hand->num_cards - 1;
    int top = table->discarded->len;
    table->discarded->cards[top] = hand->cards[card_idx]; 
    table->current_color = table->discarded->cards[top]->color;
    table->current_value = table->discarded->cards[top]->value;
    deck[table->top_of_deck].location = DISCARD;
    table->discarded->len++;
    table->top_of_deck++;
    shift_cards(hand, card_idx);
}

void print_state_for_player(Table* table, int player_id) {
    print_hand(table->player_hands[player_id-1]);
    print_discard(table);
}

ACTION determine_action(Table* table) {
    int top = table->discarded->len;
    Card* card = table->discarded->cards[top-1];
    if (card->value >= 10) {
        switch (card->value) {
            case NEXT_PAUSE:
                return NEXT_SKIP;
            case REVERSE:
                return CHANGE_DIR;
            case DRAW_TWO:
                return NEXT_DRAW2;
            case CHOOSE_COLOR:
                return SET_COLOR;
            case DRAW_FOUR:
                return NEXT_DRAW4;
        }
    } else {
        return NEXT_PLAYER;
    }
    print_card(card, 0); 
}

int hand_has_zero(Hand* hand) {
    int i = 0;
    for (i;i<hand->num_cards;i++) {
        if (hand->cards[i]->value == 0) {
            return i;
        }
    }
    // returns a num cards, if not found
    return hand->num_cards;
}

void insertion_sort_cards(Card* cards[], int n) {
    for (int i = 1; i < n; i++) {
        Card* tmp = cards[i];
        int j = i - 1;
        while (tmp->value < cards[j]->value && j >= 0) {
            cards[j + 1] = cards[j];
        --j;
        }
        cards[j + 1] = tmp;
    }
}

int get_card_idx(Table* table, Card* card_ptr) {
    Hand* hand = table->current_hand;
    int i = 0;
    for (i; i < hand->num_cards; i++) {
        if (card_ptr == hand->cards[i]) {
            return i;         
        }
    }
    return hand->num_cards;
}

void bot_best_next_card(Table* table, int player_id) {
    int max_len = table->current_hand->num_cards;
    //strategy 1 -> check if discard is zero and hand has zero
    if (table->current_value == 0) {
        int zero = hand_has_zero(table->current_hand);
        if (zero < max_len) {
            player_discard(table, player_id, zero);
            return;
        }
    }
    
    Hand* hand = table->current_hand;
    Card* possible_cards[max_len];
    //possible_cards = (Card*)calloc(max_len, sizeof(Card*));
    int i = 0;
    int size = 0;
    for (i; i < max_len; i++) {
        Card* card = hand->cards[i];
        if (card->color == table->current_color 
                || card->value == table->current_value) {
            possible_cards[size] = card;
            size++;
        }
    }

    print_hand(hand);
    if (size > 0) {
        printf("found these %d cards:\n", size);
        for (int j = 0; j < size; j++) {
            print_card(possible_cards[j], 0);
        }
    }   
    if (size == 1) {
        int idx = get_card_idx(table, possible_cards[0]);
        player_discard(table, table->current_player, idx);
        return;
    }

    if (size > 1) {
        printf("sorting...\n");
        insertion_sort_cards(possible_cards, size);
        print_card(possible_cards[size-1], 0);
        int idx = get_card_idx(table, possible_cards[size-1]);
        printf("idx %d\n", idx);
        player_discard(table, table->current_player, idx);
        return;
        
    }

    if (size == 0) {
        draw_to_player(table, table->current_player);
    }
    




    
    //print_card(possible_cards[size], 0);



    //strategy 2 -> check if a color matches
    //strategy 3 -> check if a value matches
    //strategy 4 -> check if black card in hand
    //  yes -> since no color in hand, draw4 is ok -> draw4 over choose color
    //  no -> draw card
    //NOTE -> these are merely the rules rather than strategic decisions. 
    //The latter which are a TODO
    


    //print UNO -> TODO add a ten percent chance or so that bot forgets to 
    //announce uno
}


void bot_best_next_color(Table* table) {
    int len = table->current_hand->num_cards;
    int clrs[COLORS] = {0};
    int i = 0;
    for (i; i < len; i++) {
        COLOR clr = table->current_hand->cards[i]->color;
        clrs[clr] = clrs[clr] + 1;
    }
    i = 0;
    printf("PLAYER %d colors: \n", table->current_player);
    print_hand(table->current_hand);
    int max = 0;
    int next_clr = 0;
    for (i; i < COLORS; i++) {
        if clrs[i] > max {
            max = clrs[i];
            next_clr = i;
        }
        printf("\t%s occurs %d times\n", print_color(i), clrs[i]);
    }
    printf("would choose %s", print_color(next_clr));
}



void free_table(Table * table) {
    free(table->discarded);
    int i = 0;
    for (i; i < MAX_PLAYERS;i++) {
        free(table->player_hands[i]);
    }
}

void parse_arg(char* arg, char* flag, char* value) {
        int i =0;
        while (arg[i] != '=') {
            flag[i] = arg[i];
            i++;
        }
        value[0] = arg[i+1];
}

int streq(char* a, char* b) {
    int i = 0;
    while (1) {
        if (a[i] == '\0') {
            if (b[i] == '\0') {
                return 1;
            } else {
                return 0;
            }
        }
        if (a[i] != b[i]) {
            return 0;
        }
        i++;
    } 
}

void ask_for_color(Table* table) {
    printf("Player %d, please select a color!\n", table->current_player);
    printf("\t#1 -> BLUE\n");
    printf("\t#2 -> YELLOW\n");
    printf("\t#3 -> GREEN\n");
    printf("\t#4 -> RED\n");
    printf("-> ");
    int c;
    scanf("%d", &c);
    table->current_color = c-1;
    printf("Player %d chose: %s\n", table->current_player, print_color(c-1));
}

int c_to_int(char c) {
    return ((int)c - 48);
}

void print_delim() {
    printf("\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
}
void print_action(ACTION act) { 
    switch (act) {
        case NEXT_PLAYER:
            printf("NEXT PLAYER\n");
            break;
        case NEXT_DRAW2:
            printf("NEXT DRAW 2\n");
            break;
        case NEXT_DRAW4:
            printf("NEXT DRAW 4\n");
            break;
        case NEXT_SKIP:
            printf("NEXT SKIP\n");
            break;
        case CHANGE_DIR:
            printf("CHANGE DIR\n");
            break;
        case SET_COLOR:
            printf("SET_COLOR\n");
            break;
    }
}

int main(int argc, char **argv) {
    //TODO general todo, think of best way to sort a hand
    seed(time(NULL));
    //TODO before start determine who starts by drawing cards 
    int num_players = DEFAULT_PLAYERS;
    int num_active = DEFAULT_ACTIVE;
    char* PLAYER = "player";
    char* ACTIVE = "active";
    for (int i = 1; i < argc; ++i)
    {   
        char flag[FLAG_CAP];
        char value[2];
        parse_arg(argv[i], flag, value);
        if (streq(flag, PLAYER)) {
            num_players=c_to_int(value[0]);
        } else if (streq(flag, ACTIVE)) {
            num_active=c_to_int(value[0]);
        } else {
            printf("UNKNOWN ARGUMENT - %s\n", argv[i]);
        }
    }   
    printf("[DEBUG] NUM PLAYERS: %d\n", num_players);
    printf("[DEBUG] NUM ACTIVE: %d\n", num_active);
    Table table = set_table(num_players, num_active);
    int user_play_action;
    int prev_player = 0;
    Hand* previous_hand;
    Card* drawn_card;
    int run = 1;
    int next;
    int next_action;
    COLOR clr;
    int init = 1;
    while (run) {
        print_delim();
        print_discard(&table);
        next_action = determine_action(&table);
        print_action(next_action);
        switch (next_action) {
            case CHANGE_DIR:
                change_direction(&table);
                set_current_player(&table);
                break;
            case NEXT_SKIP:
                if (init != 1) {
                    set_current_player(&table);
                    init = 0;
                }
                set_current_player(&table);
                break;
            case SET_COLOR:
                if (table.current_hand->is_active == 1) {
                    ask_for_color(&table);
                } else {
                    bot_best_next_color(&table);
                }
                set_current_player(&table);
                break;
            //TODO next draw but can play afterwards and doesn't have to draw    
            case NEXT_DRAW2:
                set_current_player(&table);
                if (table.current_hand->is_active == 1) {
                    draw_to_player(&table, table.current_player);
                    draw_to_player(&table, table.current_player);
                } else {   
                    draw_to_player(&table, table.current_player);
                    draw_to_player(&table, table.current_player);
                }   
                break; 
            case NEXT_DRAW4:
                if (table.current_hand->is_active == 1) {
                    ask_for_color(&table), table.current_player;
                    set_current_player(&table);  
                    draw_to_player(&table, table.current_player);
                    draw_to_player(&table, table.current_player);
                    draw_to_player(&table, table.current_player);
                    draw_to_player(&table, table.current_player);
                } else {   
                    bot_best_next_color(&table); 
                    set_current_player(&table);
                    draw_to_player(&table, table.current_player);
                    draw_to_player(&table, table.current_player);
                    draw_to_player(&table, table.current_player);
                    draw_to_player(&table, table.current_player);
                }   
                break; 
            default:
                if (init == 1) {
                    init = 0;
                    break;
                }
                printf("GOING TO NEXT PLAYER\n");
                set_current_player(&table);
                break;
        }
        if (run == 0) {
            break;
        }
        //print_discard(&table);
        printf("Player %d's turn!\n", table.current_player);
        set_current_hand(&table); 
        if (table.current_hand->is_active == 1) {
            print_hand(table.current_hand);
            printf("\t#0 -> draw card\n");
            printf("Which card do you want to play? ");
            scanf("%d", &user_play_action);
            if (user_play_action == 0) {
                drawn_card = draw_to_player(&table, table.current_player);
                printf("Drawn card: \n");
                print_card(drawn_card, 0);
                //TODO check if it is possible to player and if not, don't ask!!!
                printf("Do you want to play this card? [0/1] ");
                scanf("%d", &user_play_action);
                if (user_play_action == 1) {
                    player_discard_drawn(&table, table.current_player);
                }
            } else {
                printf("Player %d discards: \n", table.current_player);
                print_card(table.current_hand->cards[user_play_action-1], 1);
                player_discard(&table, table.current_player, user_play_action-1);
            }
            printf("\n");
            print_hand(table.current_hand);
            printf("Next player? [0/1] ");
            next = scanf("%d", &next);
            if (next == 1) {
                prev_player = table.current_player;
                previous_hand = get_player_hand(&table, table.current_player);
            } else {
               run = 0;
            }
        } else {
            printf("Playing computer hand...\n");
            bot_best_next_card(&table, table.current_player);
            printf("Next player? [0/1] ");
            next = scanf("%d", &next);
        }
    }

    free_table(&table);
    printf("Ok, bye!");
    return 0;
}