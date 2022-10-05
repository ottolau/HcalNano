// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"

#include "FWCore/Utilities/interface/ESInputTag.h"
#include "FWCore/Utilities/interface/Transition.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "DataFormats/NanoAOD/interface/FlatTable.h"

#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "CommonTools/Utils/interface/StringObjectFunction.h"

#include "Geometry/HcalTowerAlgo/interface/HcalTrigTowerGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "DataFormats/HcalDetId/interface/HcalTrigTowerDetId.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DQM/HcalCommon/interface/Utilities.h"

#include "HCALPFG/HcalNano/interface/HcalTrigPrimTable.h"

namespace hcalnano {
    class HcalTrigPrimTableProducer : public edm::stream::EDProducer<> {
    private:

        edm::ESGetToken<HcalDbService, HcalDbRecord> tokenHcalDbService_;
        edm::ESHandle<HcalDbService> dbService_;

        edm::InputTag tagTP_;
        edm::EDGetTokenT<HcalTrigPrimDigiCollection> tokenTP_;

        edm::InputTag tagHBHE_;
        edm::EDGetTokenT<HBHEDigiCollection> tokenHBHE_;

        edm::InputTag tagHF_;
        edm::EDGetTokenT<HFDigiCollection> tokenHF_;

        hcalnano::HcalTrigPrimTable *hcalTrigPrimTable_;

    public:
      explicit HcalTrigPrimTableProducer(const edm::ParameterSet& iConfig) : 
        tagTP_(iConfig.getUntrackedParameter<edm::InputTag>("source")),
        tagHBHE_(iConfig.getUntrackedParameter<edm::InputTag>("hbheDigis")),
        tagHF_(iConfig.getUntrackedParameter<edm::InputTag>("hfDigis"))
        {

        tokenTP_    = consumes<HcalTrigPrimDigiCollection>(tagTP_);
        tokenHBHE_  = consumes<HBHEDigiCollection>(tagHBHE_);
        tokenHF_    = consumes<HFDigiCollection>(tagHF_);

        produces<nanoaod::FlatTable>("HcalTrigPrimTable");

      }

        ~HcalTrigPrimTableProducer() {
            delete hcalTrigPrimTable_;
        };

    private:
        void beginRun(edm::Run const&, edm::EventSetup const&);
        void produce(edm::Event&, edm::EventSetup const&) override;

    };
}

void hcalnano::HcalTrigPrimTableProducer::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {
    // TODO
    // List DetIds of interest from emap
    dbService_ = iSetup.getHandle(tokenHcalDbService_);
    emap_ = dbService_->getHcalMapping();

    //std::vector<HcalGenericDetId> dids = emap_->allPrecisionId();
    edm::Handle<hcalnano::HcalChannelInfo> channelInfo;
    iRun.getByToken(tokenChannelInfo_, channelInfo);

    dids_ = channelInfo->dids;
    //eids_ = channelInfo->eids;

    // Create persistent digi storage
    hbDigiTable_ = new hcalnano::HBDigiTable(dids_[HcalBarrel], 8);
    heDigiTable_ = new hcalnano::HEDigiTable(dids_[HcalEndcap], 8);
    hfDigiTable_ = new hcalnano::HFDigiTable(dids_[HcalForward], 3);
    hoDigiTable_ = new hcalnano::HODigiTable(dids_[HcalOuter], 10);
}


void hcalnano::HcalTrigPrimTableProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
    // * Load trigger primitives and digis */
    edm::ESHandle<HcalTrigTowerGeometry> geometry;
    iSetup.get<CaloGeometryRecord>().get(geometry);

    edm::Handle<HBHEDigiCollection> hbheDigis;
    iEvent.getByToken(tokenHBHE_, hbheDigis);

    edm::Handle<HFDigiCollection> hfDigis;
    iEvent.getByToken(tokenHF_, hfDigis);

    edm::Handle<HcalTrigPrimDigiCollection> tps;
    iEvent.getByToken(tokenTP_, tps);

    // * Process digis */
    hcalTrigPrimTable_->reset();
    for (HcalTrigPrimDigiCollection::const_iterator itp = tps->begin(); itp != tps->end(); ++itp) {
        const HcalTriggerPrimitiveDigi tp = static_cast<const HcalTriggerPrimitiveDigi>(*itp);
        hcalTrigPrimTable_->add(&tp, hbheDigis, hfDigis, geometry);
    } // End loop over hcal trigger primitives

    // * Save to NanoAOD tables */

    // TODO
    // HB
    auto hbNanoTable = std::make_unique<nanoaod::FlatTable>(dids_[HcalBarrel].size(), "HBDigis", false, false);
    hbNanoTable->addColumn<int>("rawId", hbDigiTable_->rawIds_, "rawId");
    hbNanoTable->addColumn<int>("ieta", hbDigiTable_->ietas_, "ieta");
    hbNanoTable->addColumn<int>("iphi", hbDigiTable_->iphis_, "iphi");
    hbNanoTable->addColumn<int>("depth", hbDigiTable_->depths_, "depth");
    hbNanoTable->addColumn<int>("subdet", hbDigiTable_->subdets_, "subdet");
    hbNanoTable->addColumn<bool>("linkError", hbDigiTable_->linkErrors_, "linkError");
    hbNanoTable->addColumn<bool>("capidError", hbDigiTable_->capidErrors_, "capidError");
    hbNanoTable->addColumn<int>("flags", hbDigiTable_->flags_, "flags");
    hbNanoTable->addColumn<int>("soi", hbDigiTable_->sois_, "soi");
    hbNanoTable->addColumn<bool>("valid", hbDigiTable_->valids_, "valid");

    for (unsigned int iTS = 0; iTS < 8; ++iTS) {
        hbNanoTable->addColumn<int>(std::string("adc") + std::to_string(iTS), 
                                        hbDigiTable_->adcs_[iTS], 
                                        std::string("adc") + std::to_string(iTS));
        hbNanoTable->addColumn<int>(std::string("tdc") + std::to_string(iTS), 
                                        hbDigiTable_->tdcs_[iTS], 
                                        std::string("tdc") + std::to_string(iTS));
        hbNanoTable->addColumn<int>(std::string("capid") + std::to_string(iTS), 
                                        hbDigiTable_->capids_[iTS], 
                                        std::string("capid") + std::to_string(iTS));
        hbNanoTable->addColumn<float>(std::string("fc") + std::to_string(iTS), 
                                        hbDigiTable_->fcs_[iTS], 
                                        std::string("fc") + std::to_string(iTS));
        hbNanoTable->addColumn<float>(std::string("pedestalfc") + std::to_string(iTS), 
                                        hbDigiTable_->pedestalfcs_[iTS], 
                                        std::string("pedestalfc") + std::to_string(iTS));
    }
    iEvent.put(std::move(hbNanoTable), "HBDigiTable");

}

#include "FWCore/Framework/interface/MakerMacros.h"
//define this as a plug-in
DEFINE_FWK_MODULE(hcalnano::HcalTrigPrimTableProducer);
