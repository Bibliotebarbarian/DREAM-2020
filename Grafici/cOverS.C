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
	TCanvas *c2 = new TCanvas("c2","c2",900,10,600,600);
	TCanvas *c3 = new TCanvas("c3","c3",900,10,600,600);
	c2->Divide(2,2);
	c3->Divide(2);
	Int_t nEntries = t1->GetEntries();

	
	Double_t signalStart[4]; //Inizio del segnale
	
	Double_t chargeValue[4];
	
	Double_t minCharge[4];
	Double_t maxCharge[4];
	
	Int_t isGoodEvent=0;
	
	Int_t nsample=1000;
	
	Double_t channels[4][1000];
	
	Double_t time[1000];
	Double_t tau;
	
	Double_t subCharge[1000];
	Double_t Qc=0.;
	Double_t Qs=0.;

	
	Int_t startEntry[2];
	
	t1->SetBranchAddress("signalStart", signalStart);
	t1->SetBranchAddress("minCharge",minCharge);
	t1->SetBranchAddress("maxCharge",maxCharge);
	t1->SetBranchAddress("chargeValue",chargeValue);
	t1->SetBranchAddress("doubleCh",channels);
	t1->SetBranchAddress("isGoodEvent",&isGoodEvent);
	
	TF1 *scint = new TF1("scint", "[0]*exp(x/[1])", 140., 580.);   
	TF1 *cher = new TF1("cher", "[0]*exp(x/[1])", 140., 580.);
	Double_t r=0.;
	
	t1->GetEntry(0);
	
	TH1F *correctedCharge = new TH1F("correctedCharge", "", 30, 0.,.06);
	TH2F *correctedChargeS = new TH2F("correctedChargeS", "",30, 0.,.06,30, 0.,500000.);
	TH2F *correctedChargeC = new TH2F("correctedChargeC", "",30, 0.,.06,30, 0., 3000.);
	TProfile* profys=new TProfile("profys","",30, 1000.,3000000.);
	TProfile* profyc=new TProfile("profyc","",30, 0.,3000.);
	TProfile* profxs=new TProfile("profxs","",30, 0.,.06);
	TProfile* profxc=new TProfile("profxc","",30, 0.,.06);
	for(int f=0; f<1000;f++){
		time[f]= (double)f /0.9866;
	}
	

	
	for(int i=0; i < nEntries; i++){
		t1->GetEntry(i);
		Qs=0;
		if(isGoodEvent == 1){
		
			Qc=chargeValue[0];
		
		
			
			scint->SetParameters(-140.,-300.);
			
		
			startEntry[0] = floor(signalStart[0]*0.9866);
			startEntry[1] = floor(signalStart[1]*0.9866);
			for(int k=0; k<660;k++){
				Qs += channels[1][startEntry[1]+k]*(time[k+1]- time[k]);
			}
			
			TGraph* SP0 = new TGraph(nsample-1, time, channels[0]);    
			TGraph* SP1 = new TGraph(nsample-1, time, channels[1]);
			
			SP1->Fit("scint", "NQ", "", signalStart[1]+140., signalStart[1]+580.);
			
			tau = scint->GetParameter(1);
			cher->SetParameters(-140.,0.);
			cher->FixParameter(1, tau);
			
			SP0->Fit("cher", "NQ", "", signalStart[0]+140., signalStart[0]+580.);
			
			Double_t Ac = cher->GetParameter(0);
			Double_t As = scint->GetParameter(0);
			
			for(int f=0; f < 20; f++){
				channels[1][startEntry[1]+f] *= (Ac/As);
				Qc += channels[1][startEntry[1]+f]*(time[f+1]-time[f]);
				
			}
			Qs = abs(Qs);
			r=Qc/Qs;
			correctedCharge->Fill(r);
			correctedChargeS->Fill(r,Qs);
			correctedChargeC->Fill(r,Qc);
		}
	}
 TF1 *lan0 = new TF1("lan0", "landau", 0.004,.04);
 	 correctedChargeS->ProfileX("profxs");
	 correctedChargeS->ProfileY("profys");
	 correctedChargeC->ProfileY("profyc");
	 correctedChargeC->ProfileX("profxc");
	 
	 
	 c2->cd(1);
	 profxs-> GetXaxis()->SetTitle("C/S");
	 profxs-> GetYaxis()->SetTitle("Qs");
	 profxs->Draw("hist");
	 c2->cd(2);
	 profxc-> GetXaxis()->SetTitle("C/S");
	 profxc-> GetYaxis()->SetTitle("Qc");
	 profxc->Draw("hist");
	 c2->cd(3);
	 profys-> GetYaxis()->SetTitle("C/S");
	 profys-> GetXaxis()->SetTitle("Qs");
	 profys->Draw("hist");
	 c2->cd(4);
	 profyc-> GetYaxis()->SetTitle("C/S");
	 profyc-> GetXaxis()->SetTitle("Qc");
	 profyc->Draw("hist");
	
	c3->cd(1);
	correctedChargeS->SetMarkerSize(10);
	correctedChargeS->SetMarkerStyle(7);
	correctedChargeS-> GetXaxis()->SetTitle("C/S");
	correctedChargeS-> GetYaxis()->SetTitle("Qs");
	correctedChargeS->Draw();
	c3->cd(2);
	correctedChargeC->SetMarkerSize(10);
	correctedChargeC->SetMarkerStyle(7);
	correctedChargeC-> GetXaxis()->SetTitle("C/S");
	correctedChargeC-> GetYaxis()->SetTitle("Qc");
	correctedChargeC->Draw();
	c1->cd();
	//correctedCharge->Fit("lan0","R");
	//gStyle->SetOptFit(1111);
	correctedCharge->Draw();
	c1->WaitPrimitive("aaa");
	
	
}
