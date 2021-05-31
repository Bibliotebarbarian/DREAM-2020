//charge.C
//#define NSAMPLE nsample-1
#define INITIALSAMPLES 90
#define STARTCUTOFF .15
#define ENDCUTOFF .15
#define SCALING 0.2393
#define TIME_ERROR 0.0004
#define SATURATION_VALUE -990.


#include <string>
#include <cmath>
void histo(TString file){
	
	

	
	
	TCanvas *c1 = new TCanvas("c1","c1",1000,10,600,600);
	TCanvas *c2 = new TCanvas("c2","c2", 1000,700,600,600);
	TCanvas *c3 = new TCanvas("c3","c3", 1000,700,600,600);
	c1->Divide(2,2);
	c2->Divide(2,2);
	c3->Divide(2,2);
	
	
	TH1F **ChargeHist = new TH1F*[4];
	TH1F **StartHist = new TH1F*[4];	
	TH1F **ShTime  =  new TH1F*[4];

	for(int i=0; i<4; i++){
		ChargeHist[i] 	= new TH1F(TString::Format("chargehist%d",i), TString::Format("Charge Hist %d", i*2), 50, 0.,0.);
		StartHist[i]	= new TH1F(TString::Format("startHist%d",i), TString::Format("Starting time ch%d", i*2), 50, 0.,0.);
		ShTime[i]		= new TH1F(TString::Format("ShiftedTime%d",i), TString::Format("Shifted starting time ch%d", i*2), 50, 0., 0.);
	}
	
	
	TFile rootf(file.Data());
	TTree* t1 = (TTree*)rootf.Get("fintree");
	rootf.cd();
	
	
	Long64_t nentries = t1->GetEntries();
	
	Double_t signalStart[4]; //Inizio del segnale
	
	Double_t chargeValue[4];	//Integrale del segnale
	
	Double_t shiftedStart[4];
	
	Double_t maxStart[4];
	Double_t minStart[4];
	
	Double_t minCharge[4];
	Double_t maxCharge[4];
	
	Double_t minShift[4];
	Double_t maxShift[4];
	
	Int_t isGoodEvent=0;
	
	t1->SetBranchAddress("signalStart", &signalStart);
	t1->SetBranchAddress("shiftedStart", &shiftedStart);
	t1->SetBranchAddress("chargeValue",&chargeValue);
	//t1->SetBranchAddress("minCharge",&minCharge);       //non servono perchè uso range che esclude eventi oltre 3 sigma
	//t1->SetBranchAddress("maxCharge",&maxCharge);
	//t1->SetBranchAddress("minStart",&minStart);
	//t1->SetBranchAddress("maxStart",&maxStart);
	//t1->SetBranchAddress("minShift",&minShift);
	//t1->SetBranchAddress("maxShift",&maxShift);
	t1->SetBranchAddress("isGoodEvent",&isGoodEvent);
	
	cout << "number of entries : " << nentries << endl;	//nentries sono gli eventi
	//nsamples sono i bin di ogni evento
	
	
	//FINE DICHIARAZIONE VARIABILI-------------------------------------------------------------------
	
	for(Long64_t entry = 0; entry < nentries; entry++) {       //Ciclo sugli eventi
	
		t1->GetEntry(entry);
		//nsample-=1;
		//cout<<chargeValue[0]<<endl;
		if(isGoodEvent==1){
		      for(int j=0; j<4; j++){
			      ChargeHist[j]->Fill(chargeValue[j]);
			      StartHist[j]->Fill(signalStart[j]);
			      ShTime[j]->Fill(shiftedStart[j]);
		      }
		}
	}
	
	for(int i=0; i<4; i++){
	
	    minCharge[i]=ChargeHist[i]->GetXaxis()->GetBinCenter(ChargeHist[i]->FindFirstBinAbove());
	    maxCharge[i]=ChargeHist[i]->GetXaxis()->GetBinCenter(ChargeHist[i]->FindLastBinAbove());
	    
	    minStart[i]=StartHist[i]->GetXaxis()->GetBinCenter(StartHist[i]->FindFirstBinAbove());
	    maxStart[i]=StartHist[i]->GetXaxis()->GetBinCenter(StartHist[i]->FindLastBinAbove());
	    
	    minShift[i]=ShTime[i]->GetXaxis()->GetBinCenter(ShTime[i]->FindFirstBinAbove());
	    maxShift[i]=ShTime[i]->GetXaxis()->GetBinCenter(ShTime[i]->FindLastBinAbove());
	
	}
	
	for(int j=0; j<4; j++){
		cout << minCharge[j] << " " << maxCharge[j] << " " << minStart[j] << " " << maxStart[j] << " " << minShift[j] << " " << maxShift[j] << endl;
		ChargeHist[j]->SetBins(100, minCharge[j], maxCharge[j]);
		StartHist[j]->SetBins(100, minStart[j], maxStart[j]);
		ShTime[j]->SetBins(100, minShift[j], maxShift[j]);
		
		ChargeHist[j]->Rebin();
		StartHist[j]->Rebin();
		ShTime[j]->Rebin();
	
		c1->cd(j+1);
		ChargeHist[j]->Draw();
		
		c2->cd(j+1);
		StartHist[j]->Draw();
		
		c3->cd(j+1);
		ShTime[j]->Draw();
	
	}
	
	/*ChargeHist0->Write("chargeHistCherenkov");
	ChargeHist2->Write("chargeHistScint");
	ChargeHist4->Write("chargeHistPMT1");
	ChargeHist6->Write("chargeHistPMT2");
	StartHist0->Write("startHistCherenkov");
	StartHist2->Write("startHistScint");
	StartHist4->Write("startHistPMT1");
	StartHist6->Write("startHistPMT2");*/
	
	
	rootf.Close();

}
