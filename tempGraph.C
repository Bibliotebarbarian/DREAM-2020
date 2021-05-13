#include <string>
#include <cmath>

void tempGraph(TString file){

	TCanvas *c0 = new TCanvas("c0","c0",300,10,1200,600);
	TFile fin(file.Data(), "UPDATE");
	TTree* t1 = (TTree*)fin.Get("datatree");
	
	

	Long64_t nentries = t1->GetEntries();
	Double_t temp[nentries];
	Double_t dummy;
	Int_t dummyInt;
	Int_t events[nentries];
	Double_t doubles[nentries];

	t1-> SetBranchAddress("nevent", &dummyInt);
	t1-> SetBranchAddress("temp", &dummy);

	

	for(int i=0; i < nentries; i++){
		t1->GetEntry(i);
		temp[i] = dummy;
		doubles[i] = 18.73*dummyInt;
		doubles[i] /=60*60;
		
	}

	TH2F* h1 = new TH2F("h1", "h1", 2000, -0.5, doubles[nentries-1]+.5, 20, 25.5, 33.5);
	c0-> SetTitle("Temperatura");

	h1->FillN(nentries, doubles, temp, pointer, 1);
	c0->cd(1);
	TProfile* Prof1 = h1->ProfileX("Prof1");

	Prof1->SetMarkerStyle(7);
	Prof1->SetMarkerSize(10);
	Prof1->SetMaximum(30);
	Prof1->SetMinimum(26);
	Prof1-> SetTitle("Temperature");
	Prof1-> GetYaxis()->SetTitle("Temperature (C°)");
	Prof1-> GetXaxis()->SetTitle("Time (hr)");
	Prof1->Draw("hist p");
	c0->Update();
	c0->WaitPrimitive("aaa");
}
