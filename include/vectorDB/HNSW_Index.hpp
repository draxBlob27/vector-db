#include "cmath"
#include "HNSW/Node.hpp"

class HNSW_Index {
private:
    std::uint64_t m_ep; //entry point in graph
    std::vector<Node> m_nodes;
    int m_max_layer; //current max layer in graph
    std::uint32_t m_M;
    std::uint32_t m_Mmax;
    std::uint32_t m_Mmax0;
    std::uint32_t m_efConstruction;
    std::uint32_t m_efSearch;
    double m_ml;

public:
    HNSW_Index(std::uint32_t M, std::uint32_t Mmax, std::uint32_t Mmax0, std::uint32_t efConstruction, std::uint32_t efSearch) 
        :m_M{M}, m_Mmax{Mmax}, m_Mmax0{Mmax0}, m_efConstruction{efConstruction}, m_efSearch{efSearch} 
    {
        m_ml = 1 / std::log(M);
    }

    
};