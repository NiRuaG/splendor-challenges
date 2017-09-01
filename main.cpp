#include <array>
#include <unordered_set>
#include <vector>
#include <string>
#include <map>
#include <stack>
#include <algorithm>
#include <iostream>

#include "splendor-challenges\ChallengeData.h"

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
    { 'W', diamond  }, // White
    { 'D', diamond  },
    { 'U', sapphire }, // blUe
    { 'S', sapphire },
    { 'G', emerald  }, // Green
    { 'E', emerald  },
    { 'R', ruby     }, // Red
    { 'K', onyx     }, // blacK
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

    
class dev_tableau : private std::array<std::array<dev_card,4>, 3>
{
public:
    using row_type = dev_tableau::value_type;
private:
    using T_base = std::array<row_type, 3>;

public:
    using T_base::operator[];
    dev_tableau(T_base&& b)
        : T_base{ b }
    {}
};



using dev_decks = std::array< std::vector<dev_card>, 3>;

struct game_common_area {
    token_supply tokens;
    dev_tableau dev_cards;
    dev_decks dev_decks;
    unsigned short num_turns;
};

struct player_gems {
    gem_vec bonuses;
    gem_vec resources;
    std::string hash;
};
struct game_player_area {
    unsigned short points;
    player_gems gems;
};

struct game {
    game_common_area common;
    game_player_area player;
};

struct action {
    enum {
        purchase,
        draw_gems
    }type;
    std::pair<unsigned short, unsigned short> purchased_card; // location (row,col)
    gem_vec drawn_gems;
};

bool can_buy(game_player_area const& p, dev_card const& c)
{
    if (  c.cost[0]
        + c.cost[1]
        + c.cost[2] 
        + c.cost[3] 
        + c.cost[4] == 0) // not a card
        return false;
    for (auto i = 0; i < 5; ++i) /// ugly, but wtf, std array size is not static?..
    {
        if (p.gems.bonuses[i] + p.gems.resources[i] < c.cost[i])
            return false;
    }
    return true;
}

std::vector<std::pair<dev_card,std::pair<unsigned short, unsigned short>>>
    all_buyable_dev_cards(decltype(decltype(game::common)::dev_cards) const& tableau, decltype(game::player) const& player)
{
    /// include prioritization of cards that get replaced 
    // they are exclusively better than non-replaced, allowing another option
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
            newgx.first.common.tokens[i] -= x[i];
            newgx.first.player.gems.resources[i] += x[i];
            newgx.first.player.gems.hash[2*i + 1] = newgx.first.player.gems.resources[i] + '0';
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
        for (auto i = 0; i < 5; ++i)
        {
            newgx.first.player.gems.resources[i] -=
                (newgx.first.player.gems.bonuses[i] < card.cost[i] ?
                    (card.cost[i] - newgx.first.player.gems.bonuses[i]) : 0);
            newgx.first.player.gems.hash[2*i+1] = newgx.first.player.gems.resources[i] + '0';
        }
        // gain card & replace if possible
        ++newgx.first.player.gems.bonuses[card.bonus]; // update bonus
        ++newgx.first.player.gems.hash[card.bonus * 2];
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
    static std::vector<std::unordered_set<std::string>> visited(18+5); // game states encountered by points
    // size includes point states from [0, goal-1+max points in a single turn]

    std::stack<gx_pair> S;
    S.push({ gg,{} });

    while (!S.empty()) {
        auto game_action_pair = S.top();

        // output
        if (game_action_pair.first.common.num_turns <= 12)
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
                auto h = x.first.player.gems.hash;
                if (visited[x.first.player.points].count(h))
                    continue;
                visited[x.first.player.points].insert(h);

                if (x.first.common.num_turns < 15)
                {
                    S.push(x);
                }
                else if (x.first.common.num_turns < 18 && (x.first.player.points >= (x.first.common.num_turns - 15) * 5 + 3))
                    // not on track for enough points
                    // 5 is the highest points that can be attained on a single turn (based on max point of purchasable card)
                    // by turn 17, need to have 13 points (minimum)
                    // by turn 16, need 8 points; by turn 15, at least 3 points
                    /// ** this would differ if there are Nobles in the game, and ultimately is better defined by what cards & nobles remain
                    /// this so far is a weak upper bound of max points per turn
                    S.push(x);
            }
        }
    }
}

int main() {
    DFS(Splendor::Challenges::Jacques_Cartier_1_Disc_of_Newfoundland);

    std::cin.get();
    return 0;
}