#include "bTag/CommissioningCommonSetup/interface/GenericTrackSelector.h"
#include "bTag/CommissioningCommonSetup/interface/generateName.h"

#include <cstdlib>

GenericTrackSelector::GenericTrackSelector() : BaseSelector(), histograms() {} 

GenericTrackSelector::GenericTrackSelector(const Information1D& info_): BaseSelector(), nJets(0), nTracks(0),
   MCTrueFlavor(0), trackJetIndex(0), isBGluonSplitting(0), info(info_), fFormula(0),
  fSelectionJet(0), fSelectionTrack(0),
  histograms(info_.getPlotName(), info_.getPlotTitle(), info_.getNBinsX(), info_.getXLow(), info_.getXUp())
{
  histograms.setXTitle(info_.getXTitle());
  histograms.setYTitle(info_.getYTitle());
  histograms.setBScale(info_.getBScale());
  histograms.setCScale(info_.getCScale());
  histograms.sumW2();
}

void GenericTrackSelector::Init(TTree* tree)
{
  if (!tree) return;
  fChain = tree;
  fChain->SetMakeClass(1);
  treeWeight = fChain->GetWeight();

  nJets = SetPtrToValue<Int_t>(b_nJets, "nJets");
  nTracks = SetPtrToValue<Int_t>(b_nTracks, "nTracks");
  MCTrueFlavor = SetPtrToValue<Int_t>(b_MCTrueFlavor, "MCTrueFlavor");
  trackJetIndex = SetPtrToValue<Int_t>(b_trackJetIndex, "trackJetIndex");
  isBGluonSplitting = SetPtrToValue<Bool_t>(b_isBGluonSplitting, "isBGluonSplitting");

  if(fFormula == 0)
  {
    fFormula = new TTreeFormula(generateName().c_str(), info.getAliasX().c_str(), fChain);
    fFormula->SetQuickLoad(true);
  }
  else
  {
    fFormula->SetTree(fChain);
  }

  if(fSelectionJet == 0 && !info.getJetCut().empty())
  {
    fSelectionJet = new TTreeFormula(generateName().c_str(), info.getJetCut().c_str(), fChain);
    fSelectionJet->SetQuickLoad(true);
  }
  else if(fSelectionJet)
  {
    fSelectionJet->SetTree(fChain);
  }

  if(fSelectionTrack == 0 && !info.getTrackCut().empty())
  {
    fSelectionTrack = new TTreeFormula(generateName().c_str(), info.getTrackCut().c_str(), fChain);
    fSelectionTrack->SetQuickLoad(true);
  }
  else if(fSelectionTrack)
  {
    fSelectionTrack->SetTree(fChain);
  }
}

Bool_t GenericTrackSelector::Notify()
{
  if(fFormula) fFormula->Notify();
  if(fSelectionJet) fSelectionJet->Notify();
  if(fSelectionTrack) fSelectionTrack->Notify();

  return kTRUE;
}

Bool_t GenericTrackSelector::Process(Long64_t entry)
{
  if(!fFormula)
    return kTRUE;

  b_nJets->GetEntry(entry);
  b_nTracks->GetEntry(entry);
  b_MCTrueFlavor->GetEntry(entry);
  b_isBGluonSplitting->GetEntry(entry);
  b_trackJetIndex->GetEntry(entry);

  int nDataJet = fSelectionJet ? fSelectionJet->GetNdata() : *nJets;
  int nDataTrack = fSelectionTrack ? fSelectionTrack->GetNdata() : *nTracks;
  fFormula->GetNdata();

  //This is needed to ensure the needed
  //branches are loaded.
  fFormula->EvalInstance(0);
  if(fSelectionTrack) fSelectionTrack->EvalInstance(0);
  for(int i = 0; i != nDataJet; ++i)
  {
    if(!fSelectionJet || fSelectionJet->EvalInstance(i))
    {
      for(int j = 0; j != nDataTrack; ++j)
      {
        if(trackJetIndex[j] == i && (!fSelectionTrack || fSelectionTrack->EvalInstance(j)))
          histograms.fill(fFormula->EvalInstance(j), treeWeight, isData, MCTrueFlavor[i], *isBGluonSplitting);
      }
    }
  }

  return kTRUE;
}
