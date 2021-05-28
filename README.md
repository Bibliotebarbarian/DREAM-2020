# DREAM-2020




## PROGRAMMI
File|Tipo|Descrizione
-|-|-
**temperatura.cpp**			|	File cpp		|	Interfaccia con arduino
**daqvero.cpp** 				|	File cpp		| Interfaccia con il daq
**DatToRoot_v02.cpp**   | Macro Root	| Trasforma i dati in .root, chiamato automaticamente da daqvero
**analisi.c**           | Macro Root	| Analizza i dati raw del daq e scrive gli eventi buoni in un secondo tree, calcola la forme d'onda media per i dati buoni e le fitta con l'esponenziale 

### Grafici
File|Tipo|Descrizione
-|-|-
**chargeHisto.c**       | Macro Root | Istogramma della carica depositata dai dati analizzati da analisia10magg
**chargeTempGraph.c**   | Macro Root | Grafico della carica depositata in funzione della temperatura dai dati analizzati da analisia10magg
**tempGraph.c**					|	Macro Root | Grafico della temperatura in funzione dell'evento dai dati analizzati da analisia10magg
**histo.c**             | Macro Root | Grafico di tutti gli istogrammi usati fino ad adesso

## ALTRO


