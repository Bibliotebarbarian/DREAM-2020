void TrackStudy(TString listaSim){

  // name of output root file                                                                                                                                                                 TFile* out = new TFile("Output.root","RECREATE");


  //open input files (listaSim is a list of sim files)
  char Buffer[500];
  char MyRootFile[2000];
  TChain *sim = new TChain("sim");
  ifstream *inputFile = new ifstream(listaSim.Data());
  while( !(inputFile->eof()) ){
    inputFile->getline(Buffer,500);
    if (!strstr(Buffer,"#") && !(strspn(Buffer," ") == strlen(Buffer)))
      {
        sscanf(Buffer,"%s",MyRootFile);
        sim->Add(MyRootFile);
        std::cout << "chaining " << MyRootFile << std::endl;
      }
  }
  inputFile->close();
  delete inputFile;