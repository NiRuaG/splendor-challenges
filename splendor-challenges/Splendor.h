#pragma once

namespace Splendor {
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


    class dev_tableau : private std::array<std::array<dev_card, 4>, 3>
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
}