#pragma once

struct game;

namespace Splendor {
    namespace Challenges {
        extern game const Jacques_Cartier_1_Disc_of_Newfoundland =
            // 18 prestige points Goal::Points<18>
            // within 18 turns Goal::Limit<18>
            // cards are predetermined Challenge::Random<False>
            // some cards will not be replaced  
            // 2 tokens per color, once spent are not returned
            // have 1 bonuses each color
            // may not reserve
        {
            // common area
            {
                //token_supply
                { 2, 2, 2, 2, 2 },

                // dev_cards
                { {{
                        // bottom row
                        {{ { 0,"2RWKG" ,sapphire,true  },
                           { 0,"GWKU"  ,ruby           },
                           { 0,"RUK2G" ,diamond        },
                           { 0,"2R2KU" ,emerald        } }},
                        // middle row
                        { { { 2,"5R3K"  ,diamond       },
                            { 1,"2G3R3W",emerald       },
                            { 2,"5R"    ,diamond       },
                            { 1,"3G3K2U",sapphire      } }},
                        // top row
                        {{ { 5,"7U3G"  ,emerald       },
                           { 4,"6U3G3W",emerald       },
                           { 5,"3W7K"  ,diamond ,true },
                           { 4,"3U3K6W",sapphire,true } }}
        }} },

                // dev_decks REVERSE ORDER!!!
                {{
                        // bottom row
                    { { 0,"G2WKU",ruby    ,true },
                    { 0,"W2K"  ,sapphire },
                    { 0,"2U2K" ,diamond },
                    { 0,"2RK"  ,diamond },
                    { 0,"3K"   ,sapphire,true },
                    { 0,"2GR"  ,onyx    ,true },
                    { 0,"G2K2W",ruby },
                    { 0,"2G2RW",sapphire },
                    { 0,"2UG"  ,ruby },
                    { 0,"RWKU" ,emerald }
                    },
                // middle row
                    { { 2,"5W3U"  ,sapphire,true },
                    { 2,"5G3R"  ,onyx },
                    { 3,"6K"    ,onyx    ,true },
                    { 2,"4U2GW" ,ruby    ,true },
                    { 1,"2R2K3G",diamond },
                    { 3,"6W"    ,diamond ,true },
                    { 2,"4G2RU" ,onyx },
                    { 1,"3U2K2W",emerald }
                    },
                // top row
                    {
                        { 4,"7U"      ,emerald,true },
                        { 3,"3G3W3K5U",ruby },
                        { 3,"5G3W3R3U",onyx   ,true }
                    }
                    } }
            },

            // player area
            {
                0, //points
                { { 1,1,1,1,1 }, // bonuses
                  { 0,0,0,0,0 }, // resources
                   "1010101010" } // hash
            }
        };
    }
}