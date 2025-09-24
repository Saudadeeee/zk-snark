#include "zkmini/groth16.hpp"
#include "zkmini/utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace zkmini;

std::vector<Fr> parse_public_inputs(const std::string& inputs_str) {
    std::vector<Fr> inputs;
    std::stringstream ss(inputs_str);
    std::string token;
    
    while (std::getline(ss, token, ',')) {
        // TODO: Parse field element from string
        uint64_t val = std::stoull(StringUtils::trim(token));
        inputs.push_back(Fr(val));
    }
    
    return inputs;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <vk_file> <public_inputs> <proof_file>" << std::endl;
        std::cerr << "  public_inputs: comma-separated field elements" << std::endl;
        return 1;
    }
    
    std::string vk_file = argv[1];
    std::string public_str = argv[2];
    std::string proof_file = argv[3];
    
    try {
        ZK_TIMER("Verify Phase");
        
        std::cout << "Loading verifying key from: " << vk_file << std::endl;
        VerifyingKey vk = VerifyingKey::load_from_file(vk_file);
        
        std::cout << "Loading proof from: " << proof_file << std::endl;
        Proof proof = Proof::load_from_file(proof_file);
        
        std::cout << "Parsing public inputs..." << std::endl;
        std::vector<Fr> public_inputs = parse_public_inputs(public_str);
        
        std::cout << "Public inputs: " << public_inputs.size() << " elements" << std::endl;
        
        if (public_inputs.size() != vk.num_public) {
            std::cerr << "Error: Expected " << vk.num_public << " public inputs, got " 
                      << public_inputs.size() << std::endl;
            return 1;
        }
        
        std::cout << "Verifying proof..." << std::endl;
        bool is_valid = Groth16::verify(vk, public_inputs, proof);
        
        if (is_valid) {
            std::cout << "✓ Proof is VALID!" << std::endl;
            return 0;
        } else {
            std::cout << "✗ Proof is INVALID!" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}