// DAQ software
// created by Tiziano Abdelsalhin, Marco Del Tutto, Gianluca Filaci
// July, 2014
// rewritten by Guido Fantini, Valerio Giulitti
// June, 2015
// References:
//    ref1: ManualeV1731.pdf
//    ref2: V1721-31_Registers_Description.pdf

#include <fstream>
#include <iostream>
#include <ctime>
#include <cmath>
#include <cstring> // manipolazione stringhe
#include <stdlib.h>
#include "./keyb.c" //Include file keyb.c in questa cartella, dovrebbe servire a abilitare i comandi da tastiera durante l'esecuzione

#include "CAENVMElib.h"
#include "CAENDigitizer.h"

//nell'atto della compilazione ricordiamoci di aggiungere all'inizio l'opzione "-DLINUX e poi linkare
//le librerie con l'opzione -l CAENVME -l CAENDigitizer"

//Quindi per compilare si deve scrivere "g++ -o daq48h daqvero.cpp -DLINUX -l CAENVME -l CAENDigitizer" 


#define TEMPOMISURA 60*60*22 //secondi
// calibration constants
#define M0 236.7
#define Q0 240.

#define M2 236.2
#define Q2 236.6
#define M4 236.7
#define Q4 237.7

#define M6 232.4
#define Q6 230.9

using namespace std; //mi evita di dover specificare std:: prima dei cout

// PrintInfo() Establishes a connection with the board, fills DHandle, prints some info
void PrintInfo();

// self-calibration routine from ch 0 to ch 8 (CAEN manual) then calls EnablingDESmode
void SelfcalibrationDESmode(uint32_t* DACregister,uint32_t* DCoffset);
int EnablingDESmode(int Handle, uint32_t * DCoffset, uint32_t * DACregister);
// loads DCoffset in the proper register of the even channels, abort on failure
void LoadOffsetDESmode(uint32_t* DACregister,uint32_t* DCoffset);

FILE * FilePower(int choice);
double DAConverter(int channel, int value);
uint32_t ADConverter(int channel, double volts);

// TRIGGER SETTINGS (local/external)

// settings for local trigger single channel below threshold
// Vth: threshold voltage in V - negative logic (trigger active below th.)
// Nloc: number of memory locations per event 1loc = 16 samples = 16 ns (DES mode). 
//       Do not exceed 255 (if memory is divided in 1024 blocks).
void SetLocalTrigger(uint32_t ChannelMask,double Vth,uint32_t NlocBeforeTrigger,uint32_t NlocAfterTrigger);
void SetExternalTrigger(uint32_t ChannelMask,uint32_t NlocBeforeTrigger,uint32_t NlocAfterTrigger);

void WaitForTrigger();


// returns Nsamples (num of 8bit samples saved in the array)
uint32_t GetEventDESmode(uint32_t* ch0, uint32_t* ch2);
uint32_t GetEventDESmode(uint32_t* ch0, uint32_t* ch2,uint32_t* ch4, uint32_t* ch6);

void SaveFile(const char* filename,uint32_t* ch0,uint32_t* ch2,uint32_t* ch4,uint32_t* ch6,int Nsamples);

bool DatToRoot_IsBusy();
void DatToRoot_EventReady();
void DatToRoot_Stop();

int  DHandle=0; // handle della scheda
uint32_t Data; // variabile temporanea di lettura registri

