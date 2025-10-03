#include "zkmini/keys.hpp"
#include "zkmini/serialization.hpp"

namespace zkmini {

ProvingKey::ProvingKey() : num_variables(0), num_public(0), degree(0) {}

VerifyingKey::VerifyingKey() : num_public(0) {}
std::vector<uint8_t> ProvingKey::serialize() const {
    std::vector<uint8_t> result;
    
    Serialization::write_uint64(result, num_variables);
    Serialization::write_uint64(result, num_public);
    Serialization::write_uint64(result, degree);
    
    auto alpha_data = Serialization::serialize_g1(alpha_g1);
    result.insert(result.end(), alpha_data.begin(), alpha_data.end());
    
    auto beta_data = Serialization::serialize_g1(beta_g1);
    result.insert(result.end(), beta_data.begin(), beta_data.end());
    
    auto delta_data = Serialization::serialize_g1(delta_g1);
    result.insert(result.end(), delta_data.begin(), delta_data.end());
    
    auto beta_g2_data = Serialization::serialize_g2(beta_g2);
    result.insert(result.end(), beta_g2_data.begin(), beta_g2_data.end());
    
    auto delta_g2_data = Serialization::serialize_g2(delta_g2);
    result.insert(result.end(), delta_g2_data.begin(), delta_g2_data.end());
    
    Serialization::write_uint64(result, A_query_g1.size());
    for (const auto& point : A_query_g1) {
        auto point_data = Serialization::serialize_g1(point);
        result.insert(result.end(), point_data.begin(), point_data.end());
    }
    
    Serialization::write_uint64(result, B_query_g1.size());
    for (const auto& point : B_query_g1) {
        auto point_data = Serialization::serialize_g1(point);
        result.insert(result.end(), point_data.begin(), point_data.end());
    }
    
    Serialization::write_uint64(result, B_query_g2.size());
    for (const auto& point : B_query_g2) {
        auto point_data = Serialization::serialize_g2(point);
        result.insert(result.end(), point_data.begin(), point_data.end());
    }
    
    Serialization::write_uint64(result, K_query_g1.size());
    for (const auto& point : K_query_g1) {
        auto point_data = Serialization::serialize_g1(point);
        result.insert(result.end(), point_data.begin(), point_data.end());
    }
    
    Serialization::write_uint64(result, H_query_g1.size());
    for (const auto& point : H_query_g1) {
        auto point_data = Serialization::serialize_g1(point);
        result.insert(result.end(), point_data.begin(), point_data.end());
    }
    
    return result;
}

ProvingKey ProvingKey::deserialize(const std::vector<uint8_t>& data) {
    size_t offset = 0;
    ProvingKey result;
    
    result.num_variables = Serialization::read_uint64(data, offset);
    result.num_public = Serialization::read_uint64(data, offset);
    result.degree = Serialization::read_uint64(data, offset);
    
    result.alpha_g1 = Serialization::deserialize_g1(data, offset);
    result.beta_g1 = Serialization::deserialize_g1(data, offset);
    result.delta_g1 = Serialization::deserialize_g1(data, offset);
    
    result.beta_g2 = Serialization::deserialize_g2(data, offset);
    result.delta_g2 = Serialization::deserialize_g2(data, offset);
    
    uint64_t A_size = Serialization::read_uint64(data, offset);
    result.A_query_g1.resize(A_size);
    for (size_t i = 0; i < A_size; ++i) {
        result.A_query_g1[i] = Serialization::deserialize_g1(data, offset);
    }
    
    uint64_t B1_size = Serialization::read_uint64(data, offset);
    result.B_query_g1.resize(B1_size);
    for (size_t i = 0; i < B1_size; ++i) {
        result.B_query_g1[i] = Serialization::deserialize_g1(data, offset);
    }
    
    uint64_t B2_size = Serialization::read_uint64(data, offset);
    result.B_query_g2.resize(B2_size);
    for (size_t i = 0; i < B2_size; ++i) {
        result.B_query_g2[i] = Serialization::deserialize_g2(data, offset);
    }
    
    uint64_t K_size = Serialization::read_uint64(data, offset);
    result.K_query_g1.resize(K_size);
    for (size_t i = 0; i < K_size; ++i) {
        result.K_query_g1[i] = Serialization::deserialize_g1(data, offset);
    }
    
    uint64_t H_size = Serialization::read_uint64(data, offset);
    result.H_query_g1.resize(H_size);
    for (size_t i = 0; i < H_size; ++i) {
        result.H_query_g1[i] = Serialization::deserialize_g1(data, offset);
    }
    
    return result;
}

void ProvingKey::save_to_file(const std::string& filename) const {
    std::vector<uint8_t> data = serialize();
    Serialization::write_file(filename, data);
}

ProvingKey ProvingKey::load_from_file(const std::string& filename) {
    std::vector<uint8_t> data = Serialization::read_file(filename);
    return deserialize(data);
}

std::vector<uint8_t> VerifyingKey::serialize() const {
    std::vector<uint8_t> result;
    
    Serialization::write_uint64(result, num_public);
    
    auto alpha_data = Serialization::serialize_g1(alpha_g1);
    result.insert(result.end(), alpha_data.begin(), alpha_data.end());
    
    auto beta_data = Serialization::serialize_g2(beta_g2);
    result.insert(result.end(), beta_data.begin(), beta_data.end());
    
    auto gamma_data = Serialization::serialize_g2(gamma_g2);
    result.insert(result.end(), gamma_data.begin(), gamma_data.end());
    
    auto delta_data = Serialization::serialize_g2(delta_g2);
    result.insert(result.end(), delta_data.begin(), delta_data.end());
    
    Serialization::write_uint64(result, IC_g1.size());
    for (const auto& point : IC_g1) {
        auto point_data = Serialization::serialize_g1(point);
        result.insert(result.end(), point_data.begin(), point_data.end());
    }
    
    return result;
}

VerifyingKey VerifyingKey::deserialize(const std::vector<uint8_t>& data) {
    size_t offset = 0;
    VerifyingKey result;
    
    result.num_public = Serialization::read_uint64(data, offset);
    
    result.alpha_g1 = Serialization::deserialize_g1(data, offset);
    result.beta_g2 = Serialization::deserialize_g2(data, offset);
    result.gamma_g2 = Serialization::deserialize_g2(data, offset);
    result.delta_g2 = Serialization::deserialize_g2(data, offset);
    
    uint64_t IC_size = Serialization::read_uint64(data, offset);
    result.IC_g1.resize(IC_size);
    for (size_t i = 0; i < IC_size; ++i) {
        result.IC_g1[i] = Serialization::deserialize_g1(data, offset);
    }
    
    return result;
}

void VerifyingKey::save_to_file(const std::string& filename) const {
    std::vector<uint8_t> data = serialize();
    Serialization::write_file(filename, data);
}

VerifyingKey VerifyingKey::load_from_file(const std::string& filename) {
    std::vector<uint8_t> data = Serialization::read_file(filename);
    return deserialize(data);
}

std::vector<uint8_t> CRS::serialize() const {
    std::vector<uint8_t> pk_data = pk.serialize();
    std::vector<uint8_t> vk_data = vk.serialize();
    
    std::vector<uint8_t> result;
    
    Serialization::write_uint64(result, pk_data.size());
    result.insert(result.end(), pk_data.begin(), pk_data.end());
    
    Serialization::write_uint64(result, vk_data.size());
    result.insert(result.end(), vk_data.begin(), vk_data.end());
    
    return result;
}

CRS CRS::deserialize(const std::vector<uint8_t>& data) {
    size_t offset = 0;
    CRS result;
    
    uint64_t pk_size = Serialization::read_uint64(data, offset);
    std::vector<uint8_t> pk_data(data.begin() + offset, data.begin() + offset + pk_size);
    offset += pk_size;
    result.pk = ProvingKey::deserialize(pk_data);
    
    uint64_t vk_size = Serialization::read_uint64(data, offset);
    std::vector<uint8_t> vk_data(data.begin() + offset, data.begin() + offset + vk_size);
    result.vk = VerifyingKey::deserialize(vk_data);
    
    return result;
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

}