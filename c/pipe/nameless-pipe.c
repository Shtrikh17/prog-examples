#include <stdio.h>
#include <unistd.h>

// who ---(stdout)---> [pfd[1] --- pfd[0]] ---(stdin)---> wc -l

void who_wc(){
int pfd[2]; // pipe
pipe(pfd);
if(!fork()){
// Child process
close(STDOUT_FILENO); // Close stdout
dup2(pfd[1], STDOUT_FILENO); // Take over descriptor - stdout goes to pipe
close(pfd[1]); // Close pfd[1] - not necessary any more
close(pfd[0]); // pfd[0] - for stdin, not necessary here at all
execlp("who", "who", NULL); // substitute process
}
else{
// Parent process
close(STDIN_FILENO);
dup2(pfd[0], STDIN_FILENO);
close(pfd[0]);
close(pfd[1]);
execlp("wc", "wc", "-l", NULL);
}
}

int main(int argc, char** argv){
who_wc();
return 0;
}
