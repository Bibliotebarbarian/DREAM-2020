void meanWF(TString fileName){

	TFile fin(fileName.Data());
	TTree* t1 = (TTree*)fin.Get("fintree");
	
	TCanvas *c1 = new TCanvas("c1","c1",900,10,600,600);
	c1->Divide(2);
	
	Long64_t nEntries = t1->GetEntries();
	Double_t smoothP[2][1000]={0.};   
	
	Double_t signalStart[4]; //Inizio del segnale     
	Double_t doubleCh[4][1000]={0};
	Double_t time[1000]={0};
	
	Int_t signalStartSample[4] = {0};
	Int_t nsample;
	
	Int_t isGoodEvent=0;	
	Int_t counter=0;   
	
	t1->SetBranchAddress("nsample",&nsample);             
	
	           
	t1->SetBranchAddress("signalStart", &signalStart);		
	t1->SetBranchAddress("doubleCh", &doubleCh);	
	t1->SetBranchAddress("isGoodEvent", &isGoodEvent);
	
	for(int i=0; i< nEntries; i++){
	
		t1->GetEntry(i);
		
		if(isGoodEvent==1){
			counter++;
		
			signalStartSample[0]=floor(signalStart[0]*0.9866);
			signalStartSample[1]=floor(signalStart[0]*0.9866);

			for(int f=0; f<710;f++){     //for sui sample da signalstart a signalend shiftati di 50 posizioni per allineare tutto per cherenkov
				smoothP[0][f+50] += doubleCh[0][signalStartSample[0]+f];

			}

			for(int f=0; f<710;f++){     //for sui sample da signalstart a signalend shiftati di 50 posizioni per allineare tutto per scintillatore 
				smoothP[1][f+50] += doubleCh[1][signalStartSample[1]+f];
				
			}
		}
		
	}
	
	 
	            
	            
	            

	for(int g=0;g<2;g++){    //for su i canali cherenkov e scintillatore
		for(int f=0; f<1000;f++){     //for sui sample da signalstart a signalend shiftati di 50 posizioni per allineare tutto  
			smoothP[g][f]/=counter;      //dividiamo la somma totale per il numero finale di eventi buoni
			time[f]= (double)f /0.9866;
		}
	}

	TGraph* SP0 = new TGraph(nsample-1,time,smoothP[0]);    
	TGraph* SP1 = new TGraph(nsample-1,time,smoothP[1]); 



	TF1 *scint = new TF1("scint", "[0]*exp(x/[1])", 140., 580.);   
	scint->SetParameters(-140.,-300.);



	c1->cd(2);
	SP1-> SetTitle("Scintillator");
	SP1->Fit("scint","R");
	SP1-> GetXaxis()->SetTitle("Time (ns)");                  //gli assi adesso hanno un nome
	SP1-> GetYaxis()->SetTitle("Voltage (mV)");
	gStyle->SetOptFit(1111);
	SP1->Draw();

	Double_t tau = scint->GetParameter(1);

	TF1 *cher = new TF1("cher", "[0]*exp(x/[2])+[1]", 140., 580.);   
	cher->SetParameters(-140.,0.);
	cher->FixParameter(2, tau);

	c1->cd(1);
	SP0-> SetTitle("Cherenkov");
	SP0->Fit("cher","R");
	SP0-> GetXaxis()->SetTitle("Time (ns)");                  //gli assi adesso hanno un nome
	SP0-> GetYaxis()->SetTitle("Voltage (mV)");
	gStyle->SetOptFit(1111);
	SP0->Draw();
	
}
