#BOSH: BOSC Shell projekt

Ulrik Flænø Damm og Niklas Hansen

##Introduktion
Det er blevet stillet til opgave at lave en Linux-shell, ligesom BASH. Den skal være skrevet i C, og skal opfylde en række krav, som stillet i rapporten. Denne rapport beskriver det program vi har lavet, hvilke features den har, samt test cases som viser at vores program opflyder alle de stillede krav.
Den færdige kode er inkluderet med denne rapport.

##Programmets opbygning
De vigtiste dele af programmet er i shell.c og input.c. input.c parser inputtet ved at læse hver character i en string, og opbygger en cmd datastruktur, med det indtastede data. 
shell.c indeholder selve shell-koden. En funktion printer promptet inden hver linje med brugernavn, hostname og working directory, hentet henholdsvis med USER miljøvariablen, /proc/sys/kernel/hostname filen og getwch funktionen.
Når et cmd objekt skal eksekveres løber den alle de linkede kommandoer igennem, fork()'er ud i en child process og exec'er det specificerede program. Hvis input eller output skal redirectes bliver den stream det skal redirectes lukkes, og dup()'et med den åbnede file descriptor.
Skal et program pipe dets input (programmerne er kørt bagfra), vil en pipe blive åbnet, og write enden af den bliver sendt med som parameter til den næste process, som skal startes.
Inden den kører en cmd vil shellen tjekke for, om det er en indbygget kommando, som f.eks. cd eller exit. Hvis det er det, vil den udføre noget indbygget kode, og defefter afslutte. Det er i den afleverede version ikke understøttet at bruge indbyggede kommandoer med pipes.
main.c vil, i et loop, printe promptet, læse input, parse det via input.c, og eksekvere det via shell.c. Den eneste anden funktion her er at håndtere UNIX signaler, specifikt SIGINT og SIGCHLD. SIGINT er signalet ved ctrl-c, som skal afslutte de kørende programmer, men ikke shellen selv. SIGCHLD er til at fange om en child processes er blev færdig, og så wait()'e for den, så at den ikke bliver hængende som en zombie process.

##Oversigt over filerne
###main.c:
Indeholder main funktionen, starter programmet og håndterer UNIX signaler.
Selve main funktionen er ret simpel. Den kører i et uendeligt loop, som den kun hopper ud af, hvis programmet skal afsluttes (hvilket gøres via exit kommandoen). I dette loop læser den input via readline() funktionen. Som parameter gives promptet, som skal skrives inden cursoren.
Når den har fået input bliver dette parset med input_parse funktionen fra input.h, som vil konvertere inputtet til en linked list af command objekter. Denne liste vil så blive kørt bagfra igennem af shell_run_command() funktionen, som vil eksekvere brugerens input.
int_handler() funktionen er til håndtering af SIGINT signalet, som kommer når der bliver trykket ctrl-c. Dette skal ikke afslutte vores shell, som er hvad standard-handleren ville gøre, så vi implementerer vores egen, som i stedet siger til vores shell-objekt, at det skal afslutte alle kørende forgrundsprocesser. Hvis der ikke kører nogle processer i forgrunden vil funktionen i stedet bruge et long jump til at hoppe hen til det sted i main() funktionen, hvor den tager imod input. Dette vil fjerne det input der allerede står.
child_handler() funktionen er til håndtering af SIGCHLD signalet, som kommer når en child process er færdig med at køre. Hvis man bare fork()'er en ny process, men aldrig wait()'er på at den bliver færdig, vil den blive hængende som en zombie process. I tilfælde af at dette sker, vil SIGCHLD signalet blive sendt til parent processen (vores shell), og vores handler vil wait()'e på processen, så at den afslutter normalt.