int main(){
  //time_t current_time = time(NULL);
  PrintInfo();
  
  int control=0; // error flag, 0 if everything fine
  int i;
  uint32_t DACregister[8]; // channel register addresses 0x1n98: permette di settare un DC offset
  for(i=0;i<8;i++)DACregister[i]=0x1098+i*0x100;

  // The offset can be varied between 0 and 65535 (16 bit di memoria del registro 0x1n98)
  uint32_t DCoffset[7] = {0};
  // Starting values for odd channels offsets (this values will be changed by calibration)
  DCoffset[1] = 35700;
  DCoffset[3] = 36400;
  DCoffset[5] = 36000;
  DCoffset[7] = 36000;
  // Offset even channels
  DCoffset[0] = 7850;
  DCoffset[2] = 7850;
  DCoffset[4] = 7850;
  DCoffset[6] = 7850;
  
  SelfcalibrationDESmode(DACregister,DCoffset);
  LoadOffsetDESmode(DACregister,DCoffset);


  // set local trigger single channel 
  // 0x5 = 101  ch 0 e ch 2 active
  double Vth = -0.050; // 50 mV
  uint32_t s0=ADConverter(0,Vth);
  uint32_t s2=ADConverter(2,Vth);
  /* DEBUG */ cout << "SOGLIA " << s0 << " " << s2 << endl;
  //uint32_t Twindow = 28; // 28 ns
  //SetLocalTrigger(0x55,Vth,3,10);                            //In caso si volesse usare un trigger locale, ox55 seleziona il trigger interno su tutti i canali pari
  // 0x5 abilito 0-2 0x55 abilito 0-2-4-6
  SetExternalTrigger(0x55,15,45); // GF Acquisition window to be changed

  time_t beginning;
  time(&beginning);
  // Acquisition start
  printf("\n\nACQUISITION START\n\n");
  // 0x8100: ref2 p.14
  control += CAEN_DGTZ_WriteRegister(DHandle,0x8100,0x4);
  system("root -l -q \"DatToRoot_v02.cpp\"&"); // call external daemon to merge output ./tmp/tmp.dat into one single rootfile
  usleep(1000000); // dormi 1 sec
  
  int eventTOT=0; 
  uint32_t Nsamples;
  uint32_t* ch0 = new uint32_t[4096]; // più di 4096 campionamenti in DES mode non posso averli suddividendo in 1024 la memoria
  uint32_t* ch2 = new uint32_t[4096];
  uint32_t* ch4 = new uint32_t[4096];
  uint32_t* ch6 = new uint32_t[4096];
  bool busy=true;
  while(1){
    cout << "eventTOT " << eventTOT <<endl;
    WaitForTrigger();   
    system("./tmp/temperatura");
    Nsamples = GetEventDESmode(ch0,ch2,ch4,ch6);
    eventTOT++;
    
    // save event to disk
    do{
      busy=DatToRoot_IsBusy();
    }while(busy==true);
    SaveFile("./tmp/tmp.dat",ch0,ch2,ch4,ch6,Nsamples);
    //l'evento è pronto ed è stato scritto in ./tmp/tmp.dat
    DatToRoot_EventReady();
        
    // execution time checks
     if(difftime(time(NULL),beginning)>=TEMPOMISURA){
      DatToRoot_Stop();
      CAEN_DGTZ_WriteRegister(DHandle,0x8100,0x8);
      cout << "ACQUISITION END: acquisition time " << TEMPOMISURA << "sec" << endl;
      exit(EXIT_SUCCESS);
      }
    cout << endl;
    
  } // end acquisition cycle
  
  return 0;
}
/*=====================================================================================================
                            E N D   O F   M A I N
  =======================================================================================================*/

void PrintInfo(){
  //Ti da il benvenuto e scrive il tempo attuale
  cout<<endl<<endl<<"     Welcome to the data acquisition software!"<<endl;
  cout<<endl;
  time_t current_time = time(NULL); //time restituisce il tempo attuale
  cout<<ctime(&current_time)<<endl; //ctime converte la variabile time_t in una stringa

  
  //Versione software librerie:
  //
  //La funzione SWRelease prende in input una stringa di 6 caratteri e ci scrive la
  //versione software delle librerie. cvSuccess è una costante definita
  //nelle librerie CAEN che vale 0.
  char model[6];
  if(CAENVME_SWRelease(model)!=cvSuccess){
    cout<<endl<<endl<<"Error function CAENVME_SWRelease"<<endl;
  }else{
    cout<<"Library version: "<<model<<endl<<endl;
  }
  
  //Inizializzazione digitizer:
  //
  //La funzione CAEN_DGTZ_OpenDigitizer ha come input, in ordine:
  //-tipo di connessione
  //-numero di connessione (numero intero da 0 a cose , infatti è 0 nel nostro caso);
  //-CONET code, che per i dispositivi connessi via USB è 0;
  //-VME Base Address, indirizzo base del VME, nel nostro caso è 0xFFFF0000;
  //-Puntatore all'handler che viene ritornato dalla funzione stessa.
  if(CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB,0,0,0xFFFF0000,&DHandle)!=0){
    cout<<endl<<endl<<"Error opening digitizer " << CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB,0,0,0xFFFF0000,&DHandle) << endl;
    exit(EXIT_FAILURE);
  }
  else{
    cout<<"Digitizer initialized"<<endl;
  }
  
  //GetInfo
  
  //Struttura (che verrà creata da CAEN_DGTZ_GetInfo) che contiene le info.
  CAEN_DGTZ_BoardInfo_t BoardInfo;
  //GetInfo crea, se ci riesce, una struttura che contiene varie info sul modulo
  if(CAEN_DGTZ_GetInfo(DHandle, &BoardInfo)!=0){
    cout<<endl<<endl<<"Error get info"<<endl;
    exit(1);
  }
  else{
    cout<<"Board model: "<<BoardInfo.ModelName<<endl;
    cout<<"Firmware version ROC: "<<BoardInfo.ROC_FirmwareRel<<endl;
    cout<<"Firmware version AMC: "<<BoardInfo.AMC_FirmwareRel<<endl;
  }
  
  //Software reset: si opera accedendo in modalità scrittura il registro dedicato
  //La funzione WriteRegister prende in input l'handler del Digitizer, l'indirizzo del registro da scrivere e il dato a 32-bit da scriverci
  if(CAEN_DGTZ_WriteRegister(DHandle, 0xEF24, 1)!=0){
    cout<<endl<<endl<<"Reset error"<<endl;
  }
  else{
    cout<<"Resetted"<<endl;
  }

  //Software clear: si opera come descritto sopra
  if(CAEN_DGTZ_WriteRegister(DHandle, 0xEF28, 1)!=0){
    cout<<endl<<endl<<"Software clear error"<<endl;
  }
  else{
    cout<<"Buffer free"<<endl;
  }
}

