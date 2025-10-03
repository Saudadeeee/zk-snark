#pragma once

#include "g1.hpp"
#include "g2.hpp"
#include "qap.hpp"
#include <vector>
#include <string>

namespace zkmini {

struct ProvingKey {
    G1 alpha_g1;
    G1 beta_g1;
    G2 beta_g2;
    G1 delta_g1;
    G2 delta_g2;
    
    
    std::vector<G1> A_query_g1;    
    std::vector<G2> B_query_g2;    
    std::vector<G1> B_query_g1;    
    std::vector<G1> K_query_g1;    
    std::vector<G1> H_query_g1;    
    
    size_t num_variables;
    size_t num_public;
    size_t degree;
    
    ProvingKey();
    
    std::vector<uint8_t> serialize() const;
    static ProvingKey deserialize(const std::vector<uint8_t>& data);
    
    void save_to_file(const std::string& filename) const;
    static ProvingKey load_from_file(const std::string& filename);
};

struct VerifyingKey {
    G1 alpha_g1;
    G2 beta_g2;
    G2 gamma_g2;
    G2 delta_g2;
    
    
    std::vector<G1> IC_g1;  
    
    size_t num_public;
    
    VerifyingKey();
    
    std::vector<uint8_t> serialize() const;
    static VerifyingKey deserialize(const std::vector<uint8_t>& data);
    
    void save_to_file(const std::string& filename) const;
    static VerifyingKey load_from_file(const std::string& filename);
};

struct CRS {
    ProvingKey pk;
    VerifyingKey vk;
    
    std::vector<uint8_t> serialize() const;
    static CRS deserialize(const std::vector<uint8_t>& data);
    
    void save_to_file(const std::string& filename) const;
    static CRS load_from_file(const std::string& filename);
    
    void save_keys(const std::string& pk_file, const std::string& vk_file) const;
    static CRS load_keys(const std::string& pk_file, const std::string& vk_file);
};

}