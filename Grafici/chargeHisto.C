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
	Double_t min[2];
	Double_t max[2];
	
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
	for(int k=0; k < 2; k++){
		min[k]= TMath::MinElement(count-1, chargeValue[k]);
		max[k]= TMath::MaxElement(count-1, chargeValue[k]);
		//cout << min[k] << " " << max[k] << "\n";
	}
	
	
	TH1F* h1= new TH1F("h1","h1", 40, min[0], max[0]);
	TH1F* h2= new TH1F("h2","h2", 40, min[1], max[1]);
	
	h1-> FillN(count-1, chargeValue[0], NULL, 1);
	h2-> FillN(count-1, chargeValue[1], NULL, 1);
	

	c0-> SetTitle("Temperatura");
	c0->Clear();
	c0->Divide(2);
	
	
		
	c0->cd(1);
	h1->Draw();
	c0->cd(2);
	h2->Draw();
	
	
	
	c0->Update();
	c0->WaitPrimitive("aaa");
}
