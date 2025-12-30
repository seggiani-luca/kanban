# kanban
Implementazione di server e client in C per un protocollo di gestione di 
[Kanban](https://en.wikipedia.org/wiki/Kanban). `doc` contiene una breve 
documentazione.

## Compilare
Si rende disponibile un Makefile per la compilazione con `make`:
```
# compila la lavagna 
make lavagna

# esegue la lavagna con un set di card predefinito, specificato nel file dat/cards.txt
make run_lavagna

# compila il client
make client

# esegue 4 client
make run_clients

# esegue un client su una certa porta
make run_client ARGS="<porta>"

# ripulisce gli oggetti

make clean
```
