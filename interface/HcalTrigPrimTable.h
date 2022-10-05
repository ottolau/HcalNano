#ifndef HcalTrigPrimTable_h
#define HcalTrigPrimTable_h

#include <vector>
#include <map>

#include "Geometry/HcalTowerAlgo/interface/HcalTrigTowerGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "DataFormats/HcalDetId/interface/HcalTrigTowerDetId.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DQM/HcalCommon/interface/Utilities.h"

namespace hcalnano {
    class HcalTrigPrimTable {
    public:
        std::vector<HcalTrigTowerDetId> dids_;
        std::map<HcalTrigTowerDetId, unsigned int> did_indexmap_; // Use std::map for efficient lookup, rather than std::find

        std::vector<int> ietas_;
        std::vector<int> iphis_;
        std::vector<int> soiCompressedEts_;
        std::vector<int> soiFineGrains_;
        std::vector<int> presampless_;
        std::vector<bool> valids_;

        unsigned int size_;
        std::vector<std::vector<int>> compressedEts_;
        std::vector<std::vector<int>> fineGrains_;
        std::vector<std::vector<int>> hbheDigiIndexs_;
        std::vector<std::vector<int>> hfDigiIndexs_;

        HcalTrigPrimTable(std::vector<HcalTrigTowerDetId>& _dids, unsigned int _size);
        void add(const HcalTriggerPrimitiveDigi* tp, const edm::ESHandle<HcalTrigTowerGeometry>& geometry);
        void reset();

    };

}
#endif
