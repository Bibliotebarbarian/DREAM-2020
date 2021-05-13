/*  ROOT macro to save .dat into .root
    Guido Fantini 03-06-2015 gufantini@tiscali.it
    
    if ./tmp/ready exists loads ./tmp/tmp.dat into ./tmp/data.root
    while working exists ./tmp/busy
    when finished removes busy and tmp.dat
    
    if ./tmp/stop exists loads last event into rootfile
    then cleans everything, writes rootfile and dies
 */


void DatToRoot_v02(void){// Root macro 
  ifstream inFile, inFiletemp;
  ofstream outFile;
  
  // creating rootfile and appropriate branches
  TFile* rootfile = new TFile("./tmp/data.root","RECREATE");
  TTree* tree = new TTree("datatree","acquired waveforms");
  UInt_t ch0[4096],ch2[4096],ch4[4096],ch6[4096]; //già detto nel daq.cpp: al massimo con la mia scelta dei buffer un evento può essere lungo 4096 samples
  Double_t temp;
  Int_t nevent,nsample;
  tree->Branch("nevent",&nevent,"nevent/I");
  tree->Branch("nsample",&nsample,"nsample/I");
  tree->Branch("ch0",ch0,"ch0[nsample]/i");
  tree->Branch("ch2",ch2,"ch2[nsample]/i");
  tree->Branch("ch4",ch4,"ch4[nsample]/i");
  tree->Branch("ch6",ch6,"ch6[nsample]/i");
  tree->Branch("temp",&temp,"temp/D");
  
 
  nevent=0; // event number
  int stopflag=0,i;
  int time; // buttata
  int fileLine;
  
  
  while(stopflag!=2){
    // se lo stopflag è 1, scrivo in maniera corretta gli ultimi dati e POI chiudo tutto;
    if(stopflag==1)stopflag++;
    
    inFile.open("./tmp/ready"); //cerco di capire se esiste il file ready: se è così comincio a scrivere
    
	if(inFile.is_open()){// event ready
	
      inFile.close();
      // creo il file busy: finchè converto in formato root non posso elaborare altri dati
      outFile.open("./tmp/busy");
      outFile<<"busy"<<endl;
      outFile.close();
      // rimuovo il file ready
      system("rm ./tmp/ready");
      // load data into rootfile
      inFile.open("./tmp/tmp.dat");
      inFiletemp.open("./temp.dat");
      i=0;
      fileLine=0;
      do{
		inFile>>time>>ch0[i]>>ch2[i]>>ch4[i]>>ch6[i]; // legge dal file con questo formato
		//cout << "read " << time << ch0 << ch2 << ch4 << ch6 << endl; // DEBUG
		i++;
      } while(inFile.good()); //continua finchè il file non è finito

      
      while(fileLine != nevent+1)}
      	inFiletemp >> temp;
      	fileLine++;
      }
      
      nsample=i;
      tree->Fill(); //riempi il tree con i dati raccolti
      nevent++; 
      
      inFile.close();
      inFiletemp.close();
      // ora che ho finito posso liberarmi dei file busy e tmp.dat
      system("rm ./tmp/tmp.dat");
      system("rm ./tmp/busy");    
    }else{
      // check if DAQ told this routine to stop
      inFile.open("./tmp/stop");
      if(inFile.is_open()){
	stopflag=1;
	inFile.close();
	system("rm ./tmp/stop");
      }
      gSystem->Sleep(100); //aspetto 100 millisecondi, prima di ricontrollare di nuovo se il DAQ ha ricevuto un evento
    }
  }
  
  tree->Write(); // salva il tree nel file
  rootfile->Close(); // chiude il file
}
