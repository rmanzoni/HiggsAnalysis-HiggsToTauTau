#include <iostream>
#include <algorithm>

#include <TH1F.h>
#include <TFile.h>
#include <TROOT.h>
#include <TString.h>
#include <TSystem.h>
#include <Rtypes.h>

#include <TMath.h>
#include <TAxis.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TAttLine.h>
#include <TPaveText.h>
#include <TColor.h>

#include "$CMSSW_BASE/src/HiggsAnalysis/HiggsToTauTau/interface/HttStyles.h"
#include "$CMSSW_BASE/src/HiggsAnalysis/HiggsToTauTau/src/HttStyles.cc"

static const float SIGNAL_SCALE = 1.;

$DEFINE_ASIMOV
$DEFINE_DROP_SIGNAL
$DEFINE_MSSM

/**
   \class   HTT_MM_X_template HTT_MM_X_template.C "HiggsAnalysis/HiggsToTauTau/postfit/templates/HTT_MM_X_template.C"

   \brief   macro template to create pre-/postfit plots of the inputs to the limit calculation

   This is a macro template to create pre-/postfit plots of the inputs to the limit calculation.
   This macro is picked up from the produce_macros.py script in the HiggsAnalysis/HiggsToTauTau/test
   directory. The key words are replaced by proper values. 
*/

static const bool BLIND_DATA = $BLIND; //false;
static const bool FULLPLOTS = true; //true;
static const bool CONVERVATIVE_CHI2 = false;
static const float UPPER_EDGE = 1495; // 695; 1495;

float blinding_SM(float mass){
  bool blind=false;
  // blinding based on final discriminator values
  if((std::string("$CATEGORY").find(std::string("1jet"))!=std::string::npos) && (0.6<mass)){blind=true;}
  if((std::string("$CATEGORY").find(std::string("vbf" ))!=std::string::npos) && (0.5<mass)){blind=true;}
  return blind;
}
float blinding_MSSM(float mass){ return (100<mass); }
float maximum(TH1F* h, bool LOG=false){
  if(LOG){
    if(h->GetMaximum()>1000){ return 1000.*TMath::Nint(130*h->GetMaximum()/1000.); }
    if(h->GetMaximum()>  10){ return   10.*TMath::Nint(30*h->GetMaximum()/  10.); }
    return 50*h->GetMaximum(); 
  }
  else{
    if(h->GetMaximum()>  12){ return 10.*TMath::Nint((1.20*h->GetMaximum()/10.)); }
    if(h->GetMaximum()> 1.2){ return TMath::Nint((1.50*h->GetMaximum())); }
    return 1.6*h->GetMaximum(); 
  }
}

TH1F* refill(TH1F* hin, const char* sample, bool data=false)
/*
  refill histograms, divide by bin width and correct bin errors. For MC histograms set 
  bin errors to zero.
*/
{
  if(hin==0){
    std::cout << "hist not found: " << sample << "  -- close here" << std::endl;
    exit(1);  
  }
  TH1F* hout = (TH1F*)hin->Clone(); hout->Clear();
  for(int i=0; i<hout->GetNbinsX(); ++i){
    if(data){
#if defined MSSM
      hout->SetBinContent(i+1, BLIND_DATA && blinding_MSSM(hin->GetBinCenter(i+1)) ? 0. : hin->GetBinContent(i+1)/hin->GetBinWidth(i+1));
      hout->SetBinError  (i+1, BLIND_DATA && blinding_MSSM(hin->GetBinCenter(i+1)) ? 0. : hin->GetBinError(i+1)/hin->GetBinWidth(i+1));
#else
      hout->SetBinContent(i+1, BLIND_DATA && blinding_SM  (hin->GetBinCenter(i+1)) ? 0. : hin->GetBinContent(i+1)/hin->GetBinWidth(i+1));
      hout->SetBinError  (i+1, BLIND_DATA && blinding_SM  (hin->GetBinCenter(i+1)) ? 0. : hin->GetBinError(i+1)/hin->GetBinWidth(i+1));
#endif
    }
    else{
      hout->SetBinContent(i+1, hin->GetBinContent(i+1)/hin->GetBinWidth(i+1));
      hout->SetBinError(i+1, 0.);
    }
  }
  return hout;
}