###cmd.c:
Definerer datastrukturerne til kommandoerne. En Linjes user input er en serie af commands, som er sat sammen som en dobbelt linked list. Et link til en command betyder enten at der skal oprettes en pipe imellem dem, eller at de begge skal udføres, med den ene i baggrunden.
For at simplificere memory management kopierer vi alle strings, som bliver sat ind i data strukturen, over på heapen, så vi har vores egen kopi. Derfor er der defineret setter funktioner til alle string (char *) properties i struct'en.

###input.c:
Denne fil har en enkelt funktion: at parse en input string til en command list. Interfacen i input.h er rimeligt simpel: der er en enkelt funktion, parse_input(), som tager en string og sender enten en command eller en error tilbage.
Parsingen foregår ved at læse inputtet en character af gangen. source_counter variablen i parser_struct datastrukturen holder styr på hvor i inputtet vi er. parse_input() funktionen vil gå igennem input strengen, indtil den når til end of file. Den vil så tjekke hvad det er source_counter'en pejer på: hvis det er en kendt operator, som f.eks. '|' eller '>>', vil den læse den næste string i inputtet, og sætte den ind det rigtige sted i den command den er i gang med at parse.
Er der ikke nogen kendt operator vil den læse den næste string og indsætte det som et argument. Det første argument vil altid være programnavnet, så der skal ikke gøres noget specielt her.
Læsning af strings fungerer med get_string() funktionen. Den vil returnere en string_ref, som bare er en start-position og længde i input sting'en. Dette er i stedet for at kopiere en delstring ud af input string'en, og skulle håndtere memory for denne. Hvis det første tegn i strengen er en string-start ("), vil den læse alt indtil string-end ("). Ellers vil den læse indtil næste whitespace.

###shell.c:
Definerer shell objektet, som man kan bruge til at eksekvere command lister. Den består hovedsageligt at to funktioner: shell_run_command() og shell_run_command_with_pipe(). shell_run_command() funktionen er den offentligt tilgængelige funktion til at stare en programeksekvering. Den vil kigge på hvilket program der skal køres, og find ud af om det er en indbygget kommando. Hvis der er, vil den køre en indbygget handler: ellers vil den sende det vidre til shell_run_command_with_pipe().
shell_run_command_with_pipe() funktionen kører hele listen af kommandoer igennem rekuisivt, og starter hvert program. Funktionen tager en file descriptor som input: dette er til at kunne pipe to programmer sammen. Er to programmer pipet sammen, skal det ene sende dets output til dets andets input. Dette sker ved at oprette en pipe med pipe() kaldet, og så på hver process lukke en strøm og udskifte den med en pipe-ende. I dette tilfælde vil den oprette pipen, og så i child-processen lukke stdlin og dup()'e pipen's read-ende. Write enden vil blive send som parameter til det næste kald af shell_run_command_with_pipe().
Bliver shell_run_command_with_pipe() kaldt med en valid file descriptor, vil den, efter den har fork()'et til en child-process, lukke stdout og dup()'e med file descriptoren, som vil være write enden af den pipe der blev åbnet af den forrige kommando.
At starte child-processen er meget simpelt: et kald til fork() vil kopiere hele den kørende process som en child-process. Hvad fork() returnerer bestemmer hvilken process man er i: 0 betyder at man befinder sig i child-processen, og en værdi over 0 vil være child-processens process id, og vil betyde at man er i parent processen. (En værdi under 0 vil betyde at der skete en fejl, f.eks. at der ikke er systemresourcer nok til at starte nye processer.)
Efter man har splittet programmet ud i en child process vil man så skifte programmet ud med det program, man gerne vil starte. Dette gøre via exec-funktionerne. De vil tage det program, man giver som input, og skrive hele det kørende program ud med det (dog vil alle åbne file descriptors forblive åbne, hvilket gør at vi kan redirecte stdin, stdout og stderr.) Helt specifikt bruger vi funktionen execvp(), som sig at programmet skal læses fra alle mapper i systemet's PATH miljøvariabel, og at vi gerne vil give programargumenterne som et array.
Hvis exec-funktionen lykkedes, vil programmet ikke fortsætte efter dette punkt. Hvis den fejler vil den dog forsætte, så hvis koden efter exec bliver kørt, betyder det at der var en fejl i udførelsen, f.eks. at det program man gerne ville køre ikke kan findes.

