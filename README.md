# sistemes-operatius-c1
Primera pràctica en C


tres activitats

s'ha d'entregar el codi
tots els fitxers necesaris per executar (programa client i programa servidor)
un informe amb: (un informe per les tres activitats)
    explicació de com executar
    explicació del codi
    demostracio de funcionament (dues execucucions per cada activitat)

ACT1:
    S <----TCP----> C

    server generate a seed 
    socket bind listen while(1)

    client request connection to server

    server accept connection
    server generate a random number

    client while generate a number bw 0 and 100
    client send number to server

    server receive number
    server compare number with random number
    server send result to client (-1 if number is lower, 0 if number is equal, 1 if number is higher)
    
    client receive result
    client try to guess the number while result is not 0

    client close connection

    The game is the same that in the lab 0 but whit a server and a client

    Vol veure el htns??? i el htms??? no he entes molt bé las siglas

    el servidor no ha de tenir més de un client

ACT2:
    the client will be the same that in ACT1
    the server will be the same that in ACT1 but a few changes (the only change is the form of generating the random number)

    server will read a file with some lines
    server has to build a table with the number of the line and the number of characters of the line
    the number of characters has to be max 100 (if the line has more than 100 characters, the server will do the modulo 100)
    server chose a random line of the table
    the number of characters of that random line will be the number to guess by the client

ACT3:
    a third program that will be the responsible of reading the file and sending the number of characters to the server

    file3          server           client
         <----UDP         <----UDP
          UDP---->         UDP---->

    file3 will read the file and send the number of characters to the server