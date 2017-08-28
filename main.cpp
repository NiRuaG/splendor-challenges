#include <array>
#include <vector>
#include <string>
#include <map>
#include <stack>
#include <algorithm>
#include <iostream>

enum gem_types {
    diamond,
    sapphire,
    emerald,
    ruby,
    onyx
};

using gem_vec = std::array<unsigned short, 5>;

class gem_vec_in {
private:
    gem_vec vec{};
    static std::map<char, gem_types> const gem_type_of_char;
public:
    gem_vec_in(std::string s) {
        unsigned short n = 1;
        for (char c : s) {
            if (c > '0' && c <= '9')
                n = c - '0';
            else {
                vec[gem_type_of_char.at((char)toupper(c))] = n;
                n = 1;
            }
        }
    }
    gem_vec_in(decltype(vec) v)
        : vec(v) {}

    gem_vec_in() {}

    auto operator[](std::size_t i) const ->decltype(vec[i]) /// range check!?
    {
        return vec[i];
    }
};

decltype(gem_vec_in::gem_type_of_char) gem_vec_in::gem_type_of_char = {
    { 'W', diamond  },
    { 'D', diamond  },
    { 'U', sapphire },
    { 'S', sapphire },
    { 'G', emerald  },
    { 'E', emerald  },
    { 'R', ruby     },
    { 'K', onyx     },
    { 'O', onyx     }
};

using token_supply = gem_vec;

struct dev_card {
    unsigned short prestige_pts;
    gem_vec_in cost;
    gem_types bonus;
    bool does_not_replace = false;
};
const dev_card nocard;
using dev_tableau_row = std::array<dev_card, 4>;
using dev_tableau = std::array<dev_tableau_row, 3>;

using dev_decks = std::array< std::vector<dev_card>, 3>;

struct game_common_area {
    token_supply tokens;
    dev_tableau dev_cards;
    dev_decks dev_decks;
    unsigned short num_turns;
};

struct game_player_area {
    unsigned short points;
    gem_vec bonuses;
    gem_vec resources;
};

struct game {
    game_common_area common;
    game_player_area player;
} g = {
    // common area
    {
        //token_supply
        { 2,2,2,2,2 },

        // dev_cards
        {{ // bottom row
            {{ { 0,"2RWKG",sapphire,true  },
               { 0,"GWKU" ,ruby           },
               { 0,"RUK2G",diamond        },
               { 0,"2R2KU",emerald        } }},
            // middle row
            {{ { 2,"5R3K"  ,diamond       },
               { 1,"2G3R3W",emerald       },
               { 2,"5R"    ,diamond       },
               { 1,"3G3K2U",sapphire      } }},
            // top row
            {{ { 5,"7U3G"  ,emerald       },
               { 4,"6U3G3W",emerald       },
               { 5,"3W7K"  ,diamond ,true },
               { 4,"3U3K6W",sapphire,true } }}
        }},

        // dev_decks REVERSE ORDER!!!
        {{
            // bottom row
            {   { 0,"G2WKU",ruby    ,true },
                { 0,"W2K"  ,sapphire      },
                { 0,"2U2K" ,diamond       },
                { 0,"2RK"  ,diamond       },
                { 0,"3K"   ,sapphire,true },
                { 0,"2GR"  ,onyx    ,true },
                { 0,"G2K2W",ruby          },
                { 0,"2G2RW",sapphire      },
                { 0,"2UG"  ,ruby          },
                { 0,"RWKU" ,emerald       } 
            },
            // middle row
            {   { 2,"5W3U"  ,sapphire,true },
                { 2,"5G3R"  ,onyx          },
                { 3,"6K"    ,onyx    ,true },
                { 2,"4U2GW" ,ruby    ,true },
                { 1,"2R2K3G",diamond       },
                { 3,"6W"    ,diamond ,true },
                { 2,"4G2RU" ,onyx          },
                { 1,"3U2K2W",emerald       }
            },
            // top row
            { 
                { 4,"7U"      ,emerald,true },
                { 3,"3G3W3K5U",ruby         },
                { 3,"5G3W3R3U",onyx   ,true }
            }
        }}
    },
    // player area
    {
        0, //points
        { 1,1,1,1,1 },// bonuses
        { 0,0,0,0,0 } // resources
    }
};

struct action {
    enum {
        purchase,
        draw_gems
    }type;
    std::pair<unsigned short, unsigned short> purchased_card;
    gem_vec drawn_gems;
};

bool can_buy(decltype(g.player) const& p, dev_card const& c)
{
    if (  c.cost[0]
        + c.cost[1]
        + c.cost[2] 
        + c.cost[3] 
        + c.cost[4] == 0) // not a card
        return false;
    for (auto i = 0; i < 5; ++i) /// ugly, but wtf, std array size is not static?..
    {
        if (p.bonuses[i] + p.resources[i] < c.cost[i])
            return false;
    }
    return true;
}

std::vector<std::pair<dev_card,std::pair<unsigned short, unsigned short>>>
    all_buyable_dev_cards(decltype(decltype(game::common)::dev_cards) const& tableau, decltype(game::player) const& player)
{
    std::vector<std::pair<dev_card, std::pair<unsigned short, unsigned short>>> ret{};

    for (unsigned short r = 0; r < 3; ++r) {
        for (unsigned short c = 0; c < 4; ++c) {
            auto card = tableau[r][c];
            if (can_buy(player, card))
                ret.push_back({ card, {r, c} });
        }
    }

    return ret;
}