//****************************************************************************************
void SelfcalibrationDESmode(uint32_t* DACregister,uint32_t* DCoffset){
  int register1 = 0x109C; //ch. n ADC Configuration register: 0x1n9C
  int register2 = 0x1088; //ch. n Status register: 0x1n88
  int control=0;
  // Calibration (see: ref1 p.24)
  for(int k=0; k<8; k++){
    //prima devo porre a 1 il bit di calibrazione del registro di calibrazione dell'ADC
    //del canale k-esimo, poi devo rimetterlo a zero e parte l'autocalibrazione
    control += CAEN_DGTZ_WriteRegister(DHandle, register1, 2);
    control += CAEN_DGTZ_WriteRegister(DHandle, register1, 0);
    usleep(1000); //il programma aspetta 1000 microsecondi: il processo di autocalibrazione impiega
                  //infatti un tempo dell'ordine del millisecondo
    //Polling: verifica ciclica dello stato del canale k-esimo: se il settimo bit di 0x1n88 è posto
    //a 1 significa che la calibrazione è stata eseguita, se invece
    //è posto a 0 significa che la calibrazione è ancora in opera
    do{
      control += CAEN_DGTZ_ReadRegister(DHandle, register2, &Data);
    }while(!((Data & 0x40) >> 6));
    //la condizione nel while significa che sto prendendo l'and bitwise tra Data e 0x40 (che
    //in binario è 1000000), ossia sto estraendo il settimo bit. Dopodichè lo sposto
    //a destra di 6 posizioni (ossia dal settimo a primo posto): dunque ho ottenuto un numero che è
    //0 o 1 a seconda che il settimo bit di Data sia 0 o 1 rispettivamente.
    printf("Channel %i self calibrated.\n", k);
    register1 += 0x100;  
    register2 += 0x100;
  }
  if(control != 0){
    cout << "Something went wrong during calibration before activating DES mode." << endl;
    exit(EXIT_FAILURE);
  }
  int control2;
  // Enabling DES mode
  control2=EnablingDESmode(DHandle, DCoffset, DACregister);
  if (control2 == 5){
    cout << "Error during control cycle DES mode."<< endl;
    exit(EXIT_FAILURE);
  }
  if (control2 == 1){
    cout << "Error during a function called in DES mode inizialization."<< endl;
    exit(EXIT_FAILURE);
  }
}

void LoadOffsetDESmode(uint32_t* DACregister,uint32_t* DCoffset){
  // WARNING: do NOT load offset on channels you do not use (odd ch. disabled in DES mode)

  int i=0,control=0;
  for(i=0;i<8;i+=2){// Setting offset even channels (loop on even channels)
    control += CAEN_DGTZ_WriteRegister(DHandle, DACregister[i], DCoffset[i]);
  }

  cout<<endl<<endl;
  for(i=0;i<8;i+=2){  // Offset control (loop on even channels)
    do{
      cout<<"\rChecking ch. "<<i<<" DAC";
      control += CAEN_DGTZ_ReadRegister(DHandle, 0x1088 + i*0x0100, &Data); // Checking ch. i status (se bit[2]=0 DCoffset aggiornato)
    }while((Data & 0x4)>>2);
    cout<<"Channel "<<i<<" DC offset updated"<<endl;
  }

  if(control!=0){
    cout << "Something went wrong loading offsets in DES mode." << endl;
    exit(EXIT_FAILURE);
  }
}

// ********************
// This function takes the sample given by the digitizer and returns the corrispondent value in Volts
// INPUT: the channel you want to convert, the value to be converted
// RETURN: tha sample value in Volts


double DAConverter(int channel, int value){
  switch (channel) {
    case 0:
      return ((double)value-Q0)/M0;
    case 2:
      return ((double)value-Q2)/M2;
    case 4:
      return ((double)value-Q4)/M4;
    case 6:
      return ((double)value-Q6)/M6;
    default:
      cout << "DAConverter ERROR invalid channel" << endl;
      return -5000.;
  }
}

