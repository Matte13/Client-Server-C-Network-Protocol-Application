/*
    Matteo Schirinzi
    Matricola: 20035542
    Server
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <arpa/inet.h>
#include <time.h>
#define DIM 256 //Punto 5 - Costante che stabilisce la massima dimensione dei messaggi

//Dichiarazione funzioni 

//Funzione che determina se una parola è formata solo da lettere e non contiene altri simboli
int isAlpha(char *parola);
//Punto 3 - Funzione che estrae casualmente da un file .txt le parole che devono essere indovinate
void estraiParola(char *parolaServer);
//Funzione che trasforma la parola passata come parametro in maiuscolo
void toUpper(char* s);
//Funzione che rimuove eventuali spazi bianchi rimasti in coda ad una stringa
char *trimwhitespace(char *str);

int main(int argc, char* argv[])
{
    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    char max_tentativi = 6;  //Numero massimo di tentativi, 6 di default se non viene specificato un'altro valore sul terminale

    struct sockaddr_in simpleServer;

    //Controllo che venga inserita la porta di ingresso da terminale
    if (argc < 2)
    {
        fprintf(stderr, "Inserire la porta di ascolto!\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        //SOCKET
        simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }

    //Punto 2 - numero massimo di tentativi ozionale, se specificato deve essere compreso tra 6 e 10 (estremi compresi)
    if (argc == 3)                                                  
    {
        int num = atoi(argv[2]);

        if (num < 6 || num > 10)
        {

            fprintf(stderr, "Il numero di tentativi deve essere >= 6 e <=10! (default: 6)\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            max_tentativi = atoi(argv[2]);
        }
    }

    //Controllo che non vengano inseriti troppi argomenti (al massimo 3 <eseguibile> <porta> <tentativi>)
    if(argc > 3)
    {
        fprintf(stderr, "Hai inserito troppi argomenti\n");
        exit(EXIT_FAILURE);
    }

    if (simpleSocket == -1)
    {
        fprintf(stderr, "Errore: non posso creare il socket!\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Socket creato correttamente!\n");
    }

    //Estraggo e converto ad intero il valore della porta di ascolto
    simplePort = atoi(argv[1]);

    //Pulizia del buffer tramite memset per evitare "caratteri sporchi"
    memset(&simpleServer, '\0', sizeof(simpleServer));  
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
    simpleServer.sin_port = htons(simplePort);

    //BIND
    returnStatus = bind(simpleSocket, (struct sockaddr*)&simpleServer, sizeof(simpleServer));

    if (returnStatus == 0)
    {
        printf("Connessione completata!\n");
    }
    else
    {
        fprintf(stderr, "Non posso a collegarmi all'indirizzo!\n");
        close(simpleSocket);
        exit(EXIT_FAILURE);
    }

    //LISTEN
    returnStatus = listen(simpleSocket, 5);

    if (returnStatus == -1)
    {
        fprintf(stderr, "Non posso ascoltare dal socket!\n");
        close(simpleSocket);
        exit(EXIT_FAILURE);
    }

    //Esecuzione server (loop)
    while (1)
    {
        struct sockaddr_in clientName = { 0 };
        int simpleChildSocket = 0;
        unsigned int clientNameLength = sizeof(clientName);

        char comando[DIM] = "";                         //Salva il comando inviato dal client (inizialmente vuoto)
        char parola_client[DIM] = "";                   //Salva la parola inviata dal client (inizialmente vuota)
        char parola_server[DIM] = "";                   //Salva la parola estratta casualmente dal server (inizialmente vuota)
        char buf[DIM] = "";                             //Manda i messaggi al client
        char bufClient[DIM] = "";                       //Riceve i messaggi dal client
        char max_tentativi_char[10] = "";               //Per convertire da int a char
        char tentativi_char[10] = "";                   //Per convertire da int a char

        //ACCEPT
        simpleChildSocket = accept(simpleSocket, (struct sockaddr*)&clientName, &clientNameLength);

        if (simpleChildSocket == -1)
        {
            fprintf(stderr, "Non posso accettare connessioni\n");
            close(simpleSocket);
            exit(EXIT_FAILURE);
        }

        //Punto 6 - Messaggio di default all'apertura della connessione 'OK <Max Tentativi> <Messaggio>'
        char startstring[DIM] = ""; //Salva il contenuto del messaggio di benvenuto
        strcpy(startstring, "OK");
        strcat(startstring, " ");   //Carattere "spazio" ASCII 32, 0x20
        sprintf(max_tentativi_char, "%d", max_tentativi);   //Converto i tentativi da int a char per poter essere inviati sul buffer
        strcat(startstring, max_tentativi_char);   //Max tentativi a disposizione 
        strcat(startstring, " ");
        strcat(startstring, "Benvenuto ");
        strcat(startstring, inet_ntoa(clientName.sin_addr));
        strcat(startstring, " sono il server: cerca di indovinare la parola misteriosa!");    //Testo del messaggio di benvenuto personalizzato
        strcat(startstring, "\n");

        write(simpleChildSocket, startstring, sizeof(startstring)); //Invia il messaggio di benvenuto   
        
        //Punto 4 - Dopo aver effettuato la connessione con il client estraggo casualmente la parola
        estraiParola(parola_server);    //Rimane immutate finquando il client rimane connesso al server e cambia quando un nuovo client si connette

        //Inizializzo il contatore dei tentativi a partire da 1
        int tentativi = 1;

        //Esecuzione loop in cui il server comunica con il client e riceve i comandi
        while(1)
        {
            //Reset dei buffer e delle variabili per evitare "caratteri sporchi" (tramite memset)
            memset(bufClient, 0, sizeof(bufClient));
            memset(parola_client, 0, sizeof(parola_client));
            memset(comando, 0, sizeof(comando));
            memset(buf, 0, sizeof(buf));

            //Punto 7 - il server si pone in attesa di un comando da parte del client
            returnStatus = read(simpleChildSocket, bufClient, sizeof(bufClient));   
            if(returnStatus < 0)
                break;
            
            //Punto 12 - il server esamina il messaggio ricevuto ne verifica la correttezza
            //controllo messaggio vuoto
            if (strlen(bufClient) == 0 || strlen(bufClient) == 1)
            {
                char errore[DIM] = "";
                strcpy(errore, "ERR");
                strcat(errore, " ");
                strcat(errore, "Comando errato, il comando è vuoto!\n");
                strcpy(buf, errore);

                write(simpleChildSocket, buf, sizeof(buf));
                break;
            }

            //Scompongo il comando ricevuto
            for (int i = 0, j = 0, k = 0; bufClient[i] != '\n'; i++)                  
            {
                if (bufClient[i] == ' ')
                {
                    k++;
                    i++;
                }

                if (k == 0)
                {
                    //Contiene il comando
                    comando[i] = bufClient[i];                         
                }

                if (k > 0)
                {
                    //Contiene la parola del client
                    parola_client[j] = bufClient[i];                     
                    j++;
                }   
            }

            

            //Punto 8 - Controllo sulla conformita del comando ricevuto (posso ricevere solo WORD o QUIT come comandi e devono essere MAIUSCOLI)
            if ((strcmp(comando, "WORD") != 0 && strcmp(comando, "QUIT") != 0) || 
            (strcmp(comando, "QUIT") == 0 && strlen(parola_client) > 0) || 
            (strcmp(comando, "WORD") == 0 && strlen(parola_client) == 0))
            {
                char errore2[DIM] = "";
                strcpy(errore2, "ERR");
                strcat(errore2, " ");
                strcat(errore2, "Comando '");
                bufClient[strcspn(bufClient, "\n")] = 0;    //rimuovo il carattere newline '\n' per evitare che la risposta sia su due righe
                strcat(errore2, bufClient);
                strcat(errore2, "' errato!");
                strcat(errore2, "\n");
                strcpy(buf, errore2);

                write(simpleChildSocket, buf, sizeof(buf));
                break;
            }

            //controllo presenza caratteri non alfabetici != [A-Za-z] nella parola del client
            parola_client[strcspn(parola_client, "\n")] = 0;
            if (isAlpha(parola_client) == 0)
            {
                char errore1[DIM] = "";
                strcpy(errore1, "ERR");
                strcat(errore1, " ");
                strcat(errore1, "Presenza di un carattere non alfabetico!\n");
                strcpy(buf, errore1);

                write(simpleChildSocket, buf, sizeof(buf));
                break;
            }

            //toUpper di parola_client e di parola_server per il confronto NON case sensitive
            toUpper(parola_client);
            toUpper(parola_server);

            //Punto 10.a - Controllo che la parola inserita dal client sia lunga esattamente 5 caratteri
            if (strcmp(comando, "WORD") == 0 && strlen(parola_client) != 5)                                 
            {
                char errore3[DIM] = "";
                strcpy(errore3, "ERR");
                strcat(errore3, " ");
                strcat(errore3, "La parola deve contenere 5 caratteri\n");
                strcpy(buf, errore3);

                write(simpleChildSocket, buf, sizeof(buf));
                break;
            }

            //Punto 10.a - il server riceve un comando WORD
            if (strcmp(comando, "WORD") == 0)
            {
                //Punto 13.a.i Le due stringhe coincidono
                if (strcmp(parola_server, parola_client) == 0)
                {
                    char messaggio[DIM] = "";
                    strcpy(messaggio, "OK");
                    strcat(messaggio, " ");
                    strcat(messaggio, "PERFECT\n");
                    strcpy(buf, messaggio);

                    write(simpleChildSocket, buf, sizeof(buf));
                    break;
                }
                //Punto 13.a.ii Le due parole differiscono ma i tentativi non sono stati esauriti
                else if (tentativi < max_tentativi)
                {
                    char risposta[DIM] = "";  //Creo una stringa vuota per poi riempirla generando la parola di risposta
                    for (size_t i = 0; i < strlen(parola_client); i++)
                    {
                        //Carattere corretto in posizione corretta
                        if (parola_client[i] == parola_server[i])
                        {
                            strcat(risposta, "*");
                        }
                        //Carattere corretto in posizione errata
                        else if (strchr(parola_server, parola_client[i]) != NULL)    //strchr trova la prima occorrenza del carattere "parola_client[i]" in "parola_server" altrimenti NULL
                        {
                            strcat(risposta, "+");
                        }
                        //Carattere non presente nella parola
                        else
                        {
                            strcat(risposta, "-");
                        }
                    }
                    char messaggio2[DIM] = "";
                    strcpy(messaggio2, "OK");
                    strcat(messaggio2, " ");
                    sprintf(tentativi_char, "%d", tentativi);
                    strcat(messaggio2, tentativi_char);
                    strcat(messaggio2, " ");
                    strcat(messaggio2, risposta);
                    strcat(messaggio2, "\n");
                    strcpy(buf, messaggio2);

                    write(simpleChildSocket, buf, sizeof(buf));

                    tentativi++; //Incremento i tentativi effettuati dal client
                }
                //Punto 13.a.iii Le parole differiscono e sono stati esauriti i tentativi
                else
                {
                    char messaggio3[DIM] = "";
                    strcpy(messaggio3, "END");
                    strcat(messaggio3, " ");
                    sprintf(tentativi_char, "%d", tentativi);
                    strcat(messaggio3, tentativi_char);
                    strcat(messaggio3, " ");
                    strcat(messaggio3, parola_server);
                    strcat(messaggio3, "\n");
                    strcpy(buf, messaggio3);

                    write(simpleChildSocket, buf, sizeof(buf));
                    break;
                }
            }
            //Il comando e' per froza QUIT (perche' ho gia' effettuato il controllo in precedenza sulla tipologia di comando)
            else
            {
                char quit[DIM] = "";
                strcpy(quit, "QUIT");
                strcat(quit, " ");
                strcat(quit, "Arrivederci, la parola era: "); //messaggio di commiato con la parola da indovinare (opzionale)
                strcat(quit, parola_server);
                strcat(quit, "\n");
                strcpy(buf, quit);

                write(simpleChildSocket, buf, sizeof(buf));
                break;
            }

            
        }//Chiusura While(1)

        //Punto 14 - il server chiude la connessione con il client e si pone in attesa di una richiesta di un nuovo client
        close(simpleChildSocket);

    }//Chiusura While(1) Server loop - il server termina il suo ciclo

    //Il server prima di terminare chiude la socket
    close(simpleSocket);

    return 1;
}

int isAlpha(char *parola)
{
    for(unsigned int i = 0; i < strlen(parola); i++)
        if(isalpha(parola[i]) == 0)
            return 0;
            
    return 1;
}

void estraiParola(char *parolaServer)
{
    char stringa[DIM];
    FILE *filePtr = fopen("elenco_parole.txt", "r");
    int numParole = 0;  //conterrà il numero di parole del file

    if(filePtr == NULL)
    {
        fprintf(stderr, "Non esiste il file!\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        //Calcolo il numero di parole nel file
        while(!feof(filePtr))
        {
            fgets(stringa, DIM, filePtr);
            numParole++;
        }

        // mi riposiziono ad inizio file (primo byte)
        fseek(filePtr, 0, 0);

        //Ora posso inizializzare l'array di stringhe che conterrà tutte le parole del file
        char parole[numParole][DIM];

        char *ptr = strtok(stringa, "\n");

        //Copia le parola dal file al vettore delle parole
        for(int j = 0; fgets(stringa, DIM, filePtr) != NULL; j++)
            strcpy(parole[j], ptr);

        //Chiudo il file
        fclose(filePtr);

        //Estraggo casualmente la parola dal vettore delle parole
        srand(time(NULL));
        strcpy(parolaServer, parole[rand() % (numParole + 1)]);
        parolaServer = trimwhitespace(parolaServer);
    }
}

void toUpper(char* s)
{
    char *temp = s;

    while (*temp) {
        *temp = toupper((unsigned char) *temp);
        temp++;
    }
}

char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}