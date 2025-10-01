#include "zkmini/groth16.hpp"
#include "zkmini/r1cs.hpp"
#include "zkmini/qap.hpp"
#include "zkmini/utils.hpp"
#include <iostream>
#include <fstream>

using namespace zkmini;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <r1cs_file> <pk_file> <vk_file>" << std::endl;
        return 1;
    }
    
    std::string r1cs_file = argv[1];
    std::string pk_file = argv[2];
    std::string vk_file = argv[3];
    
    try {
        ZK_TIMER("Setup Phase");
        
        std::cout << "Loading R1CS from: " << r1cs_file << std::endl;
        
        // Load R1CS (placeholder - needs actual file format)
        R1CS r1cs;
        // TODO: Implement R1CS loading from file
        // r1cs = R1CS::load_from_file(r1cs_file);
        
        std::cout << "Converting R1CS to QAP..." << std::endl;
        QAP qap = r1cs_to_qap(r1cs);
        
        std::cout << "Generating trusted setup..." << std::endl;
        CRS crs = Groth16::setup(r1cs);
        
        std::cout << "Saving proving key to: " << pk_file << std::endl;
        crs.pk.save_to_file(pk_file);
        
        std::cout << "Saving verifying key to: " << vk_file << std::endl;
        crs.vk.save_to_file(vk_file);
        
        std::cout << "Setup completed successfully!" << std::endl;
        std::cout << "QAP info:" << std::endl;
        std::cout << "  Variables: " << qap.n << std::endl;
        std::cout << "  Public inputs: " << r1cs.public_inputs().size() << std::endl;
        std::cout << "  Constraints: " << qap.m << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}