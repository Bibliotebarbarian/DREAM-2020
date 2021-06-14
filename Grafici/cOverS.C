#include <iostream>
#include <string>
#include <cmath>


using std::cout;
using std::endl;

void cOverS(TString file){


	TFile rootf(file.Data());
	TTree* t1 = (TTree*)rootf.Get("fintree");
	rootf.cd();
	
	TCanvas *c1 = new TCanvas("c1","c1",900,10,600,600);
	
	Int_t nEntries = t1->GetEntries();

	
	Double_t signalStart[4]; //Inizio del segnale
	
	Double_t chargeValue[4];
	
	Double_t minCharge[4];
	Double_t maxCharge[4];
	
	Int_t isGoodEvent=0;
	Int_t waveNumber;
	
	Int_t nsample=1000;
	
	Double_t channels[4][1000];
	
	Double_t time[1000];
	Double_t tau;
	
	Double_t subCharge[1000];
	Double_t Qc=0.;
	Double_t Qs=0.;
	Double_t temp;
	
	Int_t startEntry[2];
	
	t1->SetBranchAddress("signalStart", signalStart);
	t1->SetBranchAddress("minCharge",minCharge);
	t1->SetBranchAddress("maxCharge",maxCharge);
	t1->SetBranchAddress("chargeValue",chargeValue);
	t1->SetBranchAddress("doubleCh",channels);
	t1->SetBranchAddress("isGoodEvent",&isGoodEvent);
	
	TF1 *scint = new TF1("scint", "[0]*exp(x/[1])", 140., 580.);   
	TF1 *cher = new TF1("cher", "[0]*exp(x/[1])", 140., 580.);

	
	t1->GetEntry(0);
	
	TH1F *correctedCharge = new TH1F("correctedCharge", "", 100, 0., .06);
	
	
	for(int f=0; f<1000;f++){
		time[f]= (double)f /0.9866;
	}
	

	
	for(int i=0; i < nEntries; i++){
		t1->GetEntry(i);
		
		if(isGoodEvent == 1){
		
			Qc=chargeValue[0];
			Qs=chargeValue[1];
		
			scint->SetParameters(-140.,-300.);
			cher->SetParameters(-140.,0.);
		
			startEntry[0] = floor(signalStart[0]*0.9866);
			startEntry[1] = floor(signalStart[1]*0.9866);
			
			
			TGraph* SP0 = new TGraph(nsample-1, time, channels[0]);    
			TGraph* SP1 = new TGraph(nsample-1, time, channels[1]);
			
			SP1->Fit("scint", "NQ", "", signalStart[1]+140., signalStart[1]+580.);
			
			tau = scint->GetParameter(1);

			cher->FixParameter(1, tau);
			
			SP0->Fit("cher", "NQ", "", signalStart[0]+140., signalStart[0]+580.);
			
			Double_t Ac = cher->GetParameter(0);
			Double_t As = scint->GetParameter(0);
			
			for(int f=0; f < 70; f++){
				channels[1][startEntry[1]+f] *= (Ac/As);
				Qc -= channels[1][startEntry[1]+f]/0.9866;
			}
			temp=Qc/Qs;
			correctedCharge->Fill(temp);
		}
	}
	c1->cd();
	correctedCharge->Draw();
	c1->WaitPrimitive("aaa");
}