uint32_t ADConverter(int channel, double volts){
    switch (channel) {
    case 0:
      return round(M0*volts+Q0);
    case 2:
      return round(M2*volts+Q2);
    case 4:
      return round(M4*volts+Q4);
    case 6:
      return round(M6*volts+Q6);
    default:
      cout << "ADConverter ERROR invalid channel" << endl;
      return 999999;
  }
}


// ********************
// This function is responsible to calibrate and activate the DES mode (1GS/s)
// INPUT: the array containing the channels offsets
//        the array containing the DAC register addresses
// RETURN: '0' if ok or '1' if a function called gives an error

int EnablingDESmode(int DHandle, uint32_t * DCoffset, uint32_t * DACregister){
  
  bool channelCalib[4] = {false};
  int control = 0;
  uint32_t Data;
  int sample0;
  uint32_t EventSize;
  uint32_t j;
  
  // Setting starting offsets (odd channel only)
  for (int k = 1; k < 8; k += 2) {
    control += CAEN_DGTZ_WriteRegister(DHandle, DACregister[k], DCoffset[k]);
  }
  
  // Setting trigger via software only: questo registro abilita il trigger via software se pongo a 1 il bit [31]
  control += CAEN_DGTZ_WriteRegister(DHandle, 0x810C, 0x80000000);
  // Setting 500MS/s (il bit [12] è posto a zero) and sequential memory access (il bit [4]
  // è posto a 1)
  control += CAEN_DGTZ_WriteRegister(DHandle, 0x8000, 0x10);
  // Acquisition Stop (trigger settings)(Channel mask can't be chenged if acquisition is running):
  // il bit [3] é posto a 1, il che permette di considerare tutti i trigger (per maggiori info
  // andare a ref2 p.14) e i bit [0;1] sono posti a 0, il che fa stoppare l'acquisizione poichè il bit [2] è posto a 0
  control += CAEN_DGTZ_WriteRegister(DHandle,0x8100,0x8);
  // Buffer organization (buffer diviso in 1024 blocchi, codice=1010=0xA)
  control += CAEN_DGTZ_WriteRegister(DHandle,0x800C,0xA);
  // Custom size (0 = default: 2k samples per canale per evento)
  control += CAEN_DGTZ_WriteRegister(DHandle,0x8020,0); 
  // Setting the number of samples after the trigger (@500MS/s è uguale al contenuto del
  // registro (240) * 8 + Costant Latency (vedi ref2 p.17)
  control += CAEN_DGTZ_WriteRegister(DHandle,0x8114,240);
  // Disabling even channels 0xAA=10101010 VA MESSO ALLA FINE!!!
  control += CAEN_DGTZ_WriteRegister(DHandle,0x8120,0xAA);

  // Acquisition status (waiting till is ready)
  do{
    control += CAEN_DGTZ_ReadRegister(DHandle, 0x8104, &Data);
  }while(!((Data & 0x100) >> 8));
  // abbiamo estratto il bit [8]: se esso è 1 il sistema è pronto per l'acquisizione
  
  // *** Start acquisition
  // C=1100, dunque pongo a zero i due bit [1;0] e a 1 il bit [2]: questo fa partire l'acquisizione
  control += CAEN_DGTZ_WriteRegister(DHandle, 0x8100, 0xC);
  
  do {
    // Software trigger: un accesso in modalità scrittura a questo registro genera un trigger via software
    control += CAEN_DGTZ_WriteRegister(DHandle, 0x8108, 1);
    // Waiting till at least one event is available to readout
    do{
      control += CAEN_DGTZ_ReadRegister(DHandle, 0x8104, &Data);
    }while(((Data & 0x8) >> 3) != 1);
    //controllo sul bit [3]: tale bit, se posto a 1 ci segnala che è disponibile almenoun evento per la lettura
    
    // Readout
    // HEADER
    // la prima parola del buffer contiene l'event size, in particolare questa informazione é contenuta nei bit [27:0]  
    control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
    EventSize = (Data & 0xFFFFFFF); //see ref1 p.28 (sto estraendo i bit [27:0])
    control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
    control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
    control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
    
    // DATA
    // Each step of the following 'for' statement tries to calibrate one of the odd channels
    for (int k=0; k<4; k++) {
      control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
      sample0 = Data & 0xFF; //seleziono i primi 8 bit della prima parola: si tratta del sample(0)
      
      // vedere ref1 p.24 per i dettagli
      if (channelCalib[k] == false) {
        if (sample0 > 129) {
          DCoffset[2*k+1]+=1;
          control += CAEN_DGTZ_WriteRegister(DHandle, DACregister[2*k+1], DCoffset[2*k+1]);
        } else if (sample0 < 125){
          DCoffset[2*k+1]-=1;
          control += CAEN_DGTZ_WriteRegister(DHandle, DACregister[2*k+1], DCoffset[2*k+1]);
        } else {
          channelCalib[k] = true;
          printf("Channel %i calibrated for DES mode.\n", 2*k+1);
          //printf("sample0 channel %i vale %i\n", 2*k+1, sample0);
        }
      }
      
      // Sliding remainders samples until following channel: Ho letto finora 4+1 registri, ne rimangono dunque, avendo 4 canali (i pari
      // sono stati disabilitati), un totale di (Eventsize-4)/4-1 per arrivare al sample0 del prossimo canale
      for(j = 0; j<((EventSize-4)/4-1); j++) {
        //printf("ok j= %i\n", j);
        control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
      }
      
    }
    // Reading word between two events
    //CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data); // leave without control
    
  } while (channelCalib[0] == false || channelCalib[1] == false || channelCalib[2] == false || channelCalib[3] == false);

  // Stop acquisition
  control += CAEN_DGTZ_WriteRegister(DHandle, 0x8100, 0x8);
  // *** Control cycle: this part is to check if the calibration has been done successfully
  // Start acquisition
  control += CAEN_DGTZ_WriteRegister(DHandle, 0x8100, 0xC);
  // Software trigger (vedi sopra)
  control += CAEN_DGTZ_WriteRegister(DHandle, 0x8108, 1);
  // Readout
  // HEADER
  control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  EventSize = (Data & 0xFFFFFFF);
  control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  // DATA channel 1
  control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  sample0 = Data & 0xFF;
  if(sample0<123 || sample0 >131){
    return 5; //sarà il codice d'errore per dire che la calibrazione dei canali dispari per il des mode non è andata a buon fine
  }
  // Sliding remainders samples until following channel
  for(j = 0; j<((EventSize-4)/4-1); j++) {
    control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  }
  // DATA channel 3
  control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  sample0 = Data & 0xFF;
  if(sample0<123 || sample0 >131){
    return 5;
  }
  // Sliding remainders samples until following channel
  for(j = 0; j<((EventSize-4)/4-1); j++) {
    control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  }
  // DATA channel 5
  control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  sample0 = Data & 0xFF;
  if(sample0<123 || sample0 >131){
    return 5;
  }
  // Sliding remainders samples until following channel
  for(j = 0; j<((EventSize-4)/4-1); j++) {
    control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  }
  // DATA channel 7
  control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  sample0 = Data & 0xFF;
  if(sample0<123 || sample0 >131){
    return 5;
  }
  // Sliding remainders samples until following channel
  for(j = 0; j<((EventSize-4)/4-1); j++) {
    control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  }
  // Stop acquisition
  control += CAEN_DGTZ_WriteRegister(DHandle, 0x8100, 0x8);
  // ***End control cycle
  
  // Setting 1GS/s (and sequential memory access)
  control += CAEN_DGTZ_WriteRegister(DHandle, 0x8000, 0x1010);
  // Disabling odd channels 0x55=01010101
  control += CAEN_DGTZ_WriteRegister(DHandle, 0x8120, 0x55);
  // Software reset (see ref1 p.24)
  control += CAEN_DGTZ_WriteRegister(DHandle, 0xEF24, 1);
  
  if(control != 0){
    return 1; //errore nella lettura/scrittura dei registri durante la calibrazione
  } else {
    printf("The digitizer is ready to work in DES mode (1GS/s).\n");
    printf("WARNING: you have to use the even channels only (CH 0,2,4,6).\n");
    return 0;
  }
}

