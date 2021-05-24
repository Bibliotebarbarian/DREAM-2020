//analisi.C
//#define NSAMPLE nsample-1
#define INITIALSAMPLES 90
#define STARTCUTOFF .15
#define ENDCUTOFF .15
#define SCALING 0.2393
#define TIME_ERROR 0.0004
#define SATURATION_VALUE -990.


#include <string>
#include <cmath>
void analisi(TString file){
    
    //double signalAmplitude=1000;
    //double signalPeriod=900;
    
    TCanvas *c0 = new TCanvas("c0","c0",300,10,600,600);
    TCanvas *c1 = new TCanvas("c1","c1",1000,10,600,600);
    TCanvas *c2 = new TCanvas("c2","c2", 1000,700,600,600);
TCanvas *c3 = new TCanvas("c3","c3", 1000,700,600,600);
    c1->Divide(2,2);
    c2->Divide(2,2);
    c3->Divide(2,2);
    /*
    TH1F *ChargeHist0 = new TH1F("chargehist0","Charge Hist 0", 50, 0.,14000.);
    TH1F *StartHist0 = new TH1F("startHist0","Starting time ch0",50, 0.,400.);
    

    TH1F *ChargeHist2 = new TH1F("chargehist2","Charge Hist 2", 50, 0.,450E3);
    TH1F *StartHist2 = new TH1F("startHist2","Starting time ch2",50, 0.,400.);
    
  
    
    TH1F *ChargeHist4 = new TH1F("chargehist4","Charge Hist 4", 50, 0.,30000.);
    TH1F *StartHist4 = new TH1F("startHist4","Starting time ch4",50, 0.,400.);
    

    
    TH1F *ChargeHist6 = new TH1F("chargehist6","Charge Hist 6", 50, 0.,30000.);
    TH1F *StartHist6 = new TH1F("startHist6","Starting time ch6",50, 0.,400.);
    */
    //ATTENZIONE IN QUESTE DEFINIZIONI NON TUTTI GLI EVENTI SONO NEGLI ISTOGRAMMI CAMBIA IL RANGE PER MIGLIOR DIVISIONI DEI BIN
    TH1F *ChargeHist0 = new TH1F("chargehist0","Charge Hist 0", 50, 0.,4000.);
    TH1F *StartHist0 = new TH1F("startHist0","Starting time ch0",50, 0.,400.);
    

    TH1F *ChargeHist2 = new TH1F("chargehist2","Charge Hist 2", 50, 0.,200E3);
    TH1F *StartHist2 = new TH1F("startHist2","Starting time ch2",50, 0.,400.);
    
  
    
    TH1F *ChargeHist4 = new TH1F("chargehist4","Charge Hist 4", 25, 0.,20000.);
    TH1F *StartHist4 = new TH1F("startHist4","Starting time ch4",50, 0.,400.);
    

    
    TH1F *ChargeHist6 = new TH1F("chargehist6","Charge Hist 6", 25, 0.,23000.);
    TH1F *StartHist6 = new TH1F("startHist6","Starting time ch6",50, 0.,400.);
    
    TH1F *ShTime0  =  new TH1F("ShifitedTime0","Shifted starting time ch0",40,-30.,30.);
    TH1F *ShTime2  =  new TH1F("ShifitedTime2","Shifted starting time ch2",15,-5.,10.);
    TH1F *ShTime4  =  new TH1F("ShifitedTime4","Shifted starting time ch4",17,-4.,5.);
    TH1F *ShTime6  =  new TH1F("ShifitedTime6","Shifted starting time ch6",17,-4.,5.);


    
    TFile fin(file.Data());
     
    TTree* t2 = (TTree*)fin.Get("datatree");
    
    TFile output("Analysis.root", "RECREATE");
    output.cd();
    TTree* t1= t2->CopyTree("nsample>1");
    
    
    Long64_t nentries = t1->GetEntries();
    
    Int_t count=0;
    Int_t p =0;
    Int_t q =0;
    Int_t duration =0;
    
    Double_t threshold[4]={0.};
    
    Int_t prova[4]={0};
    Int_t control[4]={0};
    Int_t kappaStart;
    Int_t kappaEnd;					//Variabili di supporto
    
    Double_t signalStart[4]={0.};
    Double_t signalEnd[4]={0.};		//Inizio e fine dei segnali
    
    Int_t signalDuration[4]={20,700,30,30};	//Durata del segnale fissata
   
    Double_t chargeValue[4]={0.};	//Integrale del segnale
    
    Int_t isGoodEvent;				//Flag per eventi buoni
    
    double media[4]={0.};
    double sigma[4]={0.};
    
    
    
    cout << "number of entries : " << nentries << endl;	//nentries sono gli eventi
    //nsamples sono i bin di ogni evento
    
    Int_t channels[4][1000]={0};
    
    
    
    Double_t doubleCh[4][1000]={0};			//dal DAQ i dati sono interi vanno convertiti a double per grafico
    
    
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
    
    
    t1->SetBranchAddress("nsample",&nsample);
    
    t1->SetBranchAddress("ch0", &channels[0]);
    t1->SetBranchAddress("ch2", &channels[1]);
    t1->SetBranchAddress("ch4", &channels[2]);
    t1->SetBranchAddress("ch6", &channels[3]);
    
    t1->Print();
    
    TBranch* signalMinBranch = t1 -> Branch("signalMin", minpoint, "minpoint[4]/D");
    TBranch* signalStartBranch = t1 -> Branch("signalStart", signalStart, "signalStart[4]/D");
    TBranch* signalEndBranch = t1 -> Branch("signalEnd", signalEnd, "signalEnd[4]/D");
    TBranch* shiftedStartBranch = t1 -> Branch("shiftedStart", shiftedStart, "shiftedStart[4]/D");
    TBranch* chargeValueBranch = t1 -> Branch("chargeValue", chargeValue, "chargeValue[4]/D");
    TBranch* isGoodEventFlag = t1 -> Branch("isGoodEvent", &isGoodEvent, "isGoodEvent/I");     //Creo Branch per i nuovi dati che mi sto calcolando
    
    
    
    
    
    //FINE DICHIARAZIONE VARIABILI-------------------------------------------------------------------
    
    for(Long64_t entry = 0; entry < nentries; entry++) {       //Ciclo sugli eventi
        
        
        for(int ii=0; ii<4; ii++){		//Inizializzazione
            control[ii]=0;
            prova[ii]=0;
            signalStart[ii] = 0.;
            signalEnd[ii] = 0.;
            shiftedStart[ii] = 0.;
            chargeValue[ii] = 0.;
            media[ii]=0.;
            sigma[ii]=0.;
        }
        
        isGoodEvent=0;
       
        
        
        t1->GetEntry(entry);
        //nsample-=1;
        
        
        
        for(int j=0; j<4; j++){
            
            for(int i=0; i<INITIALSAMPLES;i++){
                
                media[j]+=channels[j][i];
                sigma[j]+=channels[j][i]*channels[j][i];
                //TLeaf* c0 = t1->GetLeaf("ch0");
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
        
        
        
        for(int l=0;l<2;l++){
            
            min[l]= TMath::MinElement(1000, doubleCh[l]);	//calcola i minimi per cherenkov e scintillatore
            minpoint[l]=TMath::LocMin(1000, doubleCh[l]);	//trova il punto dell'array corrispondente al minimo
            
            if(min[l]< -5.*sigma[l] && doubleCh[l][minpoint[l]+1]<0. && doubleCh[l][minpoint[l]+2]<0. && doubleCh[l][minpoint[l]+3]<0.){
                //controlla se il minimo è oltre 5 sigma e che i 3 punti successivi siano minori di zero
                //controlla se il minimo si trova prima di metà dei dati (potrebbe essere modificato per controllare che sia attorno al tempo del trigger
                if(minpoint[l]<=(nsample/2)){
                    control[l]=1;
                }
            }
        }
        
        min[2]= TMath::MinElement(1000, doubleCh[2]);	//calcola i minimi per PMT1
        minpoint[2]=TMath::LocMin(1000, doubleCh[2]);	//trova il punto dell'array corrispondente al minimo
        min[3]= TMath::MinElement(1000, doubleCh[3]);	//calcola i minimi per PMT1
        minpoint[3]=TMath::LocMin(1000, doubleCh[3]);	//trova il punto dell'array corrispondente al minimo
        
        
        
        
        
        
        if(control[0]==1 && control[1]==1){
            //if(control[1]==1){
            count++;
            isGoodEvent=1;
            
            
            for(int u=0; u<4; u++){
                p=0;
                q=0;
		duration =0;
                   if(sigma[u]!=0.){
                       threshold[u]=-3.*sigma[u];
                   }else{
                   threshold[u]=-1.5;
                   }
                
                
                //----------calcolo signalStart-----------------------
             
            
                while(doubleCh[u][p]>threshold[u] || doubleCh[u][p+1]>threshold[u] || doubleCh[u][p+2]>threshold[u]){
                    p++;
                }
                signalStart[u]=time[p];
                
                
                //----------calcolo signalEnd-----------------------
		signalEnd[u]=signalStart[u]+signalDuration[u];
		duration=p+signalDuration[u];
		if(duration>nsample-1){
		  duration=nsample-1);
		}
		
		if(signalEnd[u]>time[nsample-1]){
		  signalEnd[u]=time[nsample-1];
		}
                
                
                if(signalStart[u]>maxStart[u]){
                    maxStart[u]=signalStart[u];
                }
                if(signalStart[u]<minStart[u]){
                    minStart[u]=signalStart[u];
                
            	}

		
                
       
                //----------calcolo CARICA-----------------------
                
                
		 //cout << entry  <<"     " << meanTime<<endl;
                
                for(int z = p; z <= duration; z++){ //Stiamo tenendo i primi due punti che superano il cutoff, eventualmente si scartano
                    chargeValue[u] += abs(doubleCh[u][z]*(time[z]-time[z-1]));
                
                }
                
                if(chargeValue[u]>maxCharge[u]){
                    maxCharge[u]=chargeValue[u];
                }
                if(chargeValue[u]<minCharge[u]){
                    minCharge[u]=chargeValue[u];
                }
               
            } 
            meanTime=0.5*(signalStart[2]+signalStart[3]);
              
            for(int u=0;u<4;u++){

shiftedStart[u]= signalStart[u]-meanTime;
}
        
            
            
            signalMinBranch->Fill();
            signalStartBranch->Fill();
            signalEndBranch->Fill();
            shiftedStartBranch->Fill();
            chargeValueBranch->Fill();
            isGoodEventFlag->Fill();
            
            
            ChargeHist0->Fill(chargeValue[0]);
            ChargeHist2->Fill(chargeValue[1]);
            ChargeHist4->Fill(chargeValue[2]);
            ChargeHist6->Fill(chargeValue[3]);
            
            StartHist0->Fill(signalStart[0]);
            StartHist2->Fill(signalStart[1]);
            StartHist4->Fill(signalStart[2]);
            StartHist6->Fill(signalStart[3]);
        
            
	    ShTime0->Fill(shiftedStart[0]);
	    ShTime2->Fill(shiftedStart[1]);
	    ShTime4->Fill(shiftedStart[2]);
	    ShTime6->Fill(shiftedStart[3]);

            
            
            //GRAFICI----------------------------------------------
            
            
            
            TGraphErrors* WF0 = new TGraphErrors(nsample-1,time,doubleCh[0],errx,erry[0]);
            //TGraph* WF1 = new TGraph(nsample,time,ch1);
            TGraphErrors* WF2 = new TGraphErrors(nsample-1,time,doubleCh[1],errx,erry[1]);
            //TGraph* WF3 = new TGraph(NSAMPLE,time,ch3);                                        // grafici con error bars
            TGraphErrors* WF4 = new TGraphErrors(nsample-1,time,doubleCh[2],errx,erry[2]);       //le error bars date dalla calibrazione sono veramente piccole forse con il jitter cambierà qualcosa però per ora a stento si vedono
            //TGraph* WF5 = new TGraph(nsample,time,ch5);
            TGraphErrors* WF6 = new TGraphErrors(nsample-1,time,doubleCh[3],errx,erry[3]);
            //TGraph* WF7 = new TGraph(nsample,time,ch7);
            
            
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
            
            
            c1->cd(1);
            ChargeHist0->Draw();
            
            c1->cd(2);
            ChargeHist2->Draw();
            
            c1->cd(3);
            ChargeHist4->Draw();
            
            c1->cd(4);
            ChargeHist6->Draw();
            
            gPad->Update();
            
            c2->cd(1);
            StartHist0->Draw();
            
            c2->cd(2);
            StartHist2->Draw();
            
            c2->cd(3);
            StartHist4->Draw();
            
            c2->cd(4);
            StartHist6->Draw();

	    c3->cd(1);
            ShTime0->Draw();
            
            c3->cd(2);
            ShTime2->Draw();
            
            c3->cd(3);
            ShTime4->Draw();
            
            c3->cd(4);
            ShTime6->Draw();

            gPad->Update();
           //c0->WaitPrimitive();
            //c0->Clear();
            
            //FINE GRAFICI--------------------------------------------
            
        }else{								//Il tree deve essere sempre riempito per non dare errori
            //Riempiamo di zeri
            for(int u=0; u<4; u++){
                minpoint[u] = 0;
                signalStart[u] = 0;
                signalEnd[u] = 0;
                chargeValue[u] = 0.;
            }
            isGoodEvent=0;
            
            signalMinBranch->Fill();
            signalStartBranch->Fill();
            signalEndBranch->Fill();
            chargeValueBranch->Fill();
            isGoodEventFlag->Fill();
        }
        
        
    }
    

    
    
         /*   ChargeHist0->SetBins(20,minCharge[0],maxCharge[0]);
            ChargeHist2->SetBins(20,minCharge[1],maxCharge[1]);
            ChargeHist4->SetBins(20,minCharge[2],maxCharge[2]);
            ChargeHist6->SetBins(20,minCharge[3],maxCharge[3]);
            
            StartHist0->SetBins(20,minStart[0],maxStart[0]);
            StartHist2->SetBins(20,minStart[1],maxStart[1]);
            StartHist4->SetBins(20,minStart[2],maxStart[2]);
            StartHist6->SetBins(20,minStart[3],maxStart[3]);
*/

    ChargeHist0->Write("chargeHistCherenkov");
    ChargeHist2->Write("chargeHistScint");
    ChargeHist4->Write("chargeHistPMT1");
    ChargeHist6->Write("chargeHistPMT2");
    
    /*StartHist0->Write("startHistCherenkov");
    StartHist2->Write("startHistScint");
    StartHist4->Write("startHistPMT1");
    StartHist6->Write("startHistPMT2");*/

    
    t1->Write("analysisTree");
    cout<<"Il numero di eventi selezionati è:  "<<count<<endl;   // stampa il numero di grafici "buoni"
    fin.Close();
    output.Close();
    
}
