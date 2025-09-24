#include "zkmini/keys.hpp"
#include "zkmini/serialization.hpp"

namespace zkmini {

// ProvingKey implementation
std::vector<uint8_t> ProvingKey::serialize() const {
    // TODO: Implement binary serialization
    return Serialization::serialize_g1(alpha_g1);
}

ProvingKey ProvingKey::deserialize(const std::vector<uint8_t>& data) {
    // TODO: Implement binary deserialization
    return ProvingKey();
}

void ProvingKey::save_to_file(const std::string& filename) const {
    std::vector<uint8_t> data = serialize();
    Serialization::write_file(filename, data);
}

ProvingKey ProvingKey::load_from_file(const std::string& filename) {
    std::vector<uint8_t> data = Serialization::read_file(filename);
    return deserialize(data);
}

// VerifyingKey implementation
std::vector<uint8_t> VerifyingKey::serialize() const {
    // TODO: Implement binary serialization
    return Serialization::serialize_g1(alpha_g1);
}

VerifyingKey VerifyingKey::deserialize(const std::vector<uint8_t>& data) {
    // TODO: Implement binary deserialization
    return VerifyingKey();
}

void VerifyingKey::save_to_file(const std::string& filename) const {
    std::vector<uint8_t> data = serialize();
    Serialization::write_file(filename, data);
}

VerifyingKey VerifyingKey::load_from_file(const std::string& filename) {
    std::vector<uint8_t> data = Serialization::read_file(filename);
    return deserialize(data);
}

// CRS implementation
std::vector<uint8_t> CRS::serialize() const {
    // TODO: Implement binary serialization of both keys
    std::vector<uint8_t> pk_data = pk.serialize();
    std::vector<uint8_t> vk_data = vk.serialize();
    
    std::vector<uint8_t> result;
    // Combine pk_data and vk_data with size headers
    return result;
}

CRS CRS::deserialize(const std::vector<uint8_t>& data) {
    // TODO: Implement binary deserialization
    return CRS();
}

void CRS::save_to_file(const std::string& filename) const {
    std::vector<uint8_t> data = serialize();
    Serialization::write_file(filename, data);
}

CRS CRS::load_from_file(const std::string& filename) {
    std::vector<uint8_t> data = Serialization::read_file(filename);
    return deserialize(data);
}

void CRS::save_keys(const std::string& pk_file, const std::string& vk_file) const {
    pk.save_to_file(pk_file);
    vk.save_to_file(vk_file);
}

CRS CRS::load_keys(const std::string& pk_file, const std::string& vk_file) {
    CRS crs;
    crs.pk = ProvingKey::load_from_file(pk_file);
    crs.vk = VerifyingKey::load_from_file(vk_file);
    return crs;
}

} // namespace zkmini