// ********************
// This function menage the file handling (edit2017: actually not used in file handling)
// INPUT:
// RETURN: a pointer to an open file where you should write your data
FILE * FilePower(int choice){
  ifstream IntFile;
  FILE *fileOut;
  IntFile.open("IntermidiateFile.dat");
  char  fileName[100];
  if(choice == 0){    // open new file
    system("perl script.pl");
    IntFile >> fileName;
    cout<<"File output: "<<fileName<<endl;
    fileOut=fopen(fileName, "w");
    return fileOut;
  }
  else if(choice == 1){    // close file
    return NULL;
  } else {
    return NULL;
  }
}

void SetLocalTrigger(uint32_t ChannelMask,double Vth,uint32_t NlocBeforeTrigger,uint32_t NlocAfterTrigger){
  uint32_t samples;
  uint32_t control=0;
  int i;
 
  // Acquisition Stop and count accepted trigger (trigger settings)
  control += CAEN_DGTZ_WriteRegister(DHandle,0x8100,0x0);
  // Acquisition settings (memory sequential access (bit[4]) and 1 GS/s (bit[12]) and local trigger under threshold (bit[6]))
  control += CAEN_DGTZ_WriteRegister(DHandle, 0x8000, 0x1050);
  // Setting local trigger ch. 0 and 2 with coincidence level 0 (ref1 p.38)
  // Trigger interno impostato in modo che il segnale di trigger parti con l'OR dei singoli segnali di trigger dei due canali ch0 e ch2
  control += CAEN_DGTZ_WriteRegister(DHandle, 0x810C, ChannelMask);

  uint32_t Threshold;
  for(i=0;i<8;i+=2){// loop over even channels
    // Setting V thresholds
    Threshold=ADConverter(i,Vth);
    if(Threshold < 0 || Threshold > 255){
      cout << "ERROR: invalid threshold setting." << endl;
      exit(EXIT_FAILURE);
    }
    // 0x1n80: registro per la soglia per il canale n-esimo
    control += CAEN_DGTZ_WriteRegister(DHandle, 0x1080+i*0x100, Threshold); // ch i
    // Setting Nt: Nt=number of samples groups (8 samples in DESMode) under threshold to generate a trigger signal
    // (segnale di trigger in ritardo rispetto al segnale di 4 samples)
    control += CAEN_DGTZ_WriteRegister(DHandle, 0x1084+i*0x100, 0x1);
  }

  // Buffer organization (buffer diviso in 1024 blocchi, 0xA=1010)
  control += CAEN_DGTZ_WriteRegister(DHandle,0x800C,0xA);

  // Timing of acquisition (before-after trigger)
  if(NlocBeforeTrigger+NlocAfterTrigger>255){
    cout << "Acquisition time error: too long." << endl;
    exit(EXIT_FAILURE);
  }
  // Custom size (default, 4k samples per canale per evento) (1Gs, 4 channel, buffer 1024): #sample/channel=16*N1  eventsize=16*N1+4
  // 0x8020 contiene Nloc=numero di locazioni di memoria di un evento. Una locazione corrisponde, in DES Mode, a 16 samples
  control += CAEN_DGTZ_WriteRegister(DHandle,0x8020,NlocBeforeTrigger+NlocAfterTrigger);
  control += CAEN_DGTZ_ReadRegister(DHandle,0x8020,&samples);
  //Stampo ora il numero di samples totali di un evento
  cout<<"Number of samples = "<<samples*16<<endl;
  // Setting the number of samples after the trigger
  uint32_t post;
  control += CAEN_DGTZ_WriteRegister(DHandle,0x8114,NlocAfterTrigger);
  control += CAEN_DGTZ_ReadRegister(DHandle,0x8114,&post);
  cout<<"Samples after trigger = "<<post*16<<endl;
  // Enabling ch. 0 and 2 (0x5=101)
  control += CAEN_DGTZ_WriteRegister(DHandle,0x8120,ChannelMask);
  control += CAEN_DGTZ_ReadRegister(DHandle,0x8120,&Data);
  printf("Channel mask vale %i\n", Data);
  
  // Acquisition status (waiting till is ready)
  do{
    control += CAEN_DGTZ_ReadRegister(DHandle, 0x8104, &Data);
  }while(!((Data & 0x100) >> 8)); //vedi sopra: aspetto il segnale di ready

  if(control!=0){
    cout<<"Something went wrong in setting local trigger."<<endl;
    exit(EXIT_FAILURE);
  }
}