std::vector<gem_vec> 
    all_gem_combos(decltype(decltype(game::common)::tokens) const& tok)
{
    std::vector<gem_vec> ret{};
    static std::vector<gem_vec> FIVE_CHOOSE_3 =
    {
        { 1,1,1,0,0 },{ 1,1,0,1,0 },{ 1,1,0,0,1 },{ 1,0,1,1,0 },{ 1,0,1,0,1 },{ 1,0,0,1,1 },
        { 0,1,1,1,0 },{ 0,1,1,0,1 },{ 0,1,0,1,1 },
        { 0,0,1,1,1 },
    };

    for (auto& comb : FIVE_CHOOSE_3) {
        bool candrawthese = true;
        for (unsigned short i = 0; i < 5; ++i) {
            if (comb[i] && !tok[i]) {
                candrawthese = false;
                break;
            }
        }
        if (candrawthese)
            ret.push_back(comb);
    }
 
    return ret;
}

using gx_pair = std::pair<game, std::vector<action>>;
std::vector<gx_pair> game_states_from_possible_actions(gx_pair const& gx)
{
    auto const& gg = gx.first;

    std::vector<gx_pair> ret{};

    // all combinations of 3 gems that can be drawn
    //auto TESTVV = all_gem_combos(gg.common.tokens);
    for (auto const& x : all_gem_combos(gg.common.tokens)) {
        auto newgx{ gx };
        ++newgx.first.common.num_turns;

        // remove from supply & add to player resources
        for (auto i = 0; i < 5; ++i) /// ugly, but wtf, std array size is not static?..
        {
            newgx.first.common.tokens   [i] -= x[i];
            newgx.first.player.resources[i] += x[i];
        }

        newgx.second.push_back({ action::draw_gems, {}, x });
        ret.push_back(newgx);
    }

    // for all cards that can be bought
    auto TESTVEC = all_buyable_dev_cards(gg.common.dev_cards, gg.player);
    ;
    for (auto const& x : all_buyable_dev_cards(gg.common.dev_cards, gg.player))
    {
        auto& card = x.first;
        auto& r = x.second.first;
        auto& c = x.second.second;

        if (!card.does_not_replace && gg.common.dev_decks[r].empty())
        {
            std::cout << "DONT KNOW NEXT CARD for row " << r;
            std::exit(0);
        }

        auto newgx{ gx };
        ++newgx.first.common.num_turns;
        // spend
        for (auto i = 0; i < 5; ++i) /// ugly, but wtf, std array size is not static?..
        {
            newgx.first.player.resources[i] -=
                (newgx.first.player.bonuses[i] < card.cost[i] ?
                    (card.cost[i] - newgx.first.player.bonuses[i]) : 0);
        }
        // gain card & replace if possible
        ++newgx.first.player.bonuses[card.bonus]; // update bonus
        newgx.first.player.points += card.prestige_pts;
        if (card.does_not_replace) {
            newgx.first.common.dev_cards[r][c] = nocard;
        }
        else {
            newgx.first.common.dev_cards[r][c] = newgx.first.common.dev_decks[r].back();
            newgx.first.common.dev_decks[r].pop_back();
        }

        newgx.second.push_back({ action::purchase, {r,c}});

        ret.push_back(newgx);
    }

    return ret;
}

enum DFS_return {
    GOT_IT,
    NOPE,
};

std::ostream& operator<<(std::ostream& os, gem_vec const& v)
{
    if (v[onyx])
        os << 'K';
    if (v[ruby])
        os << 'R';
    if (v[emerald])
        os << 'G';
    if (v[sapphire])
        os << 'U';
    if (v[diamond])
        os << 'W';

    return os;
}


DFS_return DFS(game gg)
{
    // std::hash<game> h(gg); 
    // hash would help a lot to determine already visited game states 
    // but hash would be hard to determine currently given the dev_deck vector
    std::stack<gx_pair> S;
    S.push({ gg,{} });

    while (!S.empty()) {
        auto game_action_pair = S.top();

        if (game_action_pair.first.common.num_turns <= 14)
        {
            if (!game_action_pair.second.empty())
            {
                std::cout
                    << "T" << std::string(game_action_pair.first.common.num_turns, ' ')
                    << game_action_pair.first.common.num_turns << "-" << game_action_pair.first.player.points << " ";

                if (game_action_pair.second.back().type == action::purchase)
                {
                    auto pc = game_action_pair.second.back().purchased_card;
                    std::cout << "buy: @ " << pc.first << ' ' << pc.second;
                }
                else
                {
                    std::cout << "take: " << game_action_pair.second.back().drawn_gems;
                }
                std::cout << '\n';
            }
            //std::cin.get();
        }
        S.pop();

        for (auto& x : game_states_from_possible_actions(game_action_pair))
        {
            if (x.first.player.points >= 18 && x.first.common.num_turns <= 18)
                return GOT_IT;
            else
            {
                if (x.first.common.num_turns <= 18)
                    S.push(x);
                ;
            }
        }
    }
}

int main() {
    DFS(g);

    std::cin.get();
    return 0;
}