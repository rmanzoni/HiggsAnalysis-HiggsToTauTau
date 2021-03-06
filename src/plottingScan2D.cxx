#include <iostream>

#include "TH2F.h"
#include "TGraph.h"
#include "TString.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TPaveText.h"
#include "TMarker.h"

void
plotting2DScan(TCanvas& canv, TH2F* plot2D, std::vector<TGraph*> graph95, std::vector<TGraph*> graph68, TGraph* bestfit, TMarker* SMexpected, TMarker* SMexpectedLayer, std::string& xaxis, std::string& yaxis, std::string& masslabel, int mass, double xmin, double xmax, double ymin, double ymax, bool temp=false, bool log=false)
{
  // set up styles
  canv.cd();
  //canv.SetGridx(1);
  //canv.SetGridy(1);
  if(temp){
    canv.SetRightMargin(0.14);
  }
  if(log){ 
    canv.SetLogz(1); 
  }
  
  plot2D->SetXTitle(xaxis.c_str());
  if(xmax>0){
    plot2D->GetXaxis()->SetRange(plot2D->GetXaxis()->FindBin(xmin), plot2D->GetXaxis()->FindBin(xmax)-1);
  }
  plot2D->GetXaxis()->SetLabelFont(62);
  plot2D->GetXaxis()->SetLabelOffset(0.018);
  plot2D->GetXaxis()->SetTitleColor(1);
  plot2D->GetXaxis()->SetTitleOffset(1.05);
  plot2D->GetXaxis()->SetTitleFont(62);
  plot2D->GetXaxis()->SetTitleSize(0.055);
  plot2D->SetYTitle(yaxis.c_str());
  if(ymax>0){
    plot2D->GetYaxis()->SetRange(plot2D->GetYaxis()->FindBin(ymin), plot2D->GetYaxis()->FindBin(ymax)-1);
  }
  plot2D->GetYaxis()->SetLabelFont(62);
  plot2D->GetYaxis()->SetLabelOffset(0.023);
  plot2D->GetYaxis()->SetTitleSize(0.055);
  plot2D->GetYaxis()->SetTitleOffset(1.4);
  plot2D->GetYaxis()->SetTitleFont(62);
  plot2D->GetYaxis()->SetLabelSize(0.04);
  plot2D->SetZTitle("#bf{#Delta NLL}");
  plot2D->GetZaxis()->SetLabelFont(62);
  plot2D->GetZaxis()->SetTitleSize(0.04);
  plot2D->GetZaxis()->SetTitleOffset(1.03);
  plot2D->GetZaxis()->SetLabelSize(0.04);
  plot2D->SetNdivisions(505, "X");
  plot2D->SetNdivisions(505, "Y");
  plot2D->SetMinimum(0.);
  plot2D->Draw("colz");
  
  /// draw contours for 95% CL
  bool FIRST=true; int igraph=0;
  for(std::vector<TGraph*>::const_reverse_iterator g=graph95.rbegin(); g!=graph95.rend(); ++g, ++igraph){
    if(!temp){
      (*g)->SetLineStyle(11); 
      (*g)->SetLineColor(kBlack);
      (*g)->SetLineWidth(3);
      if(FIRST){
	(*g)->SetFillColor(kBlue-10);
	FIRST=false;
      }
      else{
	(*g)->SetFillColor(kWhite);
      }
      (*g)->Draw("fsame"); 
      //(*g)->Draw("contfsame");
      (*g)->Draw("contsame");
    }
    else{
      (*g)->SetLineStyle(11); 
      (*g)->SetLineColor(kBlack); 
      (*g)->SetLineWidth(3);
      (*g)->Draw("contsame");
    }
  }
  /// draw contours for 68% CL
  for(std::vector<TGraph*>::const_reverse_iterator g=graph68.rbegin(); g!=graph68.rend(); ++g){
    //if(*g == graph68.begin()){continue;}
    if(!temp){ 
      (*g)->SetLineStyle(11); 
      (*g)->SetLineColor(kBlack); 
      (*g)->SetLineWidth(3);
      (*g)->SetFillColor(kBlue-8);
      //(*g)->Draw("fsame");
      (*g)->Draw("contfsame"); 
      (*g)->Draw("contsame");
    }
    else{
      (*g)->SetLineStyle(1); 
      (*g)->SetLineColor(kBlack); 
      (*g)->SetLineWidth(3);
      (*g)->Draw("contsame");
    }
  }
  /// draw best fit
  TLegend* leg;
  if(SMexpected && masslabel=="m_{#phi}") leg = new TLegend(0.55, 0.60, 0.90, 0.90);
  else leg = new TLegend(0.60, 0.70, 0.90, 0.90);
  leg->SetBorderSize( 0 );
  leg->SetFillStyle ( 0 );
  leg->SetFillColor (kWhite);
  if(!graph95.empty()){
    TGraph *graph95_copy = (TGraph*)graph95.back()->Clone();
    graph95_copy->SetLineWidth(0);
    graph95_copy->SetLineColor(0);
    graph95_copy->SetLineStyle(1);
    leg->AddEntry(graph95_copy, "95% CL", temp ? "L" : "F"); }
  if(!graph68.empty()){
    TGraph *graph68_copy = (TGraph*)graph68.back()->Clone();
    graph68_copy->SetLineWidth(0);
    graph68_copy->SetLineColor(0);
    graph68_copy->SetLineStyle(1);
    leg->AddEntry(graph68_copy, "68% CL", temp ? "L" : "F"); }
  if(bestfit){ leg->AddEntry(bestfit, "Best fit", "P"); }
  if(SMexpected && masslabel=="m_{H}"){ leg->AddEntry(SMexpected, "SM", "P"); }	
  if(SMexpected && masslabel=="m_{#phi}"){ leg->AddEntry(SMexpected, "Expected for", "P"); } 
  if(SMexpected && masslabel=="m_{#phi}"){ leg->AddEntry((TObject*)0, "SM H(125 GeV)", ""); }  												
  leg->Draw("same");
   
  //Horrible hack to get 2 diamonds to show up as one marker in legend. The bigger diamond is part of the legend, 
  //and this code positions the second diamond on top of it.
  if(SMexpectedLayer){
    canv.Update();
    SMexpectedLayer->DrawMarker(0.554*gPad->GetUxmax(),0.708*gPad->GetUymax());
  }
  
  TString label = TString::Format("%s = %d GeV", masslabel.c_str(), mass);
  TPaveText* textlabel = new TPaveText(0.18, 0.81, 0.50, 0.90, "NDC");
  textlabel->SetBorderSize(   0 );
  textlabel->SetFillStyle (   0 );
  textlabel->SetTextAlign (  12 );
  textlabel->SetTextSize  (0.04 );
  textlabel->SetTextColor (   1 );
  textlabel->SetTextFont  (  62 );
  textlabel->AddText(label);
  textlabel->Draw();
  
  
  canv.RedrawAxis();
  
  if(bestfit){
    bestfit->SetMarkerStyle(34);
    bestfit->SetMarkerSize(3.0);
    bestfit->SetMarkerColor(kBlack);
    bestfit->Draw("Psame");
  }
  
  if(SMexpected && SMexpectedLayer){
    //Hardcoded positions of the SM expectation. Fixed for the 2 diamonds. Must be done after RedrawAxis
	if((int)mass==90  ) {SMexpected->DrawMarker(0,0); SMexpectedLayer->DrawMarker(0,0);}
	if((int)mass==100 ) {SMexpected->DrawMarker(0.676,0.090);SMexpectedLayer->DrawMarker(0.676,0.090);}
	if((int)mass==120 ) {SMexpected->DrawMarker(1.815,0.077);SMexpectedLayer->DrawMarker(1.815,0.077);}
	if((int)mass==125 ) {SMexpected->DrawMarker(1.325,0.057);SMexpectedLayer->DrawMarker(1.325,0.057);}
	if((int)mass==130 ) {SMexpected->DrawMarker(0.979,0.034);SMexpectedLayer->DrawMarker(0.979,0.034);}
	if((int)mass==140 ) {SMexpected->DrawMarker(0.514,0.014);SMexpectedLayer->DrawMarker(0.514,0.014);}
	if((int)mass==160 ) {SMexpected->DrawMarker(0.146,0.009);SMexpectedLayer->DrawMarker(0.146,0.009);}
	if((int)mass==180 ) {SMexpected->DrawMarker(0.014,0.010);SMexpectedLayer->DrawMarker(0.014,0.010);}
	if((int)mass==200 ) {SMexpected->DrawMarker(0,0);SMexpectedLayer->DrawMarker(0,0);}
	if((int)mass==250 ) {SMexpected->DrawMarker(0,0);SMexpectedLayer->DrawMarker(0,0);}
	if((int)mass==300 ) {SMexpected->DrawMarker(0,0);SMexpectedLayer->DrawMarker(0,0);}
	if((int)mass==350 ) {SMexpected->DrawMarker(0,0);SMexpectedLayer->DrawMarker(0,0);}
	if((int)mass==400 ) {SMexpected->DrawMarker(0,0);SMexpectedLayer->DrawMarker(0,0);}
	if((int)mass==450 ) {SMexpected->DrawMarker(0,0);SMexpectedLayer->DrawMarker(0,0);}
	if((int)mass==500 ) {SMexpected->DrawMarker(0,0);SMexpectedLayer->DrawMarker(0,0);}
	if((int)mass==600 ) {SMexpected->DrawMarker(0,0);SMexpectedLayer->DrawMarker(0,0);}
	if((int)mass==700 ) {SMexpected->DrawMarker(0,0);SMexpectedLayer->DrawMarker(0,0);}
	if((int)mass==800 ) {SMexpected->DrawMarker(0,0);SMexpectedLayer->DrawMarker(0,0);}
	if((int)mass==900 ) {SMexpected->DrawMarker(0,0);SMexpectedLayer->DrawMarker(0,0);}
	if((int)mass==1000) {SMexpected->DrawMarker(0,0);SMexpectedLayer->DrawMarker(0,0);}         
  }
  return;
}

