#include <map>
#include <string>

#include "TH1.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/PatCandidates/interface/Muon.h"

#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"

class ZPeakAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {

public:
   explicit ZPeakAnalyzer(const edm::ParameterSet&);
   ~ZPeakAnalyzer() {};
  
private:
   virtual void beginJob() ;
   virtual void analyze(const edm::Event&, const edm::EventSetup&);
   virtual void endJob() ;
  
   // map to contain all histograms; 
   // histograms are booked in beginJob() 
   std::map<std::string, TH1D*> histograms_; 

   // Tokens, used to load object from input files
   // Used by the CMSSW framework to arrange modules in order (important for multithreading)
   edm::EDGetTokenT<pat::MuonCollection> muonToken_;
 
   // Input tags, used to identify input objects in the input file
   // See https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideEDMGetDataFromEvent#Labels
   edm::InputTag muonTag_;
};

ZPeakAnalyzer::ZPeakAnalyzer(const edm::ParameterSet& iConfig):
      histograms_(),
      muonTag_(iConfig.getUntrackedParameter<edm::InputTag>("muonTag")) {

   // Declare that this module uses the TFileService (important for multithreading, as TFileService is not thread safe)
   usesResource("TFileService");

   // Again, tokens are used to load objects from input files, and to inform the framework which input objects are needed for this module.
   muonToken_ = consumes<pat::MuonCollection>(muonTag_);
}

void ZPeakAnalyzer::beginJob() {
   // The TFileService lets us save ROOT objects to a TFile
   edm::Service<TFileService> fs;

   // Create histograms using TFileService method
   histograms_["dimuonMass"]    = fs->make<TH1D>("dimuonMass", "mass", 120, 30., 150.);
   histograms_["nMuon"]       = fs->make<TH1D>("nMuon", "Number of muons", 11, -0.5, 10.5); // Better to have integers at bin centers, not edges
   histograms_["muonPt"]      = fs->make<TH1D>("muonPt", "Muon p_{T}", 200, 0, 200);
   histograms_["muonEta"]     = fs->make<TH1D>("muonEta", "Muon #eta", 100, -5,  5);
   histograms_["muonPhi"]     = fs->make<TH1D>("muonPhi",   "muon Phi", 100, -3.5, 3.5);   
}

void ZPeakAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
   // Get pat::Muon collection 
   edm::Handle<std::vector<pat::Muon>> muons;
   iEvent.getByToken(muonToken_, muons);

   // Apply muon selection
   std::vector<pat::Muon> selectedMuons; 
   for (auto& it_mu : *muons) {
      if (it_mu.pt() > 20 && fabs(it_mu.eta() < 2.1)) {
         selectedMuons.push_back(it_mu);
      }
   }

   // Fill histogram for number of muons
   histograms_["nMuon"]->Fill(selectedMuons.size());

   // Fill single-muon histograms
   for (auto& muon : selectedMuons) { 
      histograms_["muonPt"]->Fill(muon.pt());
      histograms_["muonEta"]->Fill(muon.eta());
      histograms_["muonPhi"]->Fill(muon.phi());
   }

   // Loops over all pairs of selected muons
   if (selectedMuons.size() >= 2) {
      for (auto it_mu1 = selectedMuons.begin(); it_mu1 != selectedMuons.end(); it_mu1++) {
         for (auto it_mu2 = it_mu1 + 1; it_mu2 != selectedMuons.end(); it_mu2++) {
            // Require opposite sign
            if (it_mu1->charge() == it_mu2->charge()) {
               continue;
            }

            // Compute dimuon mass and fill histogram
            double dimuonMass = (it_mu1->p4() + it_mu2->p4()).mass();
            histograms_["dimuonMass"]->Fill(dimuonMass);
         }
      }
   }
}

void ZPeakAnalyzer::endJob() {}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(ZPeakAnalyzer);