void SetExternalTrigger(uint32_t ChannelMask,uint32_t NlocBeforeTrigger,uint32_t NlocAfterTrigger){
  uint32_t control=0;
  uint32_t samples;
  // Acquisition settings (memory sequential access and 1 GS/s)
  // GF hanno messo local trigger OVER threshold (non importa se uso ext trigger..)
  // GF local UNDER sarebbe 0x1050
  control += CAEN_DGTZ_WriteRegister(DHandle, 0x8000, 0x1010); 
  // Acquisition Stop (trigger settings)
  control += CAEN_DGTZ_WriteRegister(DHandle,0x8100,0x8);
  // Setting external trigger (bit[30] set to 1) and disabling internal trigger (other bits set to 0)
  control += CAEN_DGTZ_WriteRegister(DHandle, 0x810C, 0x40000000);
  // Buffer organization (buffer diviso in 1024 blocchi, A=1010)
  control += CAEN_DGTZ_WriteRegister(DHandle,0x800C,0xA);

  // Custom size (default, 4k samples per canale per evento) (1Gs, 4 channel, buffer 1024): #sample/channel=16*N1  eventsize=16*N1+4
  // 0x8020 contiene Nloc=Numero di locazioni di memoria di un evento. In DES MOde una locazione corrisponde a 16 samples.
  control += CAEN_DGTZ_WriteRegister(DHandle,0x8020,NlocBeforeTrigger+NlocAfterTrigger);
  control += CAEN_DGTZ_ReadRegister(DHandle,0x8020,&samples);
  cout<<"Number of samples = "<<samples*16<<endl;

  // Setting the number of samples after the trigger (vedi sopra per la funzione SetLocalTrigger, questo passaggo è identico)
  uint32_t post;
  control += CAEN_DGTZ_WriteRegister(DHandle,0x8114,NlocAfterTrigger);
  control += CAEN_DGTZ_ReadRegister(DHandle,0x8114,&post);
  cout<<"Samples after trigger = "<<post*16<<endl;

  /*
  // Enabling ALL even channels 0x55=01010101
  control += CAEN_DGTZ_WriteRegister(DHandle,0x8120,0x55);
  control += CAEN_DGTZ_ReadRegister(DHandle,0x8120,&Data);
  printf("Channel mask vale %i\n", Data);
  */

  // Enabling requested channels
  control += CAEN_DGTZ_WriteRegister(DHandle,0x8120,ChannelMask);
  control += CAEN_DGTZ_ReadRegister(DHandle,0x8120,&Data);
  printf("Channel mask vale %i\n", Data);
  
  
  // Acquisition status (waiting till is ready)
  do{
    control += CAEN_DGTZ_ReadRegister(DHandle, 0x8104, &Data);
  } while (!((Data & 0x100) >> 8));
  // Questa parte era così: }while(!((Data & 0x80) >> 7));
  // però su ref2 p.15 il bit dello stato per l'acquisizione è l'[8] e non il [7]

  if(control!=0){
    cout<<"Something went wrong in setting external trigger."<<endl;
    exit(EXIT_FAILURE);
  }
}

