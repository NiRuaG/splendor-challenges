#include <array>
#include <unordered_set>
#include <vector>
#include <string>
#include <map>
#include <stack>
#include <algorithm>
#include <iostream>

#include "splendor-challenges\ChallengeData.h"
#include "splendor-challenges\Splendor.h"

namespace Splendor {
    bool can_buy(game_player_area const& p, dev_card const& c)
    {
        if (!c.cost[0]
            && !c.cost[1]
            && !c.cost[2]
            && !c.cost[3]
            && !c.cost[4]) // not a card
            /// need a better conditional to see if card
            return false;
        for (auto i = 0; i < 5; ++i) /// ugly, but wtf, std array size is not static / constexpr ?..
        {
            if (p.gems.bonuses[i] + p.gems.resources[i] < c.cost[i])
                return false;
        }
        return true;
    }

    std::vector<std::pair<dev_card, std::pair<unsigned short, unsigned short>>>
        all_buyable_dev_cards(decltype(decltype(game::common)::dev_cards) const& tableau, decltype(game::player) const& player)
    {
        /// TODO include prioritization of cards that get replaced 
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
                newgx.first.player.gems.hash[2 * i + 1] = newgx.first.player.gems.resources[i] + '0';
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
                newgx.first.player.gems.hash[2 * i + 1] = newgx.first.player.gems.resources[i] + '0';
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

            newgx.second.push_back({ action::purchase, {r,c} });

            ret.push_back(newgx);
        }

        return ret;
    }    
}

std::ostream& operator<<(std::ostream& os, Splendor::gem_vec const& v)
{
    using namespace Splendor;

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

enum DFS_return {
    GOT_IT,
    NOPE,
};

DFS_return DFS(Splendor::challenge chlg)
{
    static std::vector<std::unordered_set<std::string>> visited(chlg.point_goal+5); // keep track of game states already encountered, grouped by points
    // size includes point states from [0, goal-1+max points in a single turn]

    std::stack<Splendor::gx_pair> S;
    S.push({ chlg.game_setup,{} });

    while (!S.empty()) {
        auto game_action_pair = S.top();

        // output
        if (game_action_pair.first.common.num_turns <= 12) /// VARIABLE value for "diagnostic", adjusts depth for how often output will show up
        {
            if (!game_action_pair.second.empty())
            {
                std::cout
                    << "T" << std::string(game_action_pair.first.common.num_turns, ' ')
                    << game_action_pair.first.common.num_turns << "-" << game_action_pair.first.player.points << " ";

                if (game_action_pair.second.back().type == Splendor::action::purchase)
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
            if (x.first.player.points >= chlg.point_goal && x.first.common.num_turns <= chlg.turn_limit)
                return GOT_IT;
            else
            {
                auto h = x.first.player.gems.hash;
                if (visited[x.first.player.points].count(h))
                    continue;
                visited[x.first.player.points].insert(h);

                if (x.first.common.num_turns < chlg.turn_limit
                    && (x.first.player.points >= 
                        chlg.point_goal - 5*signed(chlg.turn_limit-x.first.common.num_turns)) )
                    // not on track for enough points
                    // if 5 is the highest points that can be attained on a single turn (based on max point of purchasable card)
                    // by previous turn, need to have GOAL-5 points (minimum)
                    // by turn 16, need GOAL-10 points; by turn GOAL-15
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