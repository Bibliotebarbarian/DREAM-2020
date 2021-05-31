#include <iostream>

using std::cout;

void scatterPlot(TString fileName){

	TFile fin(fileName.Data());
	TTree* t1 = (TTree*)fin.Get("fintree");
	
	Long64_t nEntries = t1->GetEntries();
	
	Double_t chargeValue[4];
	Int_t isGoodEvent;
	
	t1->SetBranchAddress("chargeValue",&chargeValue);
	t1->SetBranchAddress("isGoodEvent", &isGoodEvent);
	
	TCanvas *c0 = new TCanvas("c0","c0",300,10,1200,1200);
	c0->Divide(2);
	
	t1->GetEntry(0);
	
	TGraph* scatter1 = new TGraph(nEntries);
	TGraph* scatter2 = new TGraph(nEntries);
	
	for(int i=1; i<nEntries; i++){
		t1->GetEntry(i);
		if(isGoodEvent){
			scatter1->SetPoint(i, chargeValue[2]+chargeValue[3], chargeValue[0]);
			scatter2->SetPoint(i, chargeValue[2]+chargeValue[3], chargeValue[1]);
		}
	}
	c0->cd(1);
	scatter1->SetMarkerStyle(2);
	scatter1->Draw("ap");
	c0->cd(2);
	scatter2->SetMarkerStyle(2);
	scatter2->Draw("ap");
	fin.Close();
}