###array.c:
Definerer et array objekt, som kan bruges til nemt at implementere en statisk allokeret liste af elementer, som stadig kan dynamisk udvide sig. Dette objekt blev lavet til at undgå at skulle definere et maks antal af f.eks. pipes i en enkelt linje, uden at miste performancen af statiske allokeringer. 

###util.c:
En fil til at placere globale convinience funktioner. Indeholder salloc og srealloc, som er "safe" versioner af malloc og realloc, som, hvis en allokering fejler, vil afslutte programmet, i stedet for at lade det fortsætte i udefineret state. 

##Programkrav
###shellen skal fungere uafhængigt
Der bliver i hele programmet ikke lavet noget system kald som starter bash. Det er muligt at køre bash fra shellen, da det bare er et program der kan køres, men det er ikke påkrævet for at møde kravene. 

###promtet skal vise hostnavn
Inden hvert input bliver det vist et promt, som indeholder navnet på den bruger man er logget ind med (hentet fra USER miljøvariablen), host navnet (hentet fra /proc/sys/kernel/hostname) samt den mappe man befinder sig i (hentet via getcwd()). 

###Man skal kunne køre standard UNIX programmer
Når man har givet en linjes input, vil den blive parset, og ens kommando vil blive kørt. Programmerne bliver kørt ved at forke ud i en child process og kalde execvp. Exec skifter det kørende program ud med et andet program, og ’p’ betyder at den skal læse programmerne fra alle steder i PATH miljøvariablen, som inkluderer /usr/bin, hvor alle UNIX standard programmerne ligger. Dvs at skriver f.eks. cat, vil den fork()’e, og derefter skifte child processen ud med /usr/bin/cat. 

###Programmer skal kunne eksekveres som baggrundsprocesser
Kører man en kommando med et & som den sidste parameter, vil shellen ikke vente på at programmet afslutter, men vil i stedet vise promtet igen, så man kan indtaste en ny kommando.

###Der skal være mulighed for redirection
Man kan bruge , >> og >2 for at redirecte henholdsvis stdin, stdout, stdout som append samt stderr. Det vil redirecte streamen til det filnavn man specificerer.

###Det skal være muligt at andvende pipes
Man kan pipe to programmer sammen via pipe '|' operatoren. Dette vil tage stdout fra det ene program og koble det til stdin hos det andet program.

###Shell'en skal have en exit function
Skriver man exit vil shellen afslutte på normal vis. 

###Ctrl-C skal afslutte det program der kører
Der er lavet en handler til SIGINT signalet, som bliver sendt ved Ctrl-C. Dette vil afslutte alle programmer der kører i forgrunden, og givet et nyt prompt til indtastning af et nyt program. Ctrl-C mens man ikke kører noget program vil fjerne ens input, og give et tomt prompt.

##Test
De følgende test er blevet udført for at sikre at programmet har den påkrævede funktionalitet:

###Simpel programkørsel
Beskrivelse: Denne test viser at man kan køre et simpelt UNIX-program, som 'ls'.

    ulrikdamm@ubuntu /media/psf/Home/Developer/shell>ls
    array.c  array.h  array.o  cmd.c  cmd.h  cmd.o	input.c  input.h  input.o  main.c  main.o  Makefile  readme.md	shell  shell.c	shell.h  shell.o  util.c  util.h  util.o

###Program med argumenter
Beskrivelse: Denne test viser at man kan køre et program med argumenter, som 'echo lol'

    ulrikdamm@ubuntu /media/psf/Home/Developer/shell>echo lol
    lol

