#include "HCALPFG/HcalNano/interface/HcalTrigPrimTable.h"

hcalnano::HcalTrigPrimTable::HcalTrigPrimTable(std::vector<HcalTrigTowerDetId>& _dids, unsigned int _size) {
    dids_ = _dids;
    for (std::vector<HcalTrigTowerDetId>::const_iterator it_did = dids_.begin(); it_did != dids_.end(); ++it_did) {
        did_indexmap_[*it_did] = (unsigned int)(it_did - dids_.begin());
    }

    size_ = _size;
    ietas_.resize(dids_.size());
    iphis_.resize(dids_.size());
    soiCompressedEts_.resize(dids_.size());
    soiFineGrains_.resize(dids_.size());
    presampless_.resize(dids_.size());
    valids_.resize(dids_.size());

    compressedEts_.resize(size_, std::vector<int>(dids_.size()));
    fineGrains_.resize(size_, std::vector<int>(dids_.size()));
    hbheDigiIndexs_.resize(size_, std::vector<int>(dids_.size()));
    hfDigiIndexs_.resize(size_, std::vector<int>(dids_.size()));
}

void hcalnano::HcalTrigPrimTable::add(const HcalTriggerPrimitiveDigi* tp, const edm::Handle<HBHEDigiCollection>& hbheDigis, const edm::Handle<HFDigiCollection>& hfDigis, const edm::ESHandle<HcalTrigTowerGeometry>& geometry) {
    HcalTrigTowerDetId did = tp -> id();
    unsigned int index = did_indexmap_.at(did);//std::find(dids_.begin(), dids_.end(), did) - dids_.begin();
    //if (index == dids_.size()) {
    //    std::cerr << "[HcalTrigPrimTable] ERROR : Didn't find did " << did << " in table" << std::endl;
    //    exit(1);
    //}

    HBHEDigiCollection::const_iterator iHBHE;
    HBHEDigiCollection::const_iterator first_HBHE = hbheDigis -> begin();

    HFDigiCollection::const_iterator iHF;
    HFDigiCollection::const_iterator first_HF = hfDigis -> begin();
       
    std::vector<HcalDetId> detids;

    ietas_[index] = did.ieta();
    iphis_[index] = did.iphi();
    soiCompressedEts_[index] = tp->SOI_compressedEt();
    soiFineGrains_[index] = tp->SOI_fineGrain();
    presampless_[index] = tp->presamples();

    for (unsigned int i = 0; i < (unsigned int)tp->size(); ++i) {
        compressedEts_[i][index]  = (*tp)[i].compressedEt();
        fineGrains_[i][index]     = (*tp)[i].fineGrain();
        // matching trigger primitives to digi
        
        detids.clear();
        detids = geometry->detIds(did);

        int ndetids = detids.size();

        for (int j = 0; j < ndetids; ++j){
            int digi_index; 
            HcalDetId hcalId = detids[j];
            HcalSubdetector subdet = hcalId.subdet();
      
            if ( subdet == HcalBarrel || subdet == HcalEndcap ){
                iHBHE = hbheDigis->find(hcalId);
                if ( iHBHE != hbheDigis -> end() ){
                    digi_index = iHBHE - first_HBHE;
                    hbheDigiIndexs_[i].push_back(digi_index);
                }
            }
      
            else if ( subdet == HcalForward ) {
                iHF = hfDigis->find (hcalId);
                if ( iHF != hfDigis -> end() ){
                    digi_index = iHF - first_HF;
                    hfDigiIndexs_[i].push_back(digi_index);
                }
            }
        }
        
        //hbheDigiIndexs_[i][index] = 0;
        //hfDigiIndexs_[i][index]   = 0;

    }
    valids_[index] = true;
}


void hcalnano::HcalTrigPrimTable::reset() {
    std::fill(ietas_.begin(), ietas_.end(), 0);
    std::fill(iphis_.begin(), iphis_.end(), 0);
    std::fill(soiCompressedEts_.begin(), soiCompressedEts_.end(), 0);
    std::fill(soiFineGrains_.begin(), soiFineGrains_.end(), 0);
    std::fill(presampless_.begin(), presampless_.end(), 0);
    std::fill(valids_.begin(), valids_.end(), false);

    for (auto& it : compressedEts_) {
        std::fill(it.begin(), it.end(), 0);
    }
    for (auto& it : fineGrains_) {
        std::fill(it.begin(), it.end(), 0);
    }
    for (auto& it : hbheDigiIndexs_) {
        std::fill(it.begin(), it.end(), 0);
    }
    for (auto& it : hfDigiIndexs_) {
        std::fill(it.begin(), it.end(), 0);
    }
}
