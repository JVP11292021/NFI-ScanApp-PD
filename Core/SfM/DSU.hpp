#ifndef SFM_DSU_H
#define SFM_DSU_H

#include "ns.hpp"

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

SFM_NS_B

template<typename ElemType>
class DisjointSetUnion {
public:
    DisjointSetUnion() : count(0) {}

public:
    void union_rel(ElemType e1, ElemType e2) {
        if (e1 == e2) return;
        std::int32_t e1_id = this->getId(e1);
        std::int32_t e2_id = this->getId(e2);
        std::int32_t c1 = this->findById(e1_id);
        std::int32_t c2 = this->findById(e2_id);
        if (c1 != c2) {
            if (depth[c1] < depth[c2]) {
                tr[c1] = c2;
                depth[c2] += depth[c1];
            }
            else {
                tr[c2] = c1;
                depth[c1] += depth[c2];
            }

            --count;
        }

    }
    std::int32_t find(ElemType e) {
        std::int32_t id = this->getId(e);
        return this->findById(id);
    }

    std::int32_t Connected(ElemType e1, ElemType e2) {
        return (this->find(e1) == this->find(e2)) ? 1 : 0;
    }

    std::vector<std::int32_t> getComponentIds() const {
        std::vector<std::int32_t> comp_ids;
        for (std::int32_t i = 0; i < tr.size(); ++i) {
            if (tr[i] == -1) comp_ids.push_back(i);
        }
        return comp_ids;
    }

    std::vector<ElemType> getElementsById(std::int32_t id) {
        std::vector<ElemType> comp_elems;
        for (std::int32_t i = 0; i < tr.size(); ++i) {
            if (id == FindById(i)) {
                comp_elems.push_back(GetEl(i));
            }
        }
        return comp_elems;
    }

    void printVec(std::ostream& os = std::cout) const {
        for (std::int32_t i = 0; i < tr.size(); ++i) {
            os << i << " (" << elems[i] << "): " << tr[i] << ",  depth = " << depth[i] << std::endl;
        }
    }

    inline std::int32_t count() const { return count; }

private:
    std::int32_t getId(ElemType e) {
        auto it = ids.find(e);
        if (it != ids.end()) {
            return it->second;
        }
        else {
            elems.push_back(e);
            tr.push_back(-1);
            depth.push_back(1);
            std::int32_t e_id = elems.size() - 1;
            ids.emplace(std::make_pair(e, e_id));
            ++count;
            return e_id;
        }
    }
    
    inline ElemType getEl(std::int32_t id) { return elems[id]; }
    
    std::int32_t findById(std::int32_t id) {
        while (tr[id] != -1) {
            id = tr[id];
        }
        return id;
    }

private:
    std::vector<ElemType> elems;
    std::vector<std::int32_t> tr;
    std::vector<std::int32_t> depth;
    std::map<ElemType, std::int32_t> ids;
    std::int32_t count;
};


template <typename ElemType>
using DSU = DisjointSetUnion<ElemType>;

SFM_NS_E

#endif // SFM_DSU_H