###Program med redirectet input og output
Beskrivelse: Denne test viser at man kan køre et program, og redirecte input og output streamen, som i 'wc -l < /etc/passwd > antalkontoer'

    ulrikdamm@ubuntu /media/psf/Home/Developer/shell>wc -l < /etc/passwd > antalkontoer
    ulrikdamm@ubuntu /media/psf/Home/Developer/shell>cat antalkontoer
    36

###Program med redirectet error
Beskrivelse: Denne test viser at man kan køre et program, og redirecte standard error output streamen, som i 'wget 2>outputfil'

    ulrikdamm@ubuntu /media/psf/Home/Developer/shell>wget 2>outputfil
    
    ulrikdamm@ubuntu /media/psf/Home/Developer/shell>cat outputfil
    wget: missing URL
    Usage: wget [OPTION]... [URL]...
    Try `wget --help' for more options.

###Program med append output
Beskrivelse: Denne test viser at man kan køre et program med output redirectet, så at den tilføjer indholdet til en fil i stedet for at overskrive indholdet, som hvis man kørte 'echo lol >>outfil' to gange.

    ulrikdamm@ubuntu /media/psf/Home/Developer/shell>echo lol >>outfil
    ulrikdamm@ubuntu /media/psf/Home/Developer/shell>echo lol >>outfil
    ulrikdamm@ubuntu /media/psf/Home/Developer/shell>cat outfil
    lol
    lol

###Directory change
Beskrivelse: Denne test viser at man kan bruge cd til at skrive hvilken mappe man er i.

    ulrikdamm@ubuntu /media/psf/Home/Developer/shell>ls
    antalkontoer  array.h  cmd.c  cmd.o    input.h	main.c	Makefile  outputfil  shell    shell.h  testdir	util.h
    array.c       array.o  cmd.h  input.c  input.o	main.o	outfil	  readme.md  shell.c  shell.o  util.c	util.o
    ulrikdamm@ubuntu /media/psf/Home/Developer/shell>cd testdir
    ulrikdamm@ubuntu /media/psf/Home/Developer/shell/testdir>ls
    ulrikdamm@ubuntu /media/psf/Home/Developer/shell/testdir>cd ..
    ulrikdamm@ubuntu /media/psf/Home/Developer/shell>ls
    antalkontoer  array.h  cmd.c  cmd.o    input.h	main.c	Makefile  outputfil  shell    shell.h  testdir	util.h
    array.c       array.o  cmd.h  input.c  input.o	main.o	outfil	  readme.md  shell.c  shell.o  util.c	util.o

###Piping af output
Beskrivelse: Denne test viser at man kan pipe outputtet fra et program til et andet.

    ulrikdamm@ubuntu /media/psf/Home/Developer/shell>ls | wc -w
    24

###Kørsel af programmer i baggrunden
Beskrivelse: Denne test viser at man kan bruge '&' til at køre programmer i baggrunden.

    ulrikdamm@ubuntu /media/psf/Home/Developer/shell>sleep 10 &
    ulrikdamm@ubuntu /media/psf/Home/Developer/shell>ps
      PID TTY          TIME CMD
      1894 pts/0    00:00:00 bash
      2262 pts/0    00:00:00 shell
      2279 pts/0    00:00:00 sleep
      2280 pts/0    00:00:00 ps

###Afslutning af programmer med ctrl-C
Beskrivelse: Denne test viser at man kan afslutte programmer med ctrl-c.

    ulrikdamm@ubuntu /media/psf/Home/Developer/shell>sleep 10
    ^Culrikdamm@ubuntu /media/psf/Home/Developer/shell>

###Exit af shell'en
Beskrivelse: Denne test viser at man kan afslutte shellen med exit kommandoen:

    ulrikdamm@ubuntu /media/psf/Home/Developer/shell>exit
    ulrikdamm@ubuntu:~/Parallels Shared Folders/Home/Developer/shell$ 

##Konklusion

Som vist i rapporten, så virker vores shell, og den opfylder også alle kravene. Der er test cases til hvert krav, som beviser dette.
