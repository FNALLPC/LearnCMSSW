import FWCore.ParameterSet.Config as cms

process = cms.Process("process")

process.source = cms.Source("PoolSource",
  fileNames = cms.untracked.vstring(
                'root://cmseos.fnal.gov//store/user/cmsdas/2022/pre_exercises/Set4/Input/DoubleMuon/slimMiniAOD_data_MuEle_1.root', 
              ) # For a local file, do file:/path/to/file.root
)
process.maxEvents = cms.untracked.PSet( 
    input = cms.untracked.int32(-1) 
)

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

process.zpeak_analyzer = cms.EDAnalyzer("ZPeakAnalyzer", muonTag=cms.untracked.InputTag("slimmedMuons"))
process.TFileService = cms.Service("TFileService", fileName=cms.string('ZPeak.root')) # The ZPeakAnalyzer will find this Service using the framework, we don't have to manually connect them here
process.path = cms.Path(process.zpeak_analyzer)
