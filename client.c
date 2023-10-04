/*
    Matteo Schirinzi
    Matricola: 20035542
    Client
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#define DIM 256 //Punto 3 - Costante che stabilisce la massima dimensione dei messaggi

//Dichiarazione funzioni

//Funzione che determina se una parola è formata solo da lettere e non contiene altri simboli
int isAlpha(char *s);
//Rimuove eventuali spazi bianchi in coda alla parola passata come parametro
void trimTrailing(char *str);

int main(int argc, char* argv[])
{
    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;

    char bufIngresso[DIM] = "";                 //Riceve i messaggi dal server
    char bufChiave[DIM] = "";                   //Salva la chiave di protocollo
    char bufTentativi[DIM] = "";                //Salva i tentativi rimanenti
    char bufMessaggioBenvenuto[DIM] = "";       //Messaggio di benvenuto del server
    char bufMessaggioServer[DIM] = "";          //Salva il messaggio di risposta del server
    char bufOut[DIM] = "";                      //Invia messaggi al server
    int tentativi_rimasti = 0;                  //Serve per far visualizzare all'utente quanti tentativi gli rimangono

    //Indici per iteratori
    int i = 0;
    int j = 0;
    int p = 0;
    int m = 0;

    struct sockaddr_in simpleServer;

    //Punto 1 - Il client controlla che il formato dei parametri sia rigorosamente <eseguibile> <IPv4 Server> <Porta Server>
    if (argc != 3)
    {
        fprintf(stderr, "Il formato dei parametri deve essere: <IP Server> <Porta Server>\n");
        exit(EXIT_FAILURE);
    }

    //SOCKET
    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //Controllo sulla creazione della socket
    if (simpleSocket == -1)
    {
        fprintf(stderr, "Errore: non posso creare il socket!\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Socket creato correttamente!\n");
    }

    simplePort = atoi(argv[2]);

    //Pulizia del buffer tramite memset per evitare "caratteri sporchi"
    memset(&simpleServer, '\0', sizeof(simpleServer));
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr = inet_addr(argv[1]);
    simpleServer.sin_port = htons(simplePort);

    //Apertura della connessione con il server
    returnStatus = connect(simpleSocket, (struct sockaddr*)&simpleServer, sizeof(simpleServer));

    //Verifico la connessione con il server
    if (returnStatus == 0)
    {
        printf("Connessione completata\n");
    }
    else
    {
        fprintf(stderr, "Errore: non riesco a connettermi al server\n");
        close(simpleSocket);
        exit(EXIT_FAILURE);
    }

    //Punto 3 - Leggo il messaggio di benvenuto del server
    returnStatus = read(simpleSocket, bufIngresso, sizeof(bufIngresso));

    if (returnStatus < 0)
    {
        fprintf(stderr, "Return Status = %d, errore nel messaggio di benvenuto\n", returnStatus);
        exit(EXIT_FAILURE);
    }
    else
    {
        while (bufIngresso[i] != '\n')
        {
            //Separa la chiave di protocollo 
            if (bufIngresso[i] != ' ' && p == 0)
            {
                bufChiave[i] = bufIngresso[i];                  
            }

            //Separa il numero di tentativi
            if (p == 1)
            {
                bufTentativi[j] = bufIngresso[i];                     
                j++;
            }
            //Conterra' il messaggio di benvenuto del server
            else if (p > 1)
            {
                bufMessaggioBenvenuto[m] = bufIngresso[i];                  
                m++;
            }

            if (bufIngresso[i] == ' ')
            {
                p++;
            }
            i++;
        }


        //Punto 4 - presento al client il messaggio di benvenuto del server senza delimitatore e senza il numero di tentativi (Punto 2)
        printf("\n%s\n", bufMessaggioBenvenuto);
        tentativi_rimasti = atoi(bufTentativi); //Mi salvo i tentativi a disposizione
    }

    //Loop di esecuzione del client
    while(1)
    {
        //Pulizia del buffer tramite memset per evitare "caratteri sporchi"
        memset(bufIngresso, 0, sizeof(bufIngresso));

        int scelta = 0; //Salva la scelta dell'utente
        char parola[5] = ""; //Salva la parola che inserirà l'utente

        //Punto 5 - il client mostra all'utente la scelta
        printf("\nEffettua una scelta (inserisci 1 o 2)\n");
        printf("1 - Nuovo tentativo di identificazione\n");
        printf("2 - Abbonda l'esecuzione\n");
        int controllo = scanf("%d", &scelta);

        //il client controlla che l'utente inserisca un numero e non un altro valore (lettere, parole, altri caratteri...)
        if(controllo != 1)
        {
            fprintf(stderr, "Errore nell'input! Non hai inserito un carattere numerico");
            exit(EXIT_FAILURE);
        }

        //il client controlla che il valore sia 1 o 2, altri valori o input non sono accettati e generano un errore
        while (scelta < 1 || scelta > 2)
        {

            printf("\nIl numero da inserire deve essere 1 o 2, reinserisci\n");
            controllo = scanf("%d", &scelta);

            if(controllo != 1)
            {
                fprintf(stderr, "Errore nell'input! Non hai inserito un carattere numerico");
                exit(EXIT_FAILURE);
            }
        }

        //Punto 6 - caso 5a l'utente vuole fare un tentativo di identificazione
        if (scelta == 1)
        {
            //Il clinet tiene aggiornato l'utente sui tentativi rimasti
            printf("\nTentativi rimasti: %d", tentativi_rimasti);

            //Punto 6.a il client sollecita l'utente ad inserire la parola
            printf("\nInserisci la parola di 5 lettere:\n");
            scanf("%s", parola);

            //Controllo che la parola contenga solo caratteri dell'alfabeto (Punto 11 server)
            while (isAlpha(parola) == 0)
            {
                printf("\nLa parola puo' contenere solo caratteri alfabetici\n");
                printf("\nReinserisci la parola di 5 lettere:\n");
                scanf("%s", parola);
            }

            //Invio la parola al server
            char messaggio[DIM] = "";
            strcpy(messaggio, "WORD");
            strcat(messaggio, " ");
            strcat(messaggio, parola);
            strcat(messaggio, "\n");
            strcpy(bufOut, messaggio);

            write(simpleSocket, bufOut, sizeof(bufOut));

            tentativi_rimasti--;    //dopo aver effettuato il tentativo di riconoscimento decremento i tentativi rimasti
        }
        //La scelta sara' per forza = 2 (eseguo prima il controllo su scelta)
        else
        {
            //Punto 7.a - Punto 5.b - L'utente vuole abbandonare l'esecuzione, il client manda al server il comando QUIT
            char messaggio2[DIM] = "";
            strcpy(messaggio2, "QUIT");
            strcat(messaggio2, "\n");
            strcpy(bufOut, messaggio2);

            write(simpleSocket, bufOut, sizeof(bufOut));
        }

        //Pulizia del buffer tramite memset per evitare "caratteri sporchi"
        memset(bufIngresso, 0, sizeof(bufIngresso));
        memset(bufChiave, 0, sizeof(bufChiave));
        memset(bufTentativi, 0, sizeof(bufTentativi));
        memset(bufMessaggioServer, 0, sizeof(bufMessaggioServer));
        memset(bufOut, 0, sizeof(bufOut));
        
        //Punto 6.b - il client si pone in attesa della risposta del server
        returnStatus = read(simpleSocket, bufIngresso, sizeof(bufIngresso));

        if (returnStatus < 0)
        {
            fprintf(stderr, "Return Status = %d, errore nella ricezione della risposta del server\n", returnStatus);
            break;
        }
        else
        {
            //Punto 6.b.i - controllo se la risposta è positiva = "OK PERFECT"
        
            trimTrailing(bufIngresso);    //Rimuove eventuali spazi bianchi in coda al buffer
            
            if (strcmp(bufIngresso, "OK PERFECT") == 0)
            {
                //Il client offre il riscontro all'utente ed esce dal loop e poi chiuderà la connessione
                printf("\nComplimenti! Hai indovinato la parola\n");
                break;
            }
            //Scompongo il comando ricevuto
            else
            {
                for (size_t i = 0, j = 0, p = 0, m = 0; i < strlen(bufIngresso); i++)
                {
                    //Separa la chiave di protocollo 
                    if (bufIngresso[i] != ' ' && p == 0)
                    {
                        bufChiave[i] = bufIngresso[i];                  
                    }

                    //Separa il numero di tentativi
                    if (p == 1)
                    {
                        bufTentativi[j] = bufIngresso[i];                     
                        j++;
                    }
                    //Conterra' il messaggio del server
                    else if (p > 1)
                    {
                        bufMessaggioServer[m] = bufIngresso[i];                  
                        m++;
                    }

                    if (bufIngresso[i] == ' ')
                    {
                        p++;
                    }
                }

                //Punto 6.b.ii - se la chiave e' == "OK" e sono rimasti ancora tentativi
                if (strcmp(bufChiave, "OK") == 0)
                {
                    //La legenda permette all'utente di comprendere la risposta del server
                    printf("\nLegenda simboli");
                    printf("\n * = carattere corretto in posizione corretta");
                    printf("\n + = carattere corretto in posizione errata");
                    printf("\n - = carattere non presente nella parola");
                    printf("\nRisposta: %s\n", bufMessaggioServer); //Presento all'utente la risposta costruita dal server
                }

                //Punto 6.b.iii - nel caso in cui i tentativi siano esauriti END
                if (strcmp(bufChiave, "END") == 0)
                {
                    //Riporto all'utente il messaggio del server senza i delimitatori
                    printf("\nNon hai indovinato!");
                    printf("\nTentativi effettuati: %s", bufTentativi);
                    printf("\nLa parola da indovinare era: %s\n", bufMessaggioServer);
                    //Il client poi chiude la connessione
                    break;
                }

                //Punto 7.b.i - in caso di risposta positiva QUIT
                if (strcmp(bufChiave, "QUIT") == 0)
                {
                    //Riporto all'utente il messaggio di commiato del server senza i delimitatori
                    printf("\n%s%s\n", bufTentativi, bufMessaggioServer);
                    //Il client poi chiude la connessione
                    break;
                }

                //Punto 6.b.iv - Punto 7.b.ii - Punto 8 - in caso di errore dopo la chiusura della connessione il client termina l'esecuzione
                if (strcmp(bufChiave, "ERR") == 0)
                {
                    //Riporto all'utente il messaggio del server senza i delimitatori
                    fprintf(stderr, "%s%s\n", bufTentativi, bufMessaggioServer);
                    //Il client poi chiude la connessione
                    break;
                }

            }
            
        }

    }

    //Viene effettuata la chisura della connessione e il client termina
    close(simpleSocket);
    printf("\nConnessione chiusa\n");

    return 1;
}

int isAlpha(char *s)
{
    for (unsigned int i = 0; i < strlen(s) - 1; i++)
        if(isalpha(s[i]) == 0)
            return 0;
    return 1;
}

void trimTrailing(char *str)
{
	int index, i;

    index = -1;
    i = 0;

    while(str[i] != '\0')
    {
        if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
        {
            index= i;
        }

        i++;
    }
    
    str[index + 1] = '\0';
}