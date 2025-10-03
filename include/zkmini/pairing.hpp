#pragma once

#include "g1.hpp"
#include "g2.hpp"
#include "fq12.hpp"

namespace zkmini {
class Pairing {
public:
    
    static Fq12 pairing(const G1& P, const G2& Q);
    
    
    static Fq12 miller_loop(const G1& P, const G2& Q);
    
    
    static Fq12 multi_pairing(const std::vector<G1>& P_vec, const std::vector<G2>& Q_vec);
    
    
    static bool pairing_check(const std::vector<G1>& P_vec, const std::vector<G2>& Q_vec);
    
    
    struct G2Prepared {
        std::vector<Fq12> lines;
        G2 point;
    };
    
    static G2Prepared prepare_g2(const G2& Q);
    static Fq12 miller_loop_prepared(const G1& P, const G2Prepared& Q_prep);

private:
    
    static Fq12 line_double(G2& R, const G1& P);
    static Fq12 line_add(G2& R, const G2& Q, const G1& P);
    
    
    static constexpr uint64_t ATE_LOOP_COUNT = 0x9d797039be763ba8ULL;
    static constexpr bool ATE_IS_LOOP_COUNT_NEG = false;
};

}