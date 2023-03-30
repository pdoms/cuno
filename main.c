#include<string.h>
#include<stdio.h>
#include<time.h>
#include "cuno.h"

#define FLAG_CAP 8 


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


int main(int argc, char **argv) {
    seed(time(NULL));
    //TODO before start determine who starts by drawing cards 
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