void rescale(TH1F* hin, unsigned int idx)
/*
  rescale histograms according to fit results. The keywords like $Ztt will be replaced 
  by a cout statement and a scaling command.
*/
{
  switch(idx){
  case 1: // ZTT 
  $ZTT  
  case 2: // ZMM 
  $ZMM 
  case 3: // TTJ 
  $TTJ
  case 4: // QCD
  $QCD
  case 5: // Dibosons
  $Dibosons
  case 6: // WJets
  $WJets
#if defined MSSM
  case 7: // ggH
  ${MSSM}ggH$MA
  case 8: // bbH
  ${MSSM}bbH$MA
#else
#ifndef DROP_SIGNAL
  case 7: // ggH
  ${SM}ggH125
  case 8: // qqH
  ${SM}qqH125
  case 9: // VH
  ${SM}VH125
#endif
#endif
  default :
    std::cout << "error histograms not known?!?" << std::endl;
  }
}

void 
HTT_MM_X(bool scaled=true, bool log=true, float min=0.1, float max=-1., string inputfile="root/$HISTFILE", const char* directory="mumu_$CATEGORY")
{
  // define common canvas, axes pad styles
  SetStyle(); gStyle->SetLineStyleString(11,"20 10");

  // determine category tag
  const char* category = ""; const char* category_extra = ""; const char* category_extra2 = "";
  if(std::string(directory) == std::string("mumu_0jet_low"             )){ category = "#mu#mu";          }
  if(std::string(directory) == std::string("mumu_0jet_low"             )){ category_extra = "0-jet low p_{T}^{#mu}";          }
  if(std::string(directory) == std::string("mumu_0jet_high"            )){ category = "#mu#mu";          }
  if(std::string(directory) == std::string("mumu_0jet_high"            )){ category_extra = "0-jet high p_{T}^{#mu}";         }
  if(std::string(directory) == std::string("mumu_1jet_low"          )){ category = "#mu#mu";          }
  if(std::string(directory) == std::string("mumu_1jet_low"          )){ category_extra = "1-jet low p_{T}^{#mu}";       }
  if(std::string(directory) == std::string("mumu_1jet_high"          )){ category = "#mu#mu";          }
  if(std::string(directory) == std::string("mumu_1jet_high"          )){ category_extra = "1-jet high p_{T}^{#mu}";       }
  if(std::string(directory) == std::string("mumu_vbf"            )){ category = "#mu#mu";          }
  if(std::string(directory) == std::string("mumu_vbf"            )){ category_extra = "2-jet";              }
  if(std::string(directory) == std::string("mumu_nobtag"               )){ category = "#mu#mu";          }
  if(std::string(directory) == std::string("mumu_nobtag"               )){ category_extra = "no b-tag";                        }
  if(std::string(directory) == std::string("mumu_btag"                 )){ category = "#mu#mu";          }
  if(std::string(directory) == std::string("mumu_btag"                 )){ category_extra = "b-tag";                           }

  const char* dataset;
#ifdef MSSM
  if(std::string(inputfile).find("7TeV")!=std::string::npos){dataset = "#scale[1.5]{CMS}  h,H,A#rightarrow#tau#tau                                 4.9 fb^{-1} (7 TeV)";}
  if(std::string(inputfile).find("8TeV")!=std::string::npos){dataset = "#scale[1.5]{CMS}  h,H,A#rightarrow#tau#tau                                19.7 fb^{-1} (8 TeV)";}
#else
  if(std::string(inputfile).find("7TeV")!=std::string::npos){dataset = "CMS, 4.9 fb^{-1} at 7 TeV";}
  if(std::string(inputfile).find("8TeV")!=std::string::npos){dataset = "CMS, 19.7 fb^{-1} at 8 TeV";}
#endif
 
  TFile* input = new TFile(inputfile.c_str());
#ifdef MSSM
  TFile* input2 = new TFile((inputfile+"_$MA_$TANB").c_str());
#endif
  TH1F* ZTT      = refill((TH1F*)input ->Get(TString::Format("%s/ZTT"     , directory)), "ZTT"     ); InitHist(ZTT     , "", "", TColor::GetColor(248,206,104), 1001);
  TH1F* ZMM      = refill((TH1F*)input ->Get(TString::Format("%s/ZMM"     , directory)), "ZMM"     ); InitHist(ZMM     , "", "", TColor::GetColor(100,182,232), 1001);
  TH1F* TTJ      = refill((TH1F*)input ->Get(TString::Format("%s/TTJ"     , directory)), "TTJ"     ); InitHist(TTJ     , "", "", TColor::GetColor(155,152,204), 1001);
  TH1F* QCD      = refill((TH1F*)input ->Get(TString::Format("%s/QCD"     , directory)), "QCD"     ); InitHist(QCD     , "", "", TColor::GetColor(250,202,255), 1001);
  TH1F* Dibosons = refill((TH1F*)input ->Get(TString::Format("%s/Dibosons", directory)), "Dibosons"); InitHist(Dibosons, "", "", TColor::GetColor(222,90,106), 1001);
  TH1F* WJets    = 0;
  if(!(std::string("mumu_nobtag") == std::string(directory))){
    // template has been removed from nobtag categories
    WJets = refill((TH1F*)input ->Get(TString::Format("%s/WJets"   , directory)), "WJets"   ); InitHist(WJets   , "", "", kGreen -4 , 1001);
  }
#ifdef MSSM
  TH1F* ggH      = refill((TH1F*)input2->Get(TString::Format("%s/ggH$MA"  , directory)), "ggH"     ); InitSignal(ggH); ggH->Scale($TANB);
  TH1F* bbH      = refill((TH1F*)input2->Get(TString::Format("%s/bbH$MA"  , directory)), "bbH"     ); InitSignal(bbH); bbH->Scale($TANB);
#else
#ifndef DROP_SIGNAL
  TH1F* ggH      = refill((TH1F*)input ->Get(TString::Format("%s/ggH125"  , directory)), "ggH"     ); InitSignal(ggH); ggH->Scale(SIGNAL_SCALE);
  TH1F* qqH      = refill((TH1F*)input ->Get(TString::Format("%s/qqH125"  , directory)), "qqH"     ); InitSignal(qqH); qqH->Scale(SIGNAL_SCALE);
  TH1F* VH       = refill((TH1F*)input ->Get(TString::Format("%s/VH125"   , directory)), "VH"      ); InitSignal(VH ); VH ->Scale(SIGNAL_SCALE);
#endif
#endif
#ifdef ASIMOV
  TH1F* data   = refill((TH1F*)input->Get(TString::Format("%s/data_obs_asimov", directory)), "data", true);
#else
  TH1F* data   = refill((TH1F*)input->Get(TString::Format("%s/data_obs", directory)), "data", true);
#endif
#ifdef MSSM
  InitHist(data, "#bf{m_{#tau#tau} [GeV]}" , "#bf{dN/dm_{#tau#tau} [1/GeV]}"); InitData(data);
#else
  InitHist(data, "#bf{D}", "#bf{dN/dD}"     ); InitData(data);
#endif

  TH1F* ref=(TH1F*)ZTT->Clone("ref");
  ref->Add(ZMM);
  ref->Add(TTJ);
  ref->Add(QCD);
  ref->Add(Dibosons);
  if(WJets){
    ref->Add(WJets);
  }
  double unscaled[9];
  unscaled[0] = ZTT->Integral();
  unscaled[1] = ZMM->Integral();
  unscaled[2] = TTJ->Integral();
  unscaled[3] = QCD->Integral();
  unscaled[4] = Dibosons->Integral();
  unscaled[5] = 0;
  if(WJets){
    unscaled[5] = WJets->Integral();
  }
#ifdef MSSM
  unscaled[6] = ggH->Integral();
  unscaled[7] = bbH->Integral();
  unscaled[8] = 0;
#else
#ifndef DROP_SIGNAL
  unscaled[6] = ggH->Integral();
  unscaled[7] = qqH->Integral();
  unscaled[8] = VH ->Integral();
#endif
#endif
  
  if(scaled){
    rescale(ZTT,  1); 
    rescale(ZMM,  2); 
    rescale(TTJ,  3); 
    rescale(QCD,  4); 
    rescale(Dibosons, 5); 
    if(WJets){
      rescale(WJets,  6);
    }
#ifdef MSSM 
    rescale(ggH,  7);
    rescale(bbH,  8);
#else
#ifndef DROP_SIGNAL
    rescale(ggH,  7);
    rescale(qqH,  8);
    rescale(VH,   9);
#endif
#endif
  }

  TH1F* scales[9];
  scales[0] = new TH1F("scales-ZTT", "", 9, 0, 9);
  scales[0]->SetBinContent(1, unscaled[0]>0 ? (ZTT->Integral()/unscaled[0]-1.)      : 0.);
  scales[1] = new TH1F("scales-ZMM"  , "", 9, 0, 9);
  scales[1]->SetBinContent(2, unscaled[1]>0 ? (ZMM->Integral()/unscaled[1]-1.)      : 0.);
  scales[2] = new TH1F("scales-TTJ", "", 9, 0, 9);
  scales[2]->SetBinContent(3, unscaled[2]>0 ? (TTJ->Integral()/unscaled[2]-1.)      : 0.);
  scales[3] = new TH1F("scales-QCD"  , "", 9, 0, 9);
  scales[3]->SetBinContent(4, unscaled[3]>0 ? (QCD->Integral()/unscaled[3]-1.)      : 0.);
  scales[4] = new TH1F("scales-Dibosons", "", 9, 0, 9);
  scales[4]->SetBinContent(5, unscaled[4]>0 ? (Dibosons->Integral()/unscaled[4]-1.) : 0.);
  scales[5] = new TH1F("scales-WJets"  , "", 9, 0, 9); scales[5]->SetBinContent(6, 0.);
  if(WJets){ 
    scales[5]->SetBinContent(6, unscaled[5]>0 ? (WJets->Integral()/unscaled[5]-1.)  : 0.);
  }
#ifdef MSSM
  scales[6] = new TH1F("scales-ggH"  , "", 9, 0, 9);
  scales[6]->SetBinContent(7, unscaled[6]>0 ? (ggH->Integral()/unscaled[6]-1.)      : 0.);
  scales[7] = new TH1F("scales-bbH"  , "", 9, 0, 9);
  scales[7]->SetBinContent(8, unscaled[7]>0 ? (bbH->Integral()/unscaled[7]-1.)      : 0.);
  scales[8] = new TH1F("scales-NONE" , "", 9, 0, 9);
  scales[8]->SetBinContent(9, 0.);
#else
#ifndef DROP_SIGNAL
  scales[6] = new TH1F("scales-ggH"  , "", 9, 0, 9);
  scales[6]->SetBinContent(7, unscaled[6]>0 ? (ggH->Integral()/unscaled[6]-1.)      : 0.);
  scales[7] = new TH1F("scales-qqH"  , "", 9, 0, 9);
  scales[7]->SetBinContent(8, unscaled[7]>0 ? (qqH->Integral()/unscaled[7]-1.)      : 0.);
  scales[8] = new TH1F("scales-VH"   , "", 9, 0, 9);
  scales[8]->SetBinContent(9, unscaled[8]>0 ? (VH ->Integral()/unscaled[8]-1.)      : 0.);
#endif
#endif

  if(WJets){
    Dibosons->Add(WJets);
  }
  QCD->Add(Dibosons);
  TTJ->Add(QCD);
  ZTT->Add(TTJ);
  ZMM->Add(ZTT);
  if(log){
#ifdef MSSM
    ggH  ->Add(bbH);
#else
#ifndef DROP_SIGNAL
    qqH  ->Add(VH );
    ggH  ->Add(qqH);
#endif
#endif
  }
  else{
#ifdef MSSM
    bbH  ->Add(ZMM);
    ggH  ->Add(bbH);
#else
#ifndef DROP_SIGNAL
    VH   ->Add(ZMM);
    qqH  ->Add(VH );
    ggH  ->Add(qqH);
#endif
#endif
  }


  /*
    mass plot before and after fit
  */
  TCanvas* canv = MakeCanvas("canv", "histograms", 600, 600);
  canv->cd();
  if(log){ canv->SetLogy(1); }
#if defined MSSM
  if(!log){ data->GetXaxis()->SetRange(0, data->FindBin(345)); } else{ data->GetXaxis()->SetRange(0, data->FindBin(UPPER_EDGE)); };
#else
  data->GetXaxis()->SetRange(0, data->FindBin(345));
#endif
  data->SetNdivisions(505);
  data->SetMinimum(min);
#ifndef DROP_SIGNAL
  data->SetMaximum(max>0 ? max : std::max(std::max(maximum(data, log), maximum(ZMM, log)), maximum(ggH, log)));
#else
  data->SetMaximum(max>0 ? max : std::max(maximum(data, log), maximum(ZMM, log)));
#endif
  data->Draw("e");

  TH1F* errorBand = (TH1F*)ZMM->Clone("errorBand");
  errorBand->SetMarkerSize(0);
  errorBand->SetFillColor(13);
  errorBand->SetFillStyle(3013);
  errorBand->SetLineWidth(1);
  for(int idx=0; idx<errorBand->GetNbinsX(); ++idx){
    if(errorBand->GetBinContent(idx)>0){
      std::cout << "Uncertainties on summed background samples: " << errorBand->GetBinError(idx)/errorBand->GetBinContent(idx) << std::endl;
      break;
    }
  }
  if(log){
    ZMM->Draw("histsame");
    ZTT->Draw("histsame");
    TTJ->Draw("histsame");
    QCD->Draw("histsame");
    Dibosons->Draw("histsame");
    $DRAW_ERROR
#ifndef DROP_SIGNAL
    ggH->Draw("histsame");
#endif

  }
  else{
#ifndef DROP_SIGNAL
    ggH  ->Draw("histsame");
#endif
    ZMM->Draw("histsame");
    ZTT->Draw("histsame");
    TTJ->Draw("histsame");
    QCD->Draw("histsame");
    Dibosons->Draw("histsame");
    $DRAW_ERROR
  }
  data->Draw("esame");
  canv->RedrawAxis();

  //CMSPrelim(dataset, "#tau_{#mu}#tau_{#mu}", 0.17, 0.835);
  CMSPrelim(dataset, "", 0.16, 0.835);
#if defined MSSM
  TPaveText* chan     = new TPaveText(0.20, 0.74+0.061, 0.32, 0.74+0.161, "tlbrNDC");
#else
  TPaveText* chan     = new TPaveText(0.22, (category_extra2 && category_extra2[0]=='\0') ? 0.65+0.061 : 0.65+0.061, 0.34, 0.75+0.161, "tlbrNDC");
#endif
  chan->SetBorderSize(   0 );
  chan->SetFillStyle(    0 );
  chan->SetTextAlign(   12 );
  chan->SetTextSize ( 0.05 );
  chan->SetTextColor(    1 );
  chan->SetTextFont (   62 );
  chan->AddText(category);
  chan->AddText(category_extra);
#if defined MSSM
#else
  chan->AddText(category_extra2);
#endif
  chan->Draw();

/*  TPaveText* cat      = new TPaveText(0.20, 0.71+0.061, 0.32, 0.71+0.161, "NDC");
  cat->SetBorderSize(   0 );
  cat->SetFillStyle(    0 );
  cat->SetTextAlign(   12 );
  cat->SetTextSize ( 0.05 );
  cat->SetTextColor(    1 );
  cat->SetTextFont (   62 );
  cat->AddText(category_extra);
  cat->Draw();
*/
#ifdef MSSM
  TPaveText* massA      = new TPaveText(0.53, 0.44+0.061, 0.95, 0.44+0.151, "NDC");
  massA->SetBorderSize(   0 );
  massA->SetFillStyle(    0 );
  massA->SetTextAlign(   12 );
  massA->SetTextSize ( 0.03 );
  massA->SetTextColor(    1 );
  massA->SetTextFont (   62 );
  massA->AddText("MSSM m^{h}_{max} scenario");
  massA->AddText("m_{A}=$MA GeV, tan#beta=$TANB");
  massA->Draw();
#endif

#ifdef MSSM  
  TLegend* leg = new TLegend(0.53, 0.60, 0.95, 0.90);
  SetLegendStyle(leg);
  leg->AddEntry(ggH  , "h,H,A#rightarrow#tau#tau" , "L" );
#else
  TLegend* leg = new TLegend(0.52, 0.58, 0.92, 0.89);
  SetLegendStyle(leg);
#ifndef DROP_SIGNAL
  if(SIGNAL_SCALE!=1){
    leg->AddEntry(ggH  , TString::Format("%.0f#timesH(125 GeV)#rightarrow#tau#tau", SIGNAL_SCALE) , "L" );
  }
  else{
    leg->AddEntry(ggH  , "SM H(125 GeV)#rightarrow#tau#tau" , "L" );
  }
#endif
#endif
#ifdef ASIMOV
  leg->AddEntry(data    , "sum(bkg) + H(125)"           , "LP");
#else
  leg->AddEntry(data    , "Observed"                    , "LP");
#endif
  leg->AddEntry(ZMM     , "Z#rightarrow#mu#mu"          , "F" );
  leg->AddEntry(ZTT     , "Z#rightarrow#tau#tau"        , "F" );
  leg->AddEntry(TTJ     , "t#bar{t}"                    , "F" );
  leg->AddEntry(QCD     , "QCD"                         , "F" );
  leg->AddEntry(Dibosons, "Electroweak"                 , "F" );
  $ERROR_LEGEND
  leg->Draw();

  /*
    Ratio Data over MC
  */
  TCanvas *canv0 = MakeCanvas("canv0", "histograms", 600, 400);
  canv0->SetGridx();
  canv0->SetGridy();
  canv0->cd();

  TH1F* model = (TH1F*)ZMM ->Clone("model");
  TH1F* test1 = (TH1F*)data->Clone("test1"); 
  for(int ibin=0; ibin<test1->GetNbinsX(); ++ibin){
    //the small value in case of 0 entries in the model is added to prevent the chis2 test from failing
    model->SetBinContent(ibin+1, model->GetBinContent(ibin+1)>0 ? model->GetBinContent(ibin+1)*model->GetBinWidth(ibin+1) : 0.01);
    model->SetBinError  (ibin+1, CONVERVATIVE_CHI2 ? 0. : model->GetBinError  (ibin+1)*model->GetBinWidth(ibin+1));
    test1->SetBinContent(ibin+1, test1->GetBinContent(ibin+1)*test1->GetBinWidth(ibin+1));
    test1->SetBinError  (ibin+1, test1->GetBinError  (ibin+1)*test1->GetBinWidth(ibin+1));
  }
  double chi2prob = test1->Chi2Test      (model,"PUW");        std::cout << "chi2prob:" << chi2prob << std::endl;
  double chi2ndof = test1->Chi2Test      (model,"CHI2/NDFUW"); std::cout << "chi2ndf :" << chi2ndof << std::endl;
  double ksprob   = test1->KolmogorovTest(model);              std::cout << "ksprob  :" << ksprob   << std::endl;
  double ksprobpe = test1->KolmogorovTest(model,"DX");         std::cout << "ksprobpe:" << ksprobpe << std::endl;  

  std::vector<double> edges;
  TH1F* zero = (TH1F*)ref ->Clone("zero"); zero->Clear();
  TH1F* rat1 = (TH1F*)data->Clone("rat1"); rat1->Reset("ICES");
  for(int ibin=0; ibin<rat1->GetNbinsX(); ++ibin){
    if(data->GetBinContent(ibin+1) > 0){
      rat1->SetBinContent(ibin+1, ZMM->GetBinContent(ibin+1)>0 ? data->GetBinContent(ibin+1)/ZMM->GetBinContent(ibin+1) : 0);
      rat1->SetBinError  (ibin+1, ZMM->GetBinContent(ibin+1)>0 ? data->GetBinError  (ibin+1)/ZMM->GetBinContent(ibin+1) : 0);
    }
    zero->SetBinContent(ibin+1, 0.);
    zero->SetBinError  (ibin+1, ZMM->GetBinContent(ibin+1)>0 ? ZMM ->GetBinError  (ibin+1)/ZMM->GetBinContent(ibin+1) : 0);
  }
  for(int ibin=0; ibin<rat1->GetNbinsX(); ++ibin){
    if(rat1->GetBinContent(ibin+1)>0){
      edges.push_back(TMath::Abs(rat1->GetBinContent(ibin+1)-1.)+TMath::Abs(rat1->GetBinError(ibin+1)));
      // catch cases of 0 bins, which would lead to 0-alpha*0-1
      rat1->SetBinContent(ibin+1, rat1->GetBinContent(ibin+1)-1.);
    }
  }
  float range = 0.1;
  std::sort(edges.begin(), edges.end());
  if (edges[edges.size()-2]>0.1) { range = 0.2; }
  if (edges[edges.size()-2]>0.2) { range = 0.5; }
  if (edges[edges.size()-2]>0.5) { range = 1.0; }
  if (edges[edges.size()-2]>1.0) { range = 1.5; }
  if (edges[edges.size()-2]>1.5) { range = 2.0; }
  rat1->SetLineColor(kBlack);
  rat1->SetFillColor(kGray );
  rat1->SetMaximum(+range);
  rat1->SetMinimum(-range);
  rat1->GetYaxis()->CenterTitle();
  rat1->GetYaxis()->SetTitle("#bf{Data/MC-1}");
#ifdef MSSM
  rat1->GetXaxis()->SetTitle("#bf{m_{#tau#tau} [GeV]}"); 
#else
  rat1->GetXaxis()->SetTitle("#bf{D}");
#endif
  rat1->Draw("E0");
  zero->SetFillStyle(  3013);
  zero->SetFillColor(kBlack);
  zero->SetLineColor(kBlack);
  zero->SetMarkerSize(0.1);
  zero->Draw("e2histsame");
  canv0->RedrawAxis();

  TPaveText* stat1 = new TPaveText(0.20, 0.76+0.061, 0.32, 0.76+0.161, "NDC");
  stat1->SetBorderSize(   0 );
  stat1->SetFillStyle(    0 );
  stat1->SetTextAlign(   12 );
  stat1->SetTextSize ( 0.05 );
  stat1->SetTextColor(    1 );
  stat1->SetTextFont (   62 );
  stat1->AddText(TString::Format("#chi^{2}/ndf=%.3f,  P(#chi^{2})=%.3f", chi2ndof, chi2prob));
  //stat1->AddText(TString::Format("#chi^{2}/ndf=%.3f,  P(#chi^{2})=%.3f, P(KS)=%.3f", chi2ndof, chi2prob, ksprob));
  stat1->Draw();

  /*
    Ratio After fit over Prefit
  */
  TCanvas *canv1 = MakeCanvas("canv1", "histograms", 600, 400);
  canv1->SetGridx();
  canv1->SetGridy();
  canv1->cd();

  edges.clear();
  TH1F* rat2 = (TH1F*) ZMM->Clone("rat2");
  for(int ibin=0; ibin<rat2->GetNbinsX(); ++ibin){
    rat2->SetBinContent(ibin+1, ref->GetBinContent(ibin+1)>0 ? ZMM->GetBinContent(ibin+1)/ref->GetBinContent(ibin+1) : 0);
    rat2->SetBinError  (ibin+1, ref->GetBinContent(ibin+1)>0 ? ZMM->GetBinError  (ibin+1)/ref->GetBinContent(ibin+1) : 0);
  }
  for(int ibin=0; ibin<rat2->GetNbinsX(); ++ibin){
    if(rat2->GetBinContent(ibin+1)>0){
      edges.push_back(TMath::Abs(rat2->GetBinContent(ibin+1)-1.)+TMath::Abs(rat2->GetBinError(ibin+1)));
      // catch cases of 0 bins, which would lead to 0-alpha*0-1
      rat2 ->SetBinContent(ibin+1, rat2->GetBinContent(ibin+1)-1.);
    }
  }
  range = 0.1;
  std::sort(edges.begin(), edges.end());
  if (edges[edges.size()-2]>0.1) { range = 0.2; }
  if (edges[edges.size()-2]>0.2) { range = 0.5; }
  if (edges[edges.size()-2]>0.5) { range = 1.0; }
  if (edges[edges.size()-2]>1.0) { range = 1.5; }
  if (edges[edges.size()-2]>1.5) { range = 2.0; }
#if defined MSSM
  if(!log){ rat2->GetXaxis()->SetRange(0, rat2->FindBin(345)); } else{ rat2->GetXaxis()->SetRange(0, rat2->FindBin(UPPER_EDGE)); };
#else
  rat2->GetXaxis()->SetRange(0, rat2->FindBin(345));
#endif
  rat2->SetNdivisions(505);
  rat2->SetLineColor(kRed+3);
  rat2->SetMarkerColor(kRed+3);
  rat2->SetMarkerSize(1.1);
  rat2->SetMaximum(+range);
  rat2->SetMinimum(-range);
  rat2->GetYaxis()->SetTitle("#bf{Postfit/Prefit-1}");
  rat2->GetYaxis()->CenterTitle();
#if defined MSSM
  rat2->GetXaxis()->SetTitle("#bf{m_{#tau#tau} [GeV]}"); 
#else
  rat2->GetXaxis()->SetTitle("#bf{D}");
#endif
  rat2->Draw();
  zero->SetFillStyle(  3013);
  zero->SetFillColor(kBlack);
  zero->SetLineColor(kBlack);
  zero->Draw("e2histsame");
  canv1->RedrawAxis();

  /*
    Relative shift per sample
  */
  TCanvas *canv2 = MakeCanvas("canv2", "histograms", 600, 400);
  canv2->SetGridx();
  canv2->SetGridy();
  canv2->cd();

  InitHist  (scales[0], "", "", TColor::GetColor(248,206,104), 1001);
  InitHist  (scales[1], "", "", TColor::GetColor(100,182,232), 1001);
  InitHist  (scales[2], "", "", TColor::GetColor(155,152,204), 1001);
  InitHist  (scales[3], "", "", TColor::GetColor(250,202,255), 1001);
  InitHist  (scales[4], "", "", TColor::GetColor(222,90,106), 1001);
  InitHist  (scales[5], "", "", kGreen   -  4, 1001);  
#ifndef DROP_SIGNAL
  InitSignal(scales[6]);
  InitSignal(scales[7]);
  InitSignal(scales[8]);
#endif
  scales[0]->GetXaxis()->SetBinLabel(1, "#bf{ZTT}");
  scales[0]->GetXaxis()->SetBinLabel(2, "#bf{ZMM}"  );
  scales[0]->GetXaxis()->SetBinLabel(3, "#bf{TTJ}");
  scales[0]->GetXaxis()->SetBinLabel(4, "#bf{QCD}"  );
  scales[0]->GetXaxis()->SetBinLabel(5, "#bf{Dibosons}");
  scales[0]->GetXaxis()->SetBinLabel(6, "#bf{WJets}"  );
#ifdef MSSM
  scales[0]->GetXaxis()->SetBinLabel(7, "#bf{ggH}"  );
  scales[0]->GetXaxis()->SetBinLabel(8, "#bf{bbH}"  );
  scales[0]->GetXaxis()->SetBinLabel(9, "#bf{NONE}" );
#else
  scales[0]->GetXaxis()->SetBinLabel(7, "#bf{ggH}"  );
  scales[0]->GetXaxis()->SetBinLabel(8, "#bf{qqH}"  );
  scales[0]->GetXaxis()->SetBinLabel(9, "#bf{VH}"   );
#endif
  scales[0]->SetMaximum(+0.5);
  scales[0]->SetMinimum(-0.5);
  scales[0]->GetYaxis()->CenterTitle();
  scales[0]->GetYaxis()->SetTitle("#bf{Postfit/Prefit-1}");
  scales[0]->Draw();
  scales[1]->Draw("same");
  scales[2]->Draw("same");
  scales[3]->Draw("same");
  scales[4]->Draw("same");
  scales[5]->Draw("same");
#ifndef DROP_SIGNAL
  scales[6]->Draw("same");
  scales[7]->Draw("same");
  scales[8]->Draw("same");
#endif
  TH1F* zero_samples = (TH1F*)scales[0]->Clone("zero_samples"); zero_samples->Clear();
  zero_samples->SetBinContent(1,0.);
  zero_samples->Draw("same");
  canv2->RedrawAxis();

  /*
    prepare output
  */
 bool isSevenTeV = std::string(inputfile).find("7TeV")!=std::string::npos;
  canv   ->Print(TString::Format("%s_%sfit_%s_%s.png"       , directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN")); 
  canv   ->Print(TString::Format("%s_%sfit_%s_%s.pdf"       , directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN")); 
  canv   ->Print(TString::Format("%s_%sfit_%s_%s.eps"       , directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN")); 
  if(log || FULLPLOTS)
  {
    canv0->Print(TString::Format("%s_datamc_%sfit_%s_%s.png", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN")); 
    canv0->Print(TString::Format("%s_datamc_%sfit_%s_%s.pdf", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN"));
    canv0->Print(TString::Format("%s_datamc_%sfit_%s_%s.eps", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN"));
  }
  if((log && scaled) || FULLPLOTS)
  {
    canv1->Print(TString::Format("%s_prefit_%sfit_%s_%s.png", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN")); 
    canv1->Print(TString::Format("%s_prefit_%sfit_%s_%s.pdf", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN"));
    canv1->Print(TString::Format("%s_prefit_%sfit_%s_%s.eps", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN"));
    canv2->Print(TString::Format("%s_sample_%sfit_%s_%s.png", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN")); 
    canv2->Print(TString::Format("%s_sample_%sfit_%s_%s.pdf", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN"));
    canv2->Print(TString::Format("%s_sample_%sfit_%s_%s.eps", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN"));
  }

  TFile* output = new TFile(TString::Format("%s_%sfit_%s_%s.root", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN"), "update");
  output->cd();
  data ->Write("data_obs");
  ZTT->Write("Ztt"  );
  ZMM->Write("Zmm"  );
  TTJ->Write("ttbar");
  QCD->Write("Fakes");
  Dibosons->Write("EWK");
  if(WJets){
    WJets->Write("WJets");
  }
#ifdef MSSM
  ggH  ->Write("ggH");
  bbH  ->Write("bbH");
#else
#ifndef DROP_SIGNAL
  ggH  ->Write("ggH");
  qqH  ->Write("qqH");
  VH   ->Write("VH" );
#endif
#endif
  if(errorBand){
    errorBand->Write("errorBand");
  }
  output->Close();
 
  delete errorBand;
  delete model;
  delete test1;
  delete zero;
  delete rat1;
  delete rat2;
  delete zero_samples;
  delete ref;
}
