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


    
    TFile rootf(file.Data());
    TTree* t1 = (TTree*)rootf.Get("fintree");
    rootf.cd();
    
    
    Long64_t nentries = t1->GetEntries();
    
    Int_t p =0;
    Int_t q =0;
    
    Double_t threshold[4]={0.};
    
    Int_t prova[4]={0};
    Int_t control[4]={0}; //Variabili di supporto
    
    Double_t signalStart[4]={0.}; //Inizio del segnale
    
    Int_t signalDuration[4]={20,700,30,30};	//Durata del segnale fissata
   
    Double_t chargeValue[4]={0.};	//Integrale del segnale
    
    Int_t isGoodEvent;				//Flag per eventi buoni

    
    
    cout << "number of entries : " << nentries << endl;	//nentries sono gli eventi
    //nsamples sono i bin di ogni evento
    

    Double_t time[1000]={0};
    Double_t meanTime;
    Double_t shiftedStart[4]={0};

    Double_t maxStart[4]={0};
    Double_t minStart[4]={1000,1000,1000,1000};
    
    Double_t minCharge[4]={50000.,200000.,500000.,500000.};
    Double_t maxCharge[4]={0.};
    
    
    t1->SetBranchAddress("signalStart", &signalStart);
    t1->SetBranchAddress("shiftedStart", &shiftedStart);
    t1->SetBranchAddress("chargeValue",&chargeValue);
    t1->SetBranchAddress("maxCharge",&maxCharge);
    for(int i=0;i<4;i++){
      cout<< maxCharge[i]<<endl;
}
    
    
    
    
    
    //FINE DICHIARAZIONE VARIABILI-------------------------------------------------------------------
    
    for(Long64_t entry = 0; entry < nentries; entry++) {       //Ciclo sugli eventi

        t1->GetEntry(entry);
        //nsample-=1;

            
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
    c1->cd(1);
    ChargeHist0->Draw();
    c1->cd(2);
    ChargeHist2->Draw();
    c1->cd(3);
    ChargeHist4->Draw();
    c1->cd(4);
    ChargeHist6->Draw();


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
