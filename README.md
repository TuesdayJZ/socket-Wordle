# socket-Wordle

- this is a Wordle using socket.

- the rules are the same as for regular Wordle.

- Wordle is randomly selected from "words.txt" file by the server.

- you can choose the number of trials (1 - 9).

## How to run these server and client

1. run, "run.sh" in this directory to make binaries, "Server" and "Client"

2. make sure that you have binary "Server" and "words.txt" in the same directory

3. run, "./Server [port number] ([number of players] ([-d]))"
   - arguments in () are optional

   - you can use -d option for multiple people to play different Wordle

4. run, "./Client [server IP] [port number]"

   - server will timeout if no client is connected in 30 seconds

## reference

"words.txt" is borrowed from <https://github.com/tabatkins/wordle-list.git>
