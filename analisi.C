//analisi.C
	
	#define INITIALSAMPLES 90
	#define SCALING 0.2393
	#define TIME_ERROR 0.0004
	#define SATURATION -990
	
	
	
	#include <string>
	#include <cmath>
	
	using std::cout;
	using std::endl;
        
	void analisi(TString file){
	
	
	
	TCanvas *c0 = new TCanvas("c0","c0",300,10,600,600);

	
	
	TFile fin(file.Data());
	TTree* t1 = (TTree*)fin.Get("datatree");

	TString nomeoutput= TString("Analysis_");
	nomeoutput.Append(file.Data());
	
	
	TFile output(nomeoutput, "RECREATE");
	output.cd();
	TTree* t2 = new TTree("fintree", "tree finale");
	
	
	Long64_t nentries = t1->GetEntries();
	
	
	
	Int_t count=0;
	Int_t p =0;
	Int_t signalStartSample[4] = {0};
	
	Double_t threshold[4]={0.};
	
	
	Int_t control[4]={0};
	Long64_t entry;
				//Variabili di supporto
	
	Double_t signalStart[4]={0.};            //Inizio dei segnali
	Int_t signalDuration[4]={20,700,30,30};	//Durata del segnale fissata
	
	Double_t chargeValue[4]={0.};	//Integrale del segnale
	
	Int_t isGoodEvent;				//Flag per eventi buoni
	
	double media[4]={0.};
	double sigma[4]={0.};
	
	
	
	cout << "number of entries : " << nentries << endl;	//nentries sono gli eventi
	//nsamples sono i bin di ogni evento
	
	Int_t channels[4][1000]={0};
	
	
	
	Double_t doubleCh[4][1000]={0};	                //dal DAQ i dati sono interi vanno convertiti a double per grafico

	

	
	Double_t errx[1000]={0};         //errori sul tempo (uguali per ogni canale)
	Double_t erry[4][1000]={0};           //errori sui canali (diversi per ogni canale)
	
	Double_t min[4]={0};  		//valore del minimo per cherenkov e scintillatore
	Long64_t minpoint[4]={0}; 	//punto di minimo in array per chernkov e scintillatore
	
	
	Double_t time[1000]={0};
	Double_t meanTime;
	Double_t shiftedStart[4]={0};
	Int_t nsample;
	
	Double_t maxStart[4]={0};
	Double_t minStart[4]={1000,1000,1000,1000};
	
	Double_t minCharge[4]={50000.,200000.,500000.,500000.};
	Double_t maxCharge[4]={0.};
	
	Double_t minShift[4]={1000.,1000.,1000.,1000.};
	Double_t maxShift[4]={-1000.,-1000.,-1000.,-1000.};
	
	
	t1->SetBranchAddress("nsample",&nsample);
	t1->SetBranchAddress("ch0", &channels[0]);
	t1->SetBranchAddress("ch2", &channels[1]);
	t1->SetBranchAddress("ch4", &channels[2]);
	t1->SetBranchAddress("ch6", &channels[3]);
	
	//t1->Print();
	
	
	TBranch* signalMinBranch = t2 -> Branch("signalMin", minpoint, "minpoint[4]/D");
	TBranch* signalStartBranch = t2 -> Branch("signalStart", signalStart, "signalStart[4]/D");
	TBranch* shiftedStartBranch = t2 -> Branch("shiftedStart", shiftedStart, "shiftedStart[4]/D");
	TBranch* chargeValueBranch = t2 -> Branch("chargeValue", chargeValue, "chargeValue[4]/D");   //Creo Branch per i nuovi dati che mi sto calcolando
	TBranch* meanTimeBranch = t2 -> Branch("meanTime", &meanTime, "meanTime/D");    
	TBranch* doubleChBranch = t2 -> Branch("doubleCh", doubleCh, "doubleCh[4][1000]/D");
	
	
	// in ingresso impulso proiettato in nuovo vettore (vuoto) stessa lunghezza di quello di partenza, di cui i primi 50 canali sono lasciati vuoti e dal 51 sovrascriviamo l'impulso vero partendo da starting time di ciascun evento fino a poco oltre starting time+ signalduration (aggiungiamo circa 10 sample in piu)
	// secondo me conviene farlo in histo e poi mediamo 
	
	
	
	//FINE DICHIARAZIONE VARIABILI-------------------------------------------------------------------
	
	for(entry = 0; entry < nentries; entry++) {       //Ciclo sugli eventi
	    
	    
	    for(int ii=0; ii<4; ii++){		//Inizializzazione
	        control[ii]=0;
	        signalStart[ii] = 0.;
	        shiftedStart[ii] = 0.;
	        chargeValue[ii] = 0.;
	        media[ii]=0.;
	        sigma[ii]=0.;
	        signalStartSample[ii]=0;
	    }
	    
	    
	    
	    p=0;
	
	    
	    
	    t1->GetEntry(entry);
	    //nsample-=1;
	    
	    
	    
	    for(int j=0; j<4; j++){
	        
	        for(int i=0; i<INITIALSAMPLES;i++){
	            media[j]+=channels[j][i];
	            sigma[j]+=channels[j][i]*channels[j][i];
	        }
	        
	        media[j]/=INITIALSAMPLES;		//calcolo della media
	        sigma[j]/=INITIALSAMPLES;
	        sigma[j]-=media[j]*media[j];	// calcolo della deviazione standard
	        sigma[j]=sqrt(abs(sigma[j]));
	        
	        
	        
	        for(int i=0; i<nsample-1; i++){
	            
	            
	            time[i]= (double)i /0.9866;
	            
	            doubleCh[j][i] = (channels[j][i]-media[j]);	//offset + passaggio a double
	            
	            doubleCh[j][i]/=SCALING;		//Riscalamento	dei canali
	            
	            errx[i] = TIME_ERROR*time[i];
	            erry[j][i] =0.0002*doubleCh[j][i];	//errori
	            
	            // cout << entry << " " << i << " " << ch4[i] << endl;
	            
	        }
	        media[j]/=SCALING;		//Riscalamento media e stddev
	        sigma[j]/=SCALING;
	    }
	    
	    
	    
	    for(int l=0;l<4;l++){
	        
	        min[l]= TMath::MinElement(1000, doubleCh[l]);	//calcola i minimi per tutti e 4 i canali
	        minpoint[l]=TMath::LocMin(1000, doubleCh[l]);	//trova il punto dell'array corrispondente al minimo
	        
	        if(min[l] > SATURATION && min[l]< -5.*sigma[l] && doubleCh[l][minpoint[l]+1]<0. && doubleCh[l][minpoint[l]+2]<0. && doubleCh[l][minpoint[l]+3]<0.){
	            //controlla se il minimo è oltre 5 sigma e che i 3 punti successivi siano minori di zero
	            //controlla se il minimo si trova prima di metà dei dati (potrebbe essere modificato per controllare che sia attorno al tempo del trigger
	            if(minpoint[l] <= (nsample/2)){
	                control[l] = 1;
	            }
	        }
	    }
	    
	    
	    
	    
	    
	    
	    if(control[0]==1 && control[1]==1){
	        count++;
	        
	        
	        
	        for(int u=0; u<4; u++){
	            p=0;
		    if(sigma[u]!=0.){
		    	threshold[u] = -3.*sigma[u];
			}else{
		    	threshold[u] = -1.5;
		    }
		    
		    
		    //----------calcolo signalStart-----------------------
		 
		    while(doubleCh[u][p]>threshold[u] || doubleCh[u][p+1]>threshold[u] || doubleCh[u][p+2]>threshold[u]){
		        p++;
		    }
		    signalStart[u] = time[p];

		    
	
		    
		    
		    if(signalStart[u] > maxStart[u]){
		        maxStart[u] = signalStart[u];
		    }
		    if(signalStart[u] < minStart[u]){
		    	minStart[u] = signalStart[u];
		    
			}
	
		    //----------calcolo CARICA-----------------------
		    
		    
	
		    
		    for(int z = p; z < signalDuration[u]+p; z++){ //Stiamo tenendo i primi due punti che superano il cutoff, eventualmente si scartano
		        chargeValue[u] += abs(doubleCh[u][z]*(time[z]-time[z-1]));
		    }
		    
		    if(chargeValue[u] > maxCharge[u]){
		        maxCharge[u] = chargeValue[u];
		    }
		    if(chargeValue[u] < minCharge[u]){
		        minCharge[u] = chargeValue[u];
		    }
		   
		}
		
		meanTime=0.5*(signalStart[2] + signalStart[3]);
		  
		for(int u=0; u<4; u++){
		    shiftedStart[u] = signalStart[u]-meanTime;
		    
		    if(shiftedStart[u] > maxShift[u]){
		    	maxShift[u] = shiftedStart[u];
				}
		    if(shiftedStart[u] < minShift[u]){
		    	minShift[u] = shiftedStart[u];
				}
				
		}
		
		    
			
		       t2->Fill();    //riempio il tree con solo le forme d'onda buone
			//for su i canali cherenkov e scintillatore 
			
			
			
			
			//GRAFICI DELLE FORME D'ONDA----------------------------------------------
		    //I GRAFICI DEGLI ISTOGRAMMI STANNO SU HISTO.C
			
		/*	
		    TGraphErrors* WF0 = new TGraphErrors(nsample-1,time,doubleCh[0],errx,erry[0]);
		
		    TGraphErrors* WF2 = new TGraphErrors(nsample-1,time,doubleCh[1],errx,erry[1]);
			                           // grafici con error bars
		    TGraphErrors* WF4 = new TGraphErrors(nsample-1,time,doubleCh[2],errx,erry[2]);       
		
		    TGraphErrors* WF6 = new TGraphErrors(nsample-1,time,doubleCh[3],errx,erry[3]);
		
			
			
		    std::string s = std::to_string(entry+1);
		    char const *pchar = s.c_str();                     //il nome di ciascun canvas è il numero dell'evento
		    c0->SetTitle(pchar);
			
			
		    c0->Clear();
		    c0->Divide(2,2);
		    c0->cd(1);
		    WF0-> SetTitle("Cherenkov");
		    //WF0->SetMarkerStyle(7);
		    //WF0-> SetMarkerSize(10);
		    WF0-> GetXaxis()->SetTitle("Time (ns)");
		    WF0-> GetYaxis()->SetTitle("Voltage (mV)");
		    WF0->Draw();
		    //fitFunction->GetParameters();
		    c0->cd(2);
		    WF2-> SetTitle("Scintillator");
		    //WF2->SetMarkerStyle(7);
		    //WF2-> SetMarkerSize(10);
		    WF2-> GetXaxis()->SetTitle("Time (ns)");                  //gli assi adesso hanno un nome
		    WF2-> GetYaxis()->SetTitle("Voltage (mV)");
		    WF2->Draw();
		     
		    c0->cd(3);
		    WF4-> SetTitle("PMT1");
		    //WF4->SetMarkerStyle(7);
		    //WF4->SetMarkerSize(10);
		    WF4-> GetXaxis()->SetTitle("Time (ns)");
		    WF4-> GetYaxis()->SetTitle("Voltage (mV)");
		    WF4->Draw();
		    c0->cd(4);
		    WF6-> SetTitle("PMT2");
		    //WF6->SetMarkerStyle(7);
		    //WF6->SetMarkerSize(10);
		    WF6-> GetXaxis()->SetTitle("Time (ns)");
		    WF6-> GetYaxis()->SetTitle("Voltage (mV)");
		    WF6->Draw();
		     
		    gPad->Update();
			
		     
		    //c0->WaitPrimitive();
		    //c0->Clear();
			*/
		    //FINE GRAFICI--------------------------------------------
			
		}
	
	
	}
	        
	
	       
	
	TBranch* maxChargeBranch = t2 -> Branch("maxCharge", maxCharge, "maxCharge[4]/D");      //salviamo i massimi e minimi
	TBranch* minChargeBranch = t2 -> Branch("minCharge", minCharge, "minCharge[4]/D");         //di carica e tempi di inizio per gli istogrammi
	TBranch* maxStartBranch = t2 -> Branch("maxStart", maxStart, "maxStart[4]/D");           // li definiamo qui perche cosi abbiamo un solo valore ripetuto per tutte le entries del file
	TBranch* minStartBranch = t2 -> Branch("minStart", minStart, "minStart[4]/D");
	TBranch* maxShiftBranch = t2 -> Branch("maxShift", maxShift, "maxShift[4]/D");
	TBranch* minShiftBranch = t2 -> Branch("minShift", minShift, "minShift[4]/D");
	
	maxChargeBranch->Fill();
	minChargeBranch->Fill();
	
	maxStartBranch->Fill();
	minStartBranch->Fill();      
	
	maxShiftBranch->Fill();
	minShiftBranch->Fill();
	
	                 //Selezione eventi tramite fit Gaussiano..............................
	
	TH1F FitShTime0= TH1F("Istogramma fit CH0", "Istogramma per fit Gaussiano", count*2/3, minShift[0], maxShift[0]);  
	TH1F FitShTime1= TH1F("Istogramma fit CH1", "Istogramma per fit Gaussiano", count*2/3, minShift[1], maxShift[1]);  
	TF1** fitGaussian= new TF1*[2];
	
	Double_t shifting[4]={0.};
	
	t2->SetBranchAddress("shiftedStart",&shifting);
	
	for (int jjj=0; jjj<count; jjj++){
	  
	      t2->GetEntry(jjj);
	      
	      FitShTime0.Fill(shifting[0]);
	      FitShTime1.Fill(shifting[1]);
	      
	      
	}
	
	
	for(int i=0; i<2; i++){
		
		fitGaussian[i] = new TF1(TString::Format("Gaussian_%i",i),"[0]*exp(-0.5*((x-[1])/[2])**2)",minShift[i], maxShift[i]);
	}
	
	fitGaussian[0]->SetParameters(FitShTime0.GetBinContent(FitShTime0.GetMaximumBin()),FitShTime0.GetMean(),FitShTime0.GetRMS());
	//cout<<0<<": \t"<<FitShTime0.GetBinContent(FitShTime0.GetMaximumBin()) <<" "<<FitShTime0.GetMean()<<" "<<FitShTime0.GetRMS()<<endl;
	
	fitGaussian[1]->SetParameters(FitShTime1.GetBinContent(FitShTime1.GetMaximumBin()),FitShTime1.GetMean(),FitShTime1.GetRMS());
	//cout<<1<<": \t"<<FitShTime1.GetBinContent(FitShTime1.GetMaximumBin()) <<" "<<FitShTime1.GetMean()<<" "<<FitShTime1.GetRMS()<<endl;
	
        FitShTime0.Fit("Gaussian_0","W N");
	FitShTime1.Fit("Gaussian_1","W N");
	
	Double_t meanGauss[4]= {0.};
	Double_t sigmaGauss[4]={0.};
	
	for(int j=0; j<2; j++){
		
		sigmaGauss[j]=fitGaussian[j]->GetParameter(2);
		meanGauss[j]=fitGaussian[j]->GetParameter(1);
		
	}
	
	Int_t goodCount=count;
	
	TBranch* isGoodEventBranch = t2 -> Branch("isGoodEvent", &isGoodEvent, "isGoodEvent/I");
	
	for (int jjj=0; jjj<count; jjj++){
	  
	      isGoodEvent=1;
	  
	      t2->GetEntry(jjj);
	      
	      if(abs(shifting[0]-meanGauss[0]) > 3*abs(sigmaGauss[0]) || abs(shifting[1]-meanGauss[1]) > 3*abs(sigmaGauss[1]) ){
		    isGoodEvent=0;
		    goodCount--;
		    
		    
	      }
	      
	      isGoodEventBranch->Fill();
	}
	

	
			  //FIT..............................

	
	

	
	
	/*c4->Clear();
	c4->cd(1);
	t1->Draw("temp:nevent");*/
	
	
	t2->Write("fintree");
	t1->Write("datatree");
	
	
	
	cout<<"Numero di eventi selezionati:  "<< count << endl;   // stampa il numero di grafici "buoni"
	cout<<"Numero eventi in 3 sigma: "<< goodCount <<endl;
	//cout<<"Numero di eventi selezionati dopo la seconda analisi:  "<< count << endl;
	fin.Close();
	output.Close();
	
}
