#include <fstream>
#include <iostream>

using std::cout;

void tchain(Int_t fileN){

  // name of output root file      
  TFile* out= new TFile("Output.root","RECREATE");


  TChain *datatree = new TChain("datatree");


  for(int i=1; i <= fileN; i++){ 

	datatree->Add(TString::Format("%d.root", i));
        std::cout << "chaining " << TString::Format("%d.root", i) << std::endl;
  }
  datatree->Write();
}
