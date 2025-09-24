#include "zkmini/groth16.hpp"
#include "zkmini/r1cs.hpp"
#include "zkmini/qap.hpp"
#include "zkmini/utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace zkmini;

std::vector<Fr> parse_witness(const std::string& witness_str) {
    std::vector<Fr> witness;
    std::stringstream ss(witness_str);
    std::string token;
    
    while (std::getline(ss, token, ',')) {
        // TODO: Parse field element from string
        // For now, just convert to integer
        uint64_t val = std::stoull(StringUtils::trim(token));
        witness.push_back(Fr(val));
    }
    
    return witness;
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " <r1cs_file> <pk_file> <public_inputs> <private_inputs> <proof_file>" << std::endl;
        std::cerr << "  public_inputs: comma-separated field elements" << std::endl;
        std::cerr << "  private_inputs: comma-separated field elements" << std::endl;
        return 1;
    }
    
    std::string r1cs_file = argv[1];
    std::string pk_file = argv[2];
    std::string public_str = argv[3];
    std::string private_str = argv[4];
    std::string proof_file = argv[5];
    
    try {
        ZK_TIMER("Prove Phase");
        
        std::cout << "Loading R1CS from: " << r1cs_file << std::endl;
        R1CS r1cs;
        // TODO: Implement R1CS loading
        
        std::cout << "Loading proving key from: " << pk_file << std::endl;
        ProvingKey pk = ProvingKey::load_from_file(pk_file);
        
        std::cout << "Converting R1CS to QAP..." << std::endl;
        QAP qap = QAP::from_r1cs(r1cs);
        
        std::cout << "Parsing witness..." << std::endl;
        std::vector<Fr> public_inputs = parse_witness(public_str);
        std::vector<Fr> private_inputs = parse_witness(private_str);
        
        std::cout << "Public inputs: " << public_inputs.size() << " elements" << std::endl;
        std::cout << "Private inputs: " << private_inputs.size() << " elements" << std::endl;
        
        // Verify witness satisfies R1CS
        std::cout << "Verifying witness..." << std::endl;
        if (!r1cs.is_satisfied(public_inputs, private_inputs)) {
            std::cerr << "Error: Witness does not satisfy R1CS!" << std::endl;
            return 1;
        }
        
        std::cout << "Generating proof..." << std::endl;
        Proof proof = Groth16::prove(pk, qap, public_inputs, private_inputs);
        
        std::cout << "Saving proof to: " << proof_file << std::endl;
        proof.save_to_file(proof_file);
        
        std::cout << "Proof generated successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}