void WaitForTrigger(){
  int draw=0,cycle=0;
  uint32_t control;
  // perdo tempo a fare disegnini in attesa del trigger
  cout<<"Waiting";
  do{
    control += CAEN_DGTZ_ReadRegister(DHandle, 0x8104, &Data);      //controllo bit event ready 
    fflush(0); // libero il buffer
    // ogni 2000 cicli faccio un disegnino e controllo se da tastiera inserisco il comando di stop
    cycle= ((cycle+1)%2000);
    if(cycle==9){
      if(draw==0) printf("\rWaiting.        ");
      if(draw==1) printf("\rWaiting..       ");
      if(draw==2) printf("\rWaiting...      ");
      if(draw==3) printf("\rWaiting....     ");
      if(draw==4) printf("\rWaiting.....    ");
      if(draw==5) printf("\rWaiting......   ");
      if(draw==6) printf("\rWaiting.......  ");
      if(draw==7) printf("\rWaiting........ ");
      if(draw==8) printf("\rWaiting.........");
      if(draw==9) printf("\rWaiting ........");
      if(draw==10)printf("\rWaiting  .......");
      if(draw==11)printf("\rWaiting   ......");
      if(draw==12)printf("\rWaiting    .....");
      if(draw==13)printf("\rWaiting     ....");
      if(draw==14)printf("\rWaiting      ...");
      if(draw==15)printf("\rWaiting       ..");
      if(draw==16)printf("\rWaiting        .");
      if(draw==17)printf("\rWaiting         ");
      draw=((draw+1)%18);
      
      // interruzione manuale dell'acquisizione: basta digitare "s"
      if(kbhit()){
	char c = getch();
	switch (c) {
	case 's':
	  cout<<"\r                    "<<endl;
          //fermo l'acquisizione
	  CAEN_DGTZ_WriteRegister(DHandle,0x8100,0x8);
          //creo il file di stop
	  DatToRoot_Stop();
	  cout << "Acquisition manually stopped." << endl;
	  exit(EXIT_SUCCESS);
	}
      }
    }
  }while(((Data & 0x8)>>3)==0);
  draw=0;
  cycle=0;
  // quando l'evento è pronto per la lettura finisco di perdere tempo
}
uint32_t GetEventDESmode(uint32_t* ch0, uint32_t* ch2){
  int control=0;
  uint32_t EventSize; 
  uint32_t j;
  // Readout
  // HEADER
  control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  EventSize = (Data & 0xFFFFFFF);
  //cout<<"\r                   \nNew event!      "<<endl;
  //cout<<"Eventsize= "<<EventSize<<endl;
  control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  // DATA channel 0
  for(j = 0; j<((EventSize-4)/2); j++) {
    control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
    ch0[4*j]=(Data & 0xFF);
    ch0[4*j+1]=(Data & 0xFF00)>>8;
    ch0[4*j+2]=(Data & 0xFF0000)>>16;
    ch0[4*j+3]=(Data & 0xFF000000)>>24;
  }
  //DATA channel 2
  for(j = 0; j<((EventSize-4)/2); j++) {
    control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
    ch2[4*j]=(Data & 0xFF);
    ch2[4*j+1]=(Data & 0xFF00)>>8;
    ch2[4*j+2]=(Data & 0xFF0000)>>16;
    ch2[4*j+3]=(Data & 0xFF000000)>>24;
  }

  
  if(control!=0){
    cout << "Fatal error during readout." << endl;
    exit(EXIT_FAILURE);
  }
  return (EventSize-4)*2; //ogni parola contiene 4 samples, quindi il numero totale di samples per canale è
                          //((EventSize-4)/2)*4
                          
}
uint32_t GetEventDESmode(uint32_t* ch0, uint32_t* ch2,uint32_t* ch4, uint32_t* ch6){
  int control=0;
  uint32_t EventSize; 
  uint32_t j;
  // Readout
  // HEADER
  control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  EventSize = (Data & 0xFFFFFFF);
  //cout<<"\r                   \nNew event!      "<<endl;
  //cout<<"Eventsize= "<<EventSize<<endl;
  control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
  // DATA channel 0
  for(j = 0; j<((EventSize-4)/4); j++) {
    control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
    ch0[4*j]=(Data & 0xFF);
    ch0[4*j+1]=(Data & 0xFF00)>>8;
    ch0[4*j+2]=(Data & 0xFF0000)>>16;
    ch0[4*j+3]=(Data & 0xFF000000)>>24;
  }
  //DATA channel 2
  for(j = 0; j<((EventSize-4)/4); j++) {
    control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
    ch2[4*j]=(Data & 0xFF);
    ch2[4*j+1]=(Data & 0xFF00)>>8;
    ch2[4*j+2]=(Data & 0xFF0000)>>16;
    ch2[4*j+3]=(Data & 0xFF000000)>>24;
  }
  // DATA channel 4
  for(j = 0; j<((EventSize-4)/4); j++) {
    control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
    ch4[4*j]=(Data & 0xFF);
    ch4[4*j+1]=(Data & 0xFF00)>>8;
    ch4[4*j+2]=(Data & 0xFF0000)>>16;
    ch4[4*j+3]=(Data & 0xFF000000)>>24;
  }
  // DATA channel 6
  for(j = 0; j<((EventSize-4)/4); j++) {
    control+= CAEN_DGTZ_ReadRegister(DHandle,0x0000,&Data);
    ch6[4*j]=(Data & 0xFF);
    ch6[4*j+1]=(Data & 0xFF00)>>8;
    ch6[4*j+2]=(Data & 0xFF0000)>>16;
    ch6[4*j+3]=(Data & 0xFF000000)>>24;
  }
  
  if(control!=0){
    cout << "Fatal error during readout." << endl;
    exit(EXIT_FAILURE);
  }
  return (EventSize-4); //vedi sopra
}

