#include <string>
#include <cmath>

void chargeTempGraph(TString file){

	TCanvas *c0 = new TCanvas("c0","c0",300,10,1200,600);
	TFile fin(file.Data(), "UPDATE");
	TTree* t1 = (TTree*)fin.Get("datatree");
	
	
	
	Long64_t nentries = t1->GetEntries();
	Double_t dummy;
	Double_t dummy2[4];
	Int_t isGood;
	Int_t count=0;
	Double_t temp[nentries];
	Double_t chargeValue[4][nentries];
	
	t1-> SetBranchAddress("temp", &dummy);
	t1-> SetBranchAddress("chargeValue", &dummy2);
	t1-> SetBranchAddress("isGoodEvent", &isGood);
	
	
	for(int i=0; i < nentries; i++){
		t1->GetEntry(i);
		if(isGood == 1){
	  		temp[count] = dummy;
	  		for(int j=0; j<4; j++){
				chargeValue[j][count] = -dummy2[j];
	  		}
	  	count++;
	  	//cout << "debug" << " " << dummy << " "<< temp[count-1] << " " <<dummy2[1] << " "<< chargeValue[1][count-1] <<"\n";
		}
	}
	
	

	c0-> SetTitle("Temperatura");
	c0->Clear();
	c0->Divide(2);
	
	
	TGraph* graph0 = new TGraph(count-1, temp, chargeValue[0]);
	TGraph* graph1 = new TGraph(count-1, temp, chargeValue[1]);
	
	c0->cd(1);
	graph0-> SetTitle("Cherenkov");
	graph0-> SetMarkerStyle(7);
	//graph0-> SetMarkerSize(10);
	graph0-> GetXaxis()->SetTitle("Temperature (C°)");
	graph0-> GetYaxis()->SetTitle("Charge (mV*ns)");
	graph0-> Draw("AP");
	
	
	c0->cd(2);
	graph1-> SetTitle("Scintillator");
	graph1-> SetMarkerStyle(7);
	//graph1-> SetMarkerSize(10);
	graph1-> GetXaxis()->SetTitle("Temperature (C°)");
	graph1-> GetYaxis()->SetTitle("Charge (mV*ns)");
	graph1-> Draw("AP");
	
	
	
	c0->Update();
	c0->WaitPrimitive("aaa");
}
