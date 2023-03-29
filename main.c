#include<stdio.h>
#include<stdlib.h>

#include<string.h>
#include<time.h>


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
int DEBUG = 0;

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
    printf("[GAME] shuffling deck\n");
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
    printf("[GAME] dealing hands\n");
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


int get_card_idx_from_hand(Hand* hand, Card* card_ptr) {
    int i = 0;
    for (i; i < hand->num_cards; i++) {
        if (card_ptr == hand->cards[i]) {
            return i;         
        }
    }
    return hand->num_cards;
}

int player_next_best_card_idx(Table* table) {
    Hand* hand = table->current_hand;
    int len = hand->num_cards;
    Card* possible_cards[len];

    int i = 0;
    int size = 0;
    for (i; i < len; i++) {
        Card* card = hand->cards[i];
        if (card->color == table->current_color 
                || card->value == table->current_value) {
            possible_cards[size] = card;
            size++;
        }
    }
    if (size == 0) {
        return len;
    }
    if (size == 1) {
        return get_card_idx_from_hand(hand, possible_cards[0]);       
    } else {
        insertion_sort_cards(possible_cards, size);
        return get_card_idx_from_hand(hand, possible_cards[size-1]);
    }
}

void print_player_hand(Table* table, int suggestion) {

    Hand* hand = table->current_hand;
    printf("+++ Player %d's Hand", hand->player_id);
    if (hand->is_active == 1) {
        printf(" (active)\n");
    } else {
        printf("\n");
    }
    
    int i = 0;
    int sugg;
    if (suggestion == 1) {
        sugg = player_next_best_card_idx(table);
    }
    for (i;i<hand->num_cards;i++) {
        printf("\t#%d -> color: %s; value: %s", i+1, 
                print_color(hand->cards[i]->color), 
                print_value(hand->cards[i]->value));
        if (suggestion == 1 && i == sugg) {
            printf("\t\t<-\n");
        } else {
            printf("\n");
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
    printf("[GAME] drawing initial discard\n");
    int top = table->discarded->len;
    while (deck[table->top_of_deck].color == BLACK) {
        table->discarded->cards[table->discarded->len] = &deck[table->top_of_deck];
        table->discarded->len++;
        table->top_of_deck++;
    }
    table->discarded->cards[table->discarded->len] = &deck[table->top_of_deck];
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
    table.current_hand = table.player_hands[0];
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
    if (table->dir == CLOCKWISE) {
        if (table->current_player < table->num_players) {
            table->current_player = table->current_player + 1;
        } else {
            table->current_player = 1;
        }
    } else {
        if (table->current_player == 1) {
            table->current_player = table->num_players;
        } else {
            table->current_player = table->current_player-1;
        }
    }
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
    Hand* hand = get_player_hand(table, player_id);
    Card* card = get_next_card(table);
    card->player_id = player_id;
    hand->cards[hand->num_cards] = card;
    hand->num_cards++;
    return card;
}



int player_discard(Table* table, int player_id, int card_idx) {

    Hand* hand = get_player_hand(table, player_id);
    
    int top = table->discarded->len;
    Card* card = hand->cards[card_idx];
    Card* current_discard = table->discarded->cards[top-1];

    if (card->color == current_discard->color 
            || card->value == current_discard->value
            || card->color == BLACK
            ) {
        table->discarded->cards[top] = hand->cards[card_idx]; 
        table->current_color = table->discarded->cards[top]->color;
        table->current_value = table->discarded->cards[top]->value;
        deck[table->top_of_deck].location = DISCARD;
        table->discarded->len++;
        table->top_of_deck++;
        shift_cards(hand, card_idx);
        return 0;
    }
    return 1;
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

int has_card_value(Hand *hand, int value) {
    int i =0;
    for (i; i < hand->num_cards; i++) {
        if (hand->cards[i]->value == value) {
            return i;
        }
    }
    return hand->num_cards;
}

int check_if_can_stack(Table *table) {
    Card *card = table->discarded->cards[table->discarded->len-1];
    Hand *hand = table->current_hand;
    int check = has_card_value(hand, card->value);
    return check;

}

void bot_best_next_card(Table* table, int player_id, int no_draw) {
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
    if (DEBUG == 1) {
        print_hand(hand);
    }
    if (size == 1) {
        int idx = get_card_idx(table, possible_cards[0]);
        player_discard(table, table->current_player, idx);
        return;
    }

    if (size > 1) {
        insertion_sort_cards(possible_cards, size);
        if (DEBUG == 1) {
            printf("Will play\n");
            print_card(possible_cards[size-1], 1);
        }
        int idx = get_card_idx(table, possible_cards[size-1]);
        player_discard(table, table->current_player, idx);
        return;
        
    }

    if (size == 0 && no_draw == 0) {
        printf("Will draw\n");
        draw_to_player(table, table->current_player);
        return;
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
        if (clrs[i] > max) {
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
    printf("\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\n");
}
void print_action(ACTION act) { 
    switch (act) {
        case NEXT_PLAYER:
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

int is_uno(Table *table) {
    Hand *hand = table->current_hand;
    if (hand->num_cards == 1) {
        print_delim();
        printf("=>=>=> Player %d says UNO!\n", table->current_player);
        print_delim();
        return 1;
    } else {
        return 0;
    }
}


    //NEXT_PAUSE = 10,
    //REVERSE,
    //DRAW_TWO,
    //CHOOSE_COLOR,
    //DRAW_FOUR,


int calculate_hands(Table *table, int winner) {
    printf("Hand Values:\n\n");
    int sums[table->num_players];
    memset(sums, 0, table->num_players*sizeof(int));
    int i = 0;
    int total = 0;
    for (i; i <table->num_players;i++) {
        Hand* hand = table->player_hands[i];
        int j = 0;
        for (j; j < hand->num_cards;j++) {
            Card* card = hand->cards[j];
            if (card->value < 10) {
                sums[i] += card->value;
            } else if (card-> value < 13) {
                sums[i] += 20;
            } else {
                sums[i] += 50;
            }
        }
        if (i + 1 != winner) {
            printf("\tPlayer %d: %d\n", i+1, sums[i]);
        }
        total += sums[i];
    }
    return total;
}

int player_has_won(Table *table) {
    Hand *hand = table->current_hand;
    if (hand->num_cards == 0) {
        print_delim();
        printf("=>=>=> Player %d has Won!\n", table->current_player);
        int score = calculate_hands(table, hand->player_id);
        printf("\n\tPlayer %d's score: %d\n", table->current_player, score);
        print_delim();
        return 1;
    } else {
        return 0;
    }
}

int main(int argc, char **argv) {
    seed(time(NULL));
    //TODO bug -> boundary error when draw card 
    //TODO before start determine who starts by drawing cards 
    //TODO check if deck is exhausted
    //TODO reshuffle when stack is exhausted
    int num_players = DEFAULT_PLAYERS;
    int num_active = DEFAULT_ACTIVE;
    char* PLAYER = "player";
    char* DEB = "-d";
    for (int i = 1; i < argc; ++i)
    {   
        char flag[FLAG_CAP];
        char value[2];
        parse_arg(argv[i], flag, value);
        if (streq(flag, PLAYER)) {
            num_players=c_to_int(value[0]);
        } else if (streq(flag, DEB)) {
            DEBUG = 1; 
        } else {
            printf("UNKNOWN ARGUMENT - %s\n", argv[i]);
        }
    }   
    if (DEBUG == 1) {
        printf("[DEBUG] NUM PLAYERS: %d\n", num_players);
        printf("[DEBUG] NUM ACTIVE: %d\n", num_active);
    }
    Table table = set_table(num_players, num_active);
    int user_play_action;
    Card* drawn_card;
    int run = 1;
    int next;
    int next_action;
    COLOR clr;
    int init = 1;
    int skip_play = 0;
    int comes_from_draw = 0;

    while (run) {
        print_delim();
        print_discard(&table);
        next_action = determine_action(&table);
        if (DEBUG == 1) {
            print_action(next_action);
        }
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
            case NEXT_DRAW2:
                if (init == 0) {
                    set_current_player(&table);
                }
                if (table.current_hand->is_active == 1) {
                    if (check_if_can_stack(&table) < table.current_hand->num_cards) {
                        print_player_hand(&table, 0);
                        printf("\t#0 -> draw 2 cards\n");
                        printf("Do you want to play? ");
                        scanf("%d", &user_play_action);
                        if (user_play_action == 0) {
                            draw_to_player(&table, table.current_player);
                            draw_to_player(&table, table.current_player);
                        } else {
                            player_discard(&table, table.current_player, user_play_action-1);
                            skip_play = 1;
                        }  
                    } else {
                        draw_to_player(&table, table.current_player);
                        draw_to_player(&table, table.current_player);
                    }
                } else {   
                    draw_to_player(&table, table.current_player);
                    draw_to_player(&table, table.current_player);
                }   
                comes_from_draw = 1;
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
                comes_from_draw = 1;
                break; 
            default:
                if (init == 1) {
                    init = 0;
                    break;
                }
                set_current_player(&table);
                break;
        }
        if (run == 0) {
            break;
        }
        //print_discard(&table);
        if (skip_play == 0) {
            printf("Player %d's turn!\n", table.current_player);
            set_current_hand(&table); 
            if (table.current_hand->is_active == 1) {
                print_player_hand(&table, 1);
                if (comes_from_draw == 1) {
                    printf("\t#0 -> fold\n");
                    comes_from_draw = 0;
                } else {
                    printf("\t#0 -> draw card\n");
                }
                printf("Which card do you want to play? ");
                scanf("%d", &user_play_action);
                if (user_play_action == 0) {
                    drawn_card = draw_to_player(&table, table.current_player);
                    printf("Drawn card: \n");
                    print_card(drawn_card, 0);
                    int topp = table.discarded->len;
                    Card* current_discard = table.discarded->cards[topp-1];
                    if (drawn_card->value == current_discard->value || drawn_card->color == current_discard->color) {
                        printf("Do you want to play this card? [0/1] ");
                        scanf("%d", &user_play_action);
                        if (user_play_action == 1) {
                            player_discard_drawn(&table, table.current_player); 
                        }
                    }
                } else {
                    printf("Player %d discards: \n", table.current_player);
                    print_card(table.current_hand->cards[user_play_action-1], 1);
                    int card_idx = user_play_action-1;
                    while (player_discard(&table, table.current_player, card_idx)) {
                        printf("\n\n========== ILLEGAL MOVE ===========\n");
                        printf("\nCard cannot be discarded. Try again\n");
                        printf("\n=====================================\n\n");
                        print_player_hand(&table, 1);
                        printf("\t#0 -> draw card\n");
                        printf("Which card do you want to play? ");
                        scanf("%d", &user_play_action);
                        card_idx = user_play_action-1;
                    }
                }
                printf("\n");
                print_hand(table.current_hand);
                print_delim();
                printf("Next player? [0/1] ");
                next = scanf("%d", &next);
            } else {
                printf("Playing %d's hand.\n", table.current_player);
                bot_best_next_card(&table, table.current_player, comes_from_draw);
                if (comes_from_draw == 1) {
                    comes_from_draw = 0;
                }
                print_delim();
                print_discard(&table);
                printf("Next player? [0/1] ");
                next = scanf("%d", &next);
            }
        } else {
            skip_play = 0;
        }
        is_uno(&table);
        if (player_has_won(&table) == 1) {
            run = 0;
        }
    }

    free_table(&table);
    printf("Bye!");
    return 0;
}