void SaveFile(const char* filename,uint32_t* ch0,uint32_t* ch2,uint32_t* ch4,uint32_t* ch6,int Nsamples){
  ofstream myfile;
  //apro il file
  myfile.open(filename);
  if(myfile.is_open()){
    //myfile << "# t [ns] \tch0 \tch2 \tch4 \tch6" << endl;
      int i=0;
      for(i=0;i<Nsamples;i++){
        //scrivo nel file con la giusta formattazione: #righe=Nsample, colonna0=tempo, colonna(j)=canale( (j-1)*2 )
	myfile << i << "\t\t" << ch0[i] << "\t\t" << ch2[i] << "\t\t" << ch4[i] << "\t\t" << ch6[i] <<endl;
      }
      //chiudo il file
      myfile.close();
    }else{
      cout << "Fatal error while saving to file: could not open "<< filename <<"." <<endl;
      exit(EXIT_FAILURE);
    }
}

bool DatToRoot_IsBusy(){
  ifstream fp1,fp2;
  fp1.open("./tmp/busy");
  fp2.open("./tmp/ready");
  //il processo di scrittura dei file in formato root è occupato se esistono i file busy o ready (vedi DatToRoot_v02.cpp)
  if(fp1.is_open() || fp2.is_open()){
    return true;
  }else{
    return false;
  }
}
void DatToRoot_EventReady(){
  ofstream fp;
  //se l'evento è pronto creo il file ready
  fp.open("./tmp/ready");
  fp<<"ready"<<endl;
  fp.close();
}
void DatToRoot_Stop(){
  ofstream fp;
  //se voglio fermare l'acquisizione creo il file stop
  fp.open("./tmp/stop");
  fp<<"stop"<<endl;
  fp.close();